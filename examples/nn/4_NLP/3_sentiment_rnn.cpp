/*
* EDDL Library - European Distributed Deep Learning Library.
* Version: 0.3
* copyright (c) 2019, Universidad Politécnica de Valencia (UPV), PRHLT Research Centre
* Date: October 2019
* Author: PRHLT Research Centre, UPV, (rparedes@prhlt.upv.es), (jon@prhlt.upv.es)
* All rights reserved
*/

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "eddl/apis/eddl.h"
#include "eddl/apis/eddlT.h"

using namespace eddl;

//////////////////////////////////
// Embeding+CNN for
// aclImdb sentiment analysis
//////////////////////////////////

int main(int argc, char **argv) {
    // Download aclImdb
    download_imdb();

    // Settings
    int epochs = 1000;
    int batch_size = 100;
    int num_classes = 2;

    int length=100;
    int embdim=250;
    int vocsize=75181;

    // Define network
    layer in = Input({1}); //1 word
    layer l = in;

    layer lE = Embedding(l, vocsize, 1, embdim);

    set_trainable(lE,false);

    l = BatchNormalization(RNN(lE,128,"relu"));
    l = ReLu(BatchNormalization(Dense(l,64)));

    layer out = Softmax(Dense(l, num_classes));
    model net = Model({in}, {out});

    // dot from graphviz should be installed:
    plot(net, "model.pdf");

    // Build model
    build(net,
          sgd(0.001), // Optimizer
          {"soft_cross_entropy"}, // Losses
          {"categorical_accuracy"}, // Metrics
          CS_GPU({1}) // one GPU
          //CS_GPU({1,1},100) // two GPU
          //CS_CPU(-1) // CPU with maximum threads availables
    );
    //toGPU(net,{1},100,"low_mem"); // In two gpus, syncronize every 100 batches, low_mem setup

    // View model
    summary(net);

    // Load dataset
    tensor x_train=eddlT::load("imdb_trX.bin");
    //x_train->info();
    tensor y_train=eddlT::load("imdb_trY.bin");
    //y_train->info();
    // Load dataset
    tensor x_test=eddlT::load("imdb_tsX.bin");
    //x_train->info();
    tensor y_test=eddlT::load("imdb_tsY.bin");
    //y_train->info();

    // Train model
    tensor E=eddlT::load("embedding.bin");
    E->info();

    Tensor::copy(E,lE->params[0]);
    distributeTensor(lE,"param",0);

    x_train->reshape_({x_train->shape[0],length,1}); //batch x timesteps x input_dim
    x_test->reshape_({x_test->shape[0],length,1}); //batch x timesteps x input_dim

    fit(net, {x_train}, {y_train}, batch_size, epochs);
    evaluate(net,{x_test},{y_test});


}















/////
