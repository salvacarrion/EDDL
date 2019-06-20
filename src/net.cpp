// This file is part of EDDLL an European Distributed Deep Learning Library.
// Developed within the DeepHealth project.
// Boosting AI in Europe.
//
// The MIT License (MIT)
//
// Copyright (c) 2019
//           Roberto Paredes Palacios, <rparedes@dsic.upv.es>
//           Jon Ander Gómez, <jon@dsic.upv.es>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>
#include "net.h"
#include <pthread.h>

#ifdef cGPU
#include "gpu/tensor_cuda.h"
#endif

#define VERBOSE 0

using namespace std;
using namespace std::chrono;

ostream &operator<<(ostream &os, const vector<int> shape) {
    int i;
    os << "(";
    for (i = 0; i < shape.size() - 1; ++i) {
        os << shape[i];
        os << "x";
    }
    os << shape[i] << ")";

    return os;
}

//// THREADS
struct tdata {
    Net *net;
    int batch_size;
    int eval;
};

/////////////////////////////////////////
void *train_batch_t(void *t) {
    auto *targs = (tdata *) t;

    Net *net = targs->net;

    net->reset();

    net->forward();

    net->loss();

    if (!targs->eval) {
        net->delta();
        net->backward();
        if (net->dev > DEV_CPU)
            net->applygrads(targs->batch_size);
    }

    return nullptr;
}

/////////////////////////////////////////
void *applygrads_t(void *t) {
    auto *targs = (tdata *) t;

    Net *net = targs->net;

    net->applygrads(targs->batch_size);

    return nullptr;
}


/////////////////////////////////////////
int isIn(Layer *l, vlayer vl, int &ind) {
    for (int i = 0; i < vl.size(); i++)
        if (l == vl[i]) {
            ind = i;
            return 1;
        }

    return 0;
}

/////////////////////////////////////////
int isInorig(Layer *l, vlayer vl, int &ind) {
    for (int i = 0; i < vl.size(); i++)
        if (l == vl[i]->orig) {
            ind = i;
            return 1;
        }

    return 0;
}



////////////////////////////////////
///// NET CLASS
////////////////////////////////////

Net::Net(vlayer in, vlayer out) {
    // Set input/outlayer
    lin = in;
    lout = out;

    // Default optimizer
    optimizer = nullptr;

    // Walk through the pointers of all layers, to get a plain
    // vector with all the layers
    for (int i = 0; i < lin.size(); i++) {
        walk(lin[i]);
    }
}


/////////////////////////////////////////
int Net::inNet(Layer *l) {
    // Check if the layer l is in the network
    for (int i = 0; i < layers.size(); i++)
        if (l == layers[i]) return 1;
    return 0;
}


/////////////////////////////////////////
void Net::walk(Layer *l) {
    // If this layer is not in the network, add it, as well as all its children (recursively)
    if (!inNet(l)) {
        layers.push_back(l);
        for (int i = 0; i < l->child.size(); i++)
            walk(l->child[i]);
    }
}

/////////////////////////////////////////
Layer *Net::getLayer(string name) {
    for (int i = 0; i != layers.size(); i++)
        if (name == layers[i]->name) return layers[i];

    msg("layer %s not found", "Net.getLayer");
    return nullptr;
}

/////////////////////////////////////////
string Net::summary() {
    std::stringstream ss;

    for (int i = 0; i < layers.size(); i++) {
        ss << layers[i]->name.c_str() << " ";
    }

    ss << "\n";
    for (int i = 0; i < layers.size(); i++) {
        ss << layers[i]->name << ": ";

        vector<int> si = layers[i]->input->getShape();
        vector<int> so = layers[i]->output->getShape();
        ss << si << "-->" << so << "\n";
    }

    return ss.str();
}

void Net::plot(string fname) {
    ofstream out("tmp.dot");
    int ind;
    string type = fname.substr(fname.find('.') + 1);
    string cmd;


    out << "digraph Model {\n";
    out << "rankdir=LR;\n";

    // plot layers
    for (int i = 0; i != layers.size(); i++)
        if ((!isIn(layers[i], lin, ind)) && (!isIn(layers[i], lout, ind)))
            out << layers[i]->plot(0) << "\n";

    // Input Layers
    for (int i = 0; i != lin.size(); i++)
        out << lin[i]->plot(1) << "\n";

    // Output Layers
    for (int i = 0; i != lout.size(); i++)
        out << lout[i]->plot(1) << "\n";

    //plot links
    for (int i = 0; i != layers.size(); i++)
        for (int j = 0; j < layers[i]->child.size(); j++)
            out << layers[i]->name << "->" << layers[i]->child[j]->name << "\n";

    out << "}\n";

    out.close();

    cmd = "dot -T " + type + " ./tmp.dot >" + "./" + fname;

    system(cmd.c_str());

}

/////////////////////////////////////////
void Net::initialize() {
    for (int i = 0; i != layers.size(); i++)
        layers[i]->initialize();
}

/////////////////////////////////////////
void Net::reset() {
    for (int i = 0; i != layers.size(); i++)
        layers[i]->reset();
}

/////////////////////////////////////////
void Net::fts() {
    int i, j, k, n;
    vector<int> visit;
    vector<int> gin;

    //fprintf(stdout,"FTS:");
    for (i = 0; i < layers.size(); i++) {
        visit.push_back(0);
        gin.push_back(layers[i]->lin);
    }

    for (i = 0; i < layers.size(); i++) {

        for (j = 0; j < layers.size(); j++)
            if ((gin[j] == 0) && (!visit[j])) break;

        if (j == layers.size())
            msg("error recurrent net", "Net.fts");

        /*
          if (layers[j]->lout)
          fprintf(stdout,"%s-->",layers[j]->name.c_str());
          else
          fprintf(stdout,"%s |",layers[j]->name.c_str());
        */
        visit[j] = 1;
        vfts.push_back(layers[j]);

        for (k = 0; k < layers[j]->lout; k++)
            for (n = 0; n < layers.size(); n++)
                if (layers[n] == layers[j]->child[k]) gin[n]--;

    }
    //fprintf(stdout,"\n");

}


/////////////////////////////////////////
void Net::bts() {
    int i, j, k, n;
    vector<int> visit;
    vector<int> gout;

    //fprintf(stdout,"BTS:");
    for (i = 0; i < layers.size(); i++) {
        visit.push_back(0);
        gout.push_back(layers[i]->lout);
    }

    for (i = 0; i < layers.size(); i++) {

        for (j = 0; j < layers.size(); j++)
            if ((gout[j] == 0) && (!visit[j])) break;

        if (j == layers.size())
            msg("error recurrent net in", "Net.bts");

        /*
          if (layers[j]->lin)
          fprintf(stdout,"%s-->",layers[j]->name.c_str());
          else
          fprintf(stdout,"%s |",layers[j]->name.c_str());
        */
        visit[j] = 1;
        vbts.push_back(layers[j]);

        for (k = 0; k < layers[j]->lin; k++)
            for (n = 0; n < layers.size(); n++)
                if (layers[n] == layers[j]->parent[k]) gout[n]--;

    }
    //fprintf(stdout,"\n");
}


/////////////////////////////////////////
void Net::build(optim *opt, vloss co, vmetrics me) {
    fprintf(stdout, "Build net\n");
    if (co.size() != lout.size())
        msg("Loss list size does not match output list", "Net.build");

    if (me.size() != lout.size())
        msg("Metric list size does not match output list", "Net.build");

    // check devices
    dev = -1;
    int ind;
    for (int i = 0; i < layers.size(); i++){
        // do not consider input layers, since they are always on CPU
        if (!isIn(layers[i], lin, ind)) {
            if (dev == -1) dev = layers[i]->dev;
            else {
                if (layers[i]->dev != dev)
                    msg("Net with layers in different devices", "Net.build");
            }
        }
    }
    if (dev == DEV_CPU)
        cout << "Net running on CPU\n";
    else if (dev < DEV_FPGA)
        cout << "Net running on GPU " << dev - DEV_GPU << "\n";
    else
        cout << "Net running on FPGA " << dev - DEV_FPGA << "\n";

    // set optimizer
    optimizer = opt;
    optimizer->setlayers(layers);
    // Initialize fiting errors vector
    for (int i = 0; i < co.size(); i++) {
        fiterr.push_back(0.0);
        fiterr.push_back(0.0);
    }
    // set loss functions and create targets tensors
    this->losses = vloss(co);
    for (int i = 0; i < co.size(); i++) {
        if (co[i]->name == "soft_cross_entropy") lout[i]->delta_bp = 1;
        lout[i]->target = new Tensor(lout[i]->output->getShape(), dev);
    }

    // set metrics
    this->metrics = vmetrics(me);

    // forward sort
    fts();
    // backward sort
    bts();
    // random params
    initialize();
}

void Net::set_compserv(CompServ *cs){
    int todev;

    if (cs->type == "local") {

        if (cs->local_gpus.size() > 0) todev = DEV_GPU;
        else if (cs->local_fpgas.size() > 0) todev = DEV_FPGA;
        else todev = DEV_CPU;

        // split net in devices
        if (todev == DEV_CPU) {
            if (dev == DEV_CPU) {
                // split on multiple threads
                unsigned int nthreads = cs->local_threads;

                if (nthreads <= 0)
                    msg("Threads must be > 0", "Net.build");

                cout << "set threads to " << nthreads << "\n";

                Eigen::initParallel();
                Eigen::setNbThreads(1);
                split(nthreads, DEV_CPU);
            } else {
                msg("Net and Layers device missmatch", "Net.build");
            }
        } else if (todev < DEV_FPGA) {
#ifndef cGPU
            msg("EDDLL not compiled for GPU", "Net.build");
#else
            // split on multiple GPUs
        int ngpus=gpu_devices();
        if (ngpus==0) {
          msg("GPU devices not found","Net.build");
        }
        if (cs->local_gpus.size()>ngpus)
        {
          msg("GPU list on ComputingService is larger than available devices","Net.build");
        }

        for(int i=0;i<cs->local_gpus.size();i++)
          if (cs->local_gpus[i]) devsel.push_back(i);
        if (!devsel.size())
          msg("No gpu selected","Net.build");

        cout<<"split into "<<devsel.size()<<" GPUs devices\n";
        split(cs->local_gpus.size(),DEV_GPU);
#endif
        } else {
            // split on multiple FPGAs
        }
    } else {
        msg("Distributed version not yet implemented", "Net.build");
    }
}

void Net::split(int c, int todev) {
    int i, j, k, l;

    vlayer nlayers;
    vlayer nin;
    vlayer nout;
    int ind;


    int batch = (lin[0]->input->getShape())[0];
    if (batch < c)
        msg("Too small batch size to split into cores", "Net.split");


    int bs = batch / c;
    int m = batch % c;


    // Tensors for input/output for split nets.
    for (int i = 0; i < c; i++)
        for (int j = 0; j < lin.size(); j++) {
            vector<int> shape = lin[j]->input->getShape();
            if (i == (c - 1)) shape[0] = bs + m;
            else shape[0] = bs;

            Xs[i].push_back(new Tensor(shape));
        }


    for (int i = 0; i < c; i++)
        for (int j = 0; j < lout.size(); j++) {
            vector<int> shape = lout[j]->output->getShape();
            if (i == (c - 1)) shape[0] = bs + m;
            else shape[0] = bs;

            Ys[i].push_back(new Tensor(shape));
        }
    ////

    for (i = 0; i < c; i++) {
        cout << "Split " << i << "\n";

        nlayers.clear();
        nin.clear();
        nout.clear();

        if (i == c - 1) bs += m;

        // set inputs
        for (j = 0; j < lin.size(); j++) {
            vlayer par;

            if (todev == DEV_CPU) nin.push_back(layers[j]->share(i, bs, par));
            else nin.push_back(layers[j]->clone(c, bs, par, todev + devsel[i]));
            nlayers.push_back(nin[j]);
        }

        for (k = 0; k < layers.size(); k++)
            for (j = 0; j < layers.size(); j++) {
                if (!isInorig(layers[j], nlayers, ind)) {
                    vlayer par;
                    for (l = 0; l < layers[j]->parent.size(); l++)
                        if (!isInorig(layers[j]->parent[l], nlayers, ind)) break;
                        else par.push_back(nlayers[ind]);

                    if (l == layers[j]->parent.size()) {
                        if (todev == DEV_CPU) nlayers.push_back(layers[j]->share(i, bs, par));
                        else nlayers.push_back(layers[j]->clone(i, bs, par, todev + devsel[i]));
                    }
                }
            }

        // set outputs
        for (j = 0; j < lout.size(); j++)
            if (isInorig(lout[j], nlayers, ind))
                nout.push_back(nlayers[ind]);

        // create new net
        snets.push_back(new Net(nin, nout));

        //snets[i]->info();

        // build new net
        snets[i]->build(optimizer->clone(), losses, metrics);
    }

}

void Net::setmode(int m) {
    for (int i = 0; i < layers.size(); i++)
        layers[i]->setmode(m);

    if (snets.size())
        for (int i = 0; i != snets.size(); i++)
            snets[i]->setmode(m);
}

/////////////////////////////////////////
void Net::forward() {
    for (int i = 0; i < vfts.size(); i++) {
        vfts[i]->forward();
    }

    if (VERBOSE) {
        for (int i = 0; i < layers.size(); i++) {
            cout << layers[i]->name << "\n";
            fprintf(stdout, "  %s In:%f\n", layers[i]->name.c_str(), layers[i]->input->total_sum());
            fprintf(stdout, "  %s Out:%f\n", layers[i]->name.c_str(), layers[i]->output->total_sum());
        }

        getchar();
    }


}


void Net::delta() {
    for (int i = 0; i < lout.size(); i++)
        losses[i]->delta(lout[i]->target, lout[i]->output, lout[i]->delta);

}


void Net::loss() {
    int p = 0;
    for (int i = 0; i < lout.size(); i++, p += 2) {
        // loss value
        fiterr[p] = losses[i]->value(lout[i]->target, lout[i]->output);
        // metric value
        fiterr[p + 1] = metrics[i]->value(lout[i]->target, lout[i]->output);
    }
}


/////////////////////////////////////////
void Net::backward() {
    for (int i = 0; i < vbts.size(); i++)
        vbts[i]->backward();

}


/////////////////////////////////////////
void Net::applygrads(int batch) {

    if (VERBOSE) {
        for (int i = 0; i < layers.size(); i++) {
            cout << layers[i]->name << "\n";
            fprintf(stdout, "  In:%f\n", layers[i]->input->total_abs());
            fprintf(stdout, "  Out:%f\n", layers[i]->output->total_abs());
            fprintf(stdout, "  Delta:%f\n", layers[i]->delta->total_abs());
            for (int j = 0; j < layers[i]->gradients.size(); j++) {
                fprintf(stdout, "  %f\n", layers[i]->gradients[j]->total_abs());
            }
        }
        getchar();
    }

    optimizer->applygrads(batch);
}


void Net::build(optim *opt, vloss in, vmetrics out, CompServ *cs){
    build(opt, in, out);
    set_compserv(cs);
}
void Net::fit(vtensor tin, vtensor tout, int batch_size, int epochs) {
    int i, j, k, n;

    // Check current optimizer
    if (optimizer == nullptr)
        msg("Net is not build", "Net.fit");

    // Check if number of input/output network layers matches with the input/output tensor data
    if (tin.size() != lin.size())
        msg("input tensor list does not match with defined input layers", "Net.fit");
    if (tout.size() != lout.size())
        msg("output tensor list does not match with defined output layers", "Net.fit");

    // Check if all the data inputs has the same number of samples
    n = tin[0]->shape[0];
    for (i = 1; i < tin.size(); i++)
        if (tin[i]->shape[0] != n)
            msg("different number of samples in input tensor", "Net.fit");

    // Check if the size of the input layers matches with the batch size
    for (i = 0; i < lin.size(); i++)
        if (lin[i]->input->shape[0] != batch_size)
            msg("different number of samples in input tensor w.r.t batch size", "Net.fit");

    // Check if the size of the output layers matches with the batch size
    for (i = 1; i < tout.size(); i++)
        if (tout[i]->shape[0] != n)
            msg("different number of samples in output tensor", "Net.fit");

    // Create internal variables

    // Create array to store batch indices (later random)
    vind sind;
    for (i = 0; i < batch_size; i++)
        sind.push_back(0);

    // Store errors of each output layer
    verr total_loss;
    verr total_metric;
    for (i = 0; i < tout.size(); i++) {
        total_loss.push_back(0.0);
        total_metric.push_back(0.0);
    }

    // Start training
    setmode(TRMODE);

    // Set some parameters
    int num_batches = n / batch_size;

    // Train network
    fprintf(stdout, "%d epochs of %d batches of size %d\n", epochs, num_batches, batch_size);
    for (i = 0; i < epochs; i++) {
        high_resolution_clock::time_point e1 = high_resolution_clock::now();
        fprintf(stdout, "Epoch %d\n", i + 1);

        // Reset errors
        for (j = 0; j < tout.size(); j++){
            total_loss[j] = 0.0;
            total_metric[j] = 0.0;
        }

        // For each batch
        for (j = 0; j < num_batches; j++) {

            // Set random indices
            for (k = 0; k < batch_size; k++) sind[k] = rand() % n;

            // Train batch
            high_resolution_clock::time_point t1 = high_resolution_clock::now();
            train_batch(tin, tout, sind, batch_size);
            high_resolution_clock::time_point t2 = high_resolution_clock::now();
            duration<double> time_span = t2 - t1;

            // Print errors
            int p = 0;
            fprintf(stdout, "batch %d ", j + 1);
            for (k = 0; k < tout.size(); k++, p += 2) {
                total_loss[p] += fiterr[p];  // loss
                total_metric[p] += fiterr[p + 1];  // metric

                fprintf(stdout, "%s(%s=%1.3f,%s=%1.3f) ", lout[k]->name.c_str(),
                        losses[k]->name.c_str(), total_loss[p] / (batch_size * (j + 1)),
                        metrics[k]->name.c_str(), total_metric[p] / (batch_size * (j + 1)));

                fiterr[p] = fiterr[p + 1] = 0.0;
            }
            fprintf(stdout, "%1.3f secs/batch\n", time_span.count());
            fflush(stdout);
        }

        high_resolution_clock::time_point e2 = high_resolution_clock::now();
        duration<double> epoch_time_span = e2 - e1;
        fprintf(stdout, "\n%1.3f secs/epoch\n", epoch_time_span.count());
    }
    fflush(stdout);
}


void Net::train_batch_ni(vector<Tensor *> X, vector<Tensor *> Y) {
    vind sind;

    // Check shape
    if (X.size() != lin.size()){
        msg("input tensor list does not match", "Net.train_batch");
    }

    if (Y.size() != lout.size()) {
        msg("output tensor list does not match", "Net.train_batch");
    }

    for (int i = 0; i < lin.size(); i++) {
        if (!Tensor::eqsize(lin[i]->input, X[i]))
            msg("input tensor shapes does not match", "Net.train_batch");
    }

    for (int i = 0; i < lin.size(); i++){
        if (!Tensor::eqsize(lout[i]->output, Y[i]))
            msg("output tensor shapes does not match", "Net.train_batch");
    }

    // Create indices
    for (int i = 0; i < Y[i]->shape[0]; i++)
        sind.push_back(i);

    setmode(TRMODE);
    train_batch(X, Y, sind, lin[0]->input->shape[0]);
}


/////////////////////////////////////////
void Net::train_batch(vtensor X, vtensor Y, vind sind, int batch_size, int eval) {
    void *status;
    int rc;
    pthread_t thr[100];
    struct tdata td[100];

    int thread_batch_size = batch_size / snets.size();

    // Check indices
    if (sind.size() == 0) msg("error void index","Net::train_btch");

    // Split data for each network
    for (int i = 0; i < snets.size(); i++) {
        int start = i * thread_batch_size;
        int end = start + Xs[i][0]->shape[0];

        // Copy samples
        for (int j = 0; j < X.size(); j++) {
            Tensor::select(X[j], Xs[i][j], sind, start, end);
            Tensor::copy(Xs[i][j], snets[i]->lin[j]->input);
        }

        // Copy targets
        for (int j = 0; j < Y.size(); j++) {
            Tensor::select(Y[j], Ys[i][j], sind, start, end);
            Tensor::copy(Ys[i][j], snets[i]->lout[j]->target);
        }

        // Thread params
        td[i].net = snets[i];
        td[i].batch_size = batch_size;
        td[i].eval = eval;

        // Call thread
        rc = pthread_create(&thr[i], nullptr, train_batch_t, (void *) (&td[i]));
        if (rc) {
            fprintf(stderr, "Error:unable to create thread %d", rc);
            exit(-1);
        }
    }

    // Wait until all threads have finished
    for (int i = 0; i < snets.size(); i++) {
        rc = pthread_join(thr[i], &status);
        if (rc) {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
    }

    // If training (eval==0), apply gradients
    if (!eval) {
        if (snets[0]->dev == DEV_CPU) {
            for (int i = 0; i < snets.size(); i++) {
                rc = pthread_create(&thr[i], nullptr, applygrads_t, (void *) (&td[i]));
                if (rc) {
                    fprintf(stderr, "Error:unable to create thread %d", rc);
                    exit(-1);
                }
            }
            for (int i = 0; i < snets.size(); i++) {
                rc = pthread_join(thr[i], &status);
                if (rc) {
                    cout << "Error:unable to join," << rc << endl;
                    exit(-1);
                }
            }
        }
        // In case of multiple GPUS or FPGA synchronize params
        if ((snets[0]->dev != DEV_CPU) && (snets.size() > 1)) sync_weights();
    }

    // Sum all errors
    for (int i = 0; i < snets.size(); i++) {
        for (int j = 0; j < 2 * lout.size(); j++) {
            fiterr[j] += snets[i]->fiterr[j];
        }
    }
}


/////////////////////////////////////////
void Net::sync_weights() {
    for (int j = 0; j < layers.size(); j++)
        for (int k = 0; k < layers[j]->params.size(); k++) {
            // Taking average
            layers[j]->params[k]->set(0.0);
            for (int i = 0; i < snets.size(); i++) {
                Tensor::inc(snets[i]->layers[j]->params[k], layers[j]->params[k]);
            }
            layers[j]->params[k]->div(snets.size());

            // copy-back to devices
            for (int i = 0; i < snets.size(); i++) {
                Tensor::copy(layers[j]->params[k], snets[i]->layers[j]->params[k]);
            }
        }
}

///////////////////////////////////////////

void Net::evaluate(vtensor tin, vtensor tout) {

    int i, j, k, n;

    // Check list shape
    if (tin.size() != lin.size())
        msg("input tensor list does not match with defined input layers", "Net.evaluate");
    if (tout.size() != lout.size())
        msg("output tensor list does not match with defined output layers", "Net.evaluate");


    // Check data consistency
    n = tin[0]->shape[0];
    for (i = 1; i < tin.size(); i++)
        if (tin[i]->shape[0] != n)
            msg("different number of samples in input tensor", "Net.evaluate");


    int batch = lin[0]->input->shape[0];
    for (i = 1; i < lin.size(); i++)
        if (lin[i]->input->shape[0] != batch)
            msg("different number of input tensors w.r.t", "Net.evaluate");

    for (i = 1; i < tout.size(); i++)
        if (tout[i]->shape[0] != n)
            msg("different number of samples in output tensor", "Net.evaluate");

    // Create internal variables
    vind sind;
    for (k=0;k<batch;k++)
      sind.push_back(0);

    verr errors;
    for (i = 0; i < tout.size(); i++) {
        errors.push_back(0.0);
        errors.push_back(0.0);
    }

    // Start eval
    int p=0;
    for (j = 0; j < 2 * tout.size(); j++,p+=2) {fiterr[p] = fiterr[p + 1] = 0.0;errors[j] = 0.0;}

    setmode(TSMODE);
    for (j = 0; j < n / batch; j++) {

        for (k=0;k<batch;k++)
          sind[k]=(j*batch)+k;

        train_batch(tin, tout, sind, batch, 1);
        p = 0;
        for (k = 0; k < tout.size(); k++, p += 2) {
            errors[p] += fiterr[p];
            errors[p + 1] += fiterr[p + 1];
            fiterr[p] = fiterr[p + 1] = 0.0;
        }
    }

    p = 0;
    for (k = 0; k < tout.size(); k++, p += 2)
        fprintf(stdout, "%s(%s=%1.3f,%s=%1.3f) ", lout[k]->name.c_str(), losses[k]->name.c_str(), errors[p] / n,
                metrics[k]->name.c_str(), errors[p + 1] / n);
    fprintf(stdout, "\n");
    fflush(stdout);

}


//////
