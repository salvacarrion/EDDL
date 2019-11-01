/*
* EDDL Library - European Distributed Deep Learning Library.
* Version: 0.1
* copyright (c) 2019, Universidad Politécnica de Valencia (UPV), PRHLT Research Centre
* Date: October 2019
* Author: PRHLT Research Centre, UPV, (rparedes@prhlt.upv.es), (jon@prhlt.upv.es)
* All rights reserved
*/

#include <cmath>
#include <limits>
#include <iostream>
#include <utility>

#include "tensor.h"
#include "../hardware/cpu/cpu_hw.h"

#ifdef cGPU
#include "../hardware/gpu/gpu_tensor.h"
#include "../hardware/gpu/gpu_hw.h"
#include "../hardware/gpu/nn/gpu_nn.h"
#endif


using namespace std;


void Tensor::shift(Tensor *A, Tensor *B, vector<int> shift, string mode, float constant){
    if (A->isCPU()) {
        cpu_shift(A, B, std::move(shift), get_mode(std::move(mode)), constant);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_shift(A, B, std::move(shift), get_mode(std::move(mode)), constant);
      }
#endif
#ifdef cFPGA
    else {

    }
#endif
}



void Tensor::rotate(Tensor *A, Tensor *B, float angle, vector<int> axis, string mode, float constant) {
    if (A->isCPU()) {
        cpu_rotate(A, B, angle, std::move(axis), get_mode(std::move(mode)), constant);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        msg("Only implemented for CPU Tensors", "Tensor::rotate");
      }
#endif
#ifdef cFPGA
    else {

    }
#endif
}

void Tensor::scale(Tensor *A, Tensor *B, string mode, float constant) {
    if (A->isCPU()) {
        cpu_scale(A, B, get_mode(std::move(mode)), constant);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_scale(A, B, get_mode(std::move(mode)), constant);
      }
#endif
#ifdef cFPGA
    else {

    }
#endif
}


void Tensor::flip(Tensor *A, Tensor *B, int axis) {
    if (A->isCPU()) {
        cpu_flip(A, B, axis);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_flip(A, B, axis);
      }
#endif
#ifdef cFPGA
    else {

    }
#endif
}

void Tensor::crop(Tensor *A, Tensor *B, vector<int> coords_from, vector<int> coords_to, float constant) {
    if (A->isCPU()) {
        cpu_crop(A, B, std::move(coords_from), std::move(coords_to), constant);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_crop(A, B, std::move(coords_from), std::move(coords_to), constant);
      }
#endif
#ifdef cFPGA
    else {

    }
#endif
}


void Tensor::cutout(Tensor *A, Tensor *B, vector<int> coords_from, vector<int> coords_to, float constant) {
    if (A->isCPU()) {
        cpu_cutout(A, B, std::move(coords_from), std::move(coords_to), constant);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_cutout(A, B, std::move(coords_from), std::move(coords_to), constant);
      }
#endif
#ifdef cFPGA
    else {

    }
#endif
}
