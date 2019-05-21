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
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "../layer.h"

using namespace std;


// ---- MAXPOOL2D ----
// constructors and clones
LMaxPool::LMaxPool(Layer *parent, const initializer_list<int> &ks, const initializer_list<int> &st, string p, string name,
               int d) : LMaxPool(parent, new PoolDescriptor(ks, st, p), name, d) {}

LMaxPool::LMaxPool(Layer *parent, const initializer_list<int> &ks, const initializer_list<int> &st,
               const initializer_list<int> &p, string name, int d) : LMaxPool(parent, new PoolDescriptor(ks, st, p), name, d) {}

LMaxPool::LMaxPool(Layer *parent, const vector<int> &ks, const vector<int> &st, string p, string name, int d) : LMaxPool(
        parent, new PoolDescriptor(ks, st, p), name, d) {}

LMaxPool::LMaxPool(Layer *parent, PoolDescriptor *D, string name, int d) : LPool(parent, D, name, d) {
    // params
    D->indX = new Tensor(D->O->getShape(), d);
    D->indY = new Tensor(D->O->getShape(), d);
}


void LMaxPool::forward() {
    Tensor::MPool2D(pd);
}

void LMaxPool::backward() {
    // backprop delta
    if (parent.size()) {
        Tensor::MPool2D_back(pd);
    }
}

Layer *LMaxPool::share(int c, int bs, vector<Layer *> p) {
    LMaxPool *n = new LMaxPool(p[0], {pd->kr, pd->kc}, {pd->sr, pd->sc}, {pd->padr, pd->padc},
                           "share_" + to_string(c) + name, dev);
    n->orig = this;

    return n;
}

Layer *LMaxPool::clone(int c, int bs, vector<Layer *> p, int todev) {
    LMaxPool *n = new LMaxPool(p[0], {pd->kr, pd->kc}, {pd->sr, pd->sc}, {pd->padr, pd->padc},
                           "clone_" + to_string(todev) + name, todev);
    n->orig = this;

    return n;
}

string LMaxPool::plot(int c) {
    string s;

    if (c) s = name + " [label=" + "\"" + name + "\",style=filled,fontsize=12,fillcolor=gray,shape=box]";
    else s = name + " [label=" + "\"" + name + "\",style=filled,fontsize=12,fillcolor=red,shape=box]";

    return s;
}