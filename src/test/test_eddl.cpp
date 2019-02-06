// This file is part of EDDLL an European Distributed Deep Learning Library.
// Developed within the DeepHealth project.
// Boosting AI in Europe.
//
// The MIT License (MIT)
//
// Copyright (c) 2019 Roberto Paredes Palacios, <rparedes@dsic.upv.es>

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

#include "../net.h"

int main(int argc, char **argv)
{
  int batch=100;

  Tensor *tin=new Tensor({batch,784});
  Input* in=new Input(tin);
  Dense* d1=new Dense(in,512);
  Dense* d2=new Dense(d1,256);
  Dense* d3=new Dense(d2,128);
  Dense* out=new Dense(d3,10);

  // define input and output layers list
  Net *net=new Net({in},{out});

  net->info();

  // Attach an optimizer and a list of error criteria
  net->build(SGD(0.01,0.9),{"mse"});

  /// read data somewhere
  Tensor *X=new Tensor({60000,784});
  Tensor *Y=new Tensor({60000,10});

  // fit, list of input and output tensors, batch, epochs
  net->fit({X},{Y},batch,1);


}


















  ///////////
