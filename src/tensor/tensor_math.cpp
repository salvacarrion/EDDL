/*
* EDDL Library - European Distributed Deep Learning Library.
* Version: 0.2
* copyright (c) 2019, Universidad Politécnica de Valencia (UPV), PRHLT Research Centre
* Date: October 2019
* Author: PRHLT Research Centre, UPV, (rparedes@prhlt.upv.es), (jon@prhlt.upv.es)
* All rights reserved
*/

#include <cmath>
#include <limits>
#include <iostream>

#include "tensor.h"
#include "../hardware/cpu/cpu_hw.h"

#ifdef cGPU
#include "../hardware/gpu/gpu_tensor.h"
#include "../hardware/gpu/gpu_hw.h"
#include "../hardware/gpu/nn/gpu_nn.h"
#endif

#ifdef cFPGA
#include "../hardware/fpga/tensor_hls_op.h"
#endif

using namespace std;

ProfilerStorage mult2d_ps("mult2d");

void Tensor::abs_() {
    if (isCPU()) {
        cpu_abs_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_abs_(this);
      }
#endif
#ifdef cFPGA
    else {
       msg("Tensor ABS not implemented for FPGA yet\n");
    }
#endif
}

Tensor* Tensor::abs(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->abs_();
    return t_new;
}

void Tensor::acos_(){
    if (isCPU()) {
        cpu_acos_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_acos_(this);
      }
#endif
#ifdef cFPGA
    else {
       msg("acos not implemented for FPGA yet\n");
    }
#endif
}

Tensor* Tensor::acos(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->acos_();
    return t_new;
 }

void Tensor::add_(float v) {
    if (isCPU()) {
        cpu_add_(this, v);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_add_(this, v);
      }
#endif
#ifdef cFPGA
    else if (isFPGA())
      {
        //tensor_op_hls(this,v,FPGASUM);
        Tensor *nA=new Tensor(this->getShape(),DEV_CPU);
        fpga_copy_from_fpga(this, nA->ptr);
        cpu_add_(nA, v);
        fpga_copy_to_fpga(nA->ptr, this);             
        delete nA; 
      }
#endif
}

void Tensor::add_(Tensor *A){
    Tensor::add(1.0f, this, 1.0f, A, this, 0);
}

Tensor* Tensor::add(Tensor *A, Tensor *B){
    Tensor *t_new = A->clone();
    add(1.0f, A, 1.0f, B, t_new, 0);
    return t_new;
 }

void Tensor::add(float scA, Tensor *A, float scB, Tensor *B, Tensor *C, int incC) {
    ///////////////////////////////////////
    //// sum C=(sca*A)+(scb*B)
    //// or C+=(sca*A)+(scb*B) if incC is 1
    //// Dimensions and types must be compatible
    ///////////////////////////////////////
    int aux = 0;


    if ((A->device != B->device) || (A->device != C->device)) msg("Tensors in different devices", "Tensor::add_");
    if ((!eqsize(A, B)) || (!eqsize(A, C))) {
        A->info();
        B->info();
        C->info();
        msg("Incompatible dims", "Tensor::add");
    }

    C->tsem->lock();
    if (A->isCPU()) {
      cpu_add(scA, A, scB, B, C, incC);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_addc(scA,A,scB,B,C,incC);
      }
#endif
#ifdef cFPGA
    else if (A->isFPGA())
      {
        //msg("Not implemented for FPGA yet", "Tensor::add");
        fpga_tensor_add(scA, A, scB, B, C, incC);
        //printf("FPGA::ADD\n");
        // Tensor *nA=new Tensor(A->getShape(),DEV_CPU);
        // Tensor *nB=new Tensor(B->getShape(),DEV_CPU);
        // Tensor *nC=new Tensor(C->getShape(),DEV_CPU);
        // fpga_copy_from_fpga(A, nA->ptr);
        // fpga_copy_from_fpga(B, nB->ptr);
        // fpga_copy_from_fpga(C, nC->ptr);
        // cpu_add(scA,nA,scB, nB,nC,incC);
        // fpga_copy_to_fpga(nA->ptr, A);
        // fpga_copy_to_fpga(nB->ptr, B);
        // fpga_copy_to_fpga(nC->ptr, C);
        // delete nA;
        // delete nB;
        // delete nC;
      }
#endif

    C->tsem->unlock();
}
void Tensor::add(Tensor *A, Tensor *B, Tensor *C) {
    Tensor::add(1.0, A, 1.0, B, C, 0);
}

void Tensor::inc(Tensor *A, Tensor *B) {
    // TODO: Review against add

    if (!Tensor::eqsize(A, B))
        msg("Tensors with different shape", "Tensor::inc");


    if ((A->isCPU()) && (B->isCPU())) {
        cpu_inc(A, B);
    }
#ifdef cGPU
        else if ((A->isGPU())&&(B->isGPU())) {
        Tensor::add(1,A,1,B,B,0);
    }
    else if (((A->isCPU())&&(B->isGPU()))||((A->isGPU())&&(B->isCPU())))
    {
        Tensor *n=new Tensor(B->getShape(),B->device);
        Tensor::copy(A,n);
        Tensor::add(1,n,1,B,B,0);
        delete n;
    }
#endif
#ifdef cFPGA
    else if ((A->isFPGA())&&(B->isFPGA())) {
        Tensor::add(1,A,1,B,B,0); 
    }
#endif
    else
    {
        fprintf(stderr,"(%d %d)\n",A->device,B->device);
        msg("unsupported copy between devices","Tensor::inc");
    }
}

void Tensor::asin_(){
    if (isCPU()) {
        cpu_asin_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_asin_(this);
      }
#endif
#ifdef cFPGA
    else {
       msg("Not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::asin(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->asin_();
    return t_new;
 }

void Tensor::atan_(){
    if (isCPU()) {
        cpu_atan_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_atan_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("atan not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::atan(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->atan_();
    return t_new;
 }

void Tensor::ceil_(){
    if (isCPU()) {
        cpu_ceil_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_ceil_(this);
      }
#endif
#ifdef cFPGA
    else {
       msg("ceil not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::ceil(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->ceil_();
    return t_new;
 }

void Tensor::clamp_(float min, float max){
    if (isCPU()) {
        cpu_clamp_(this, min, max);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_clamp_(this, min, max);
      }
#endif
#ifdef cFPGA
    else {
      msg("clamp no ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::clamp(Tensor *A, float min, float max){
    Tensor *t_new = A->clone();
    t_new->clamp_(min, max);
    return t_new;
 }

void Tensor::clampmax_(float max){ clamp_(MIN_FLOAT, max); }

Tensor* Tensor::clampmax(Tensor *A, float max){
    Tensor *t_new = A->clone();
    t_new->clampmax_(max);
    return t_new;
}

void Tensor::clampmin_(float min){ clamp_(min, MAX_FLOAT); }
Tensor* Tensor::clampmin(Tensor *A, float min){
    Tensor *t_new = A->clone();
    t_new->clampmin_(min);
    return t_new;
}

void Tensor::cos_(){
    if (isCPU()) {
        cpu_cos_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_cos_(this);
      }
#endif
#ifdef cFPGA
    else {
       msg("cos ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::cos(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->cos_();
    return t_new;
}

void Tensor::cosh_(){
    if (isCPU()) {
        cpu_cosh_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_cosh_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("cosh ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::cosh(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->cosh_();
    return t_new;
}


void Tensor::div_(float v) { mult_(1.0f / v); }

void Tensor::inv_() {
  if (isCPU()) {
      cpu_inv_(this);
  }
  #ifdef cGPU
  else if (isGPU())
    {
      gpu_inv_(this);
    }
  #endif
  #ifdef cFPGA
  else {
      msg("Inv not ready for FPGA yet");
  }
  #endif
}


Tensor* Tensor::div(Tensor *A, float v){
    Tensor *t_new = A->clone();
    t_new->div_(v);
    return t_new;
}

void Tensor::el_div(Tensor *A, Tensor *B, Tensor *C, int incC) {
    ///////////////////////////////////////
    //// Element Div C=A./B
    //// incC 1 means C+=A./B (increment over C)
    //// Dimensions must be compatible
    ///////////////////////////////////////

    if ((A->device != B->device) || (A->device != C->device)) msg("Tensors in different devices", "Tensor::el_div");
    if ((!eqsize(A, B)) || (!eqsize(A, C))) msg("Incompatible dims", "Tensor::el_div");

    C->tsem->lock();
    if (A->isCPU()) {
        cpu_el_div(A, B, C, incC);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_el_div(A,B,C,incC);
      }
#endif
#ifdef cFPGA
    else if (A->isFPGA())
      {
        //fpga_el_div_mult(A,B,C,incC, 0);
        Tensor *nA=new Tensor(A->getShape(),DEV_CPU);
        Tensor *nB=new Tensor(B->getShape(),DEV_CPU);
        Tensor *nC=new Tensor(C->getShape(),DEV_CPU);
        fpga_copy_from_fpga(A, nA->ptr);
        fpga_copy_from_fpga(B, nB->ptr);
        fpga_copy_from_fpga(C, nC->ptr);
        cpu_el_div(nA, nB, nC, incC);
        fpga_copy_to_fpga(nA->ptr, A);
        fpga_copy_to_fpga(nB->ptr, B);
        fpga_copy_to_fpga(nC->ptr, C);
        delete nA;
        delete nB;
        delete nC;
      }
#endif
    C->tsem->unlock();
}

void Tensor::exp_() {
    if (isCPU()) {
        cpu_exp_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_exp_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("exp not ready for FPGA yet");
    }
#endif
}


Tensor* Tensor::exp(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->exp_();
    return t_new;
}

void Tensor::floor_(){
    if (isCPU()) {
        cpu_floor_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_floor_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("floor not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::floor(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->floor_();
    return t_new;
}


void Tensor::log_() {
    if (isCPU()) {
        cpu_log_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_log_(this);
      }
#endif
#ifdef cFPGA
    else {
       msg("Log not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::log(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->log_();
    return t_new;
}

void Tensor::log2_() {
    if (isCPU()) {
        cpu_log2_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_log2_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("Log2 not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::log2(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->log2_();
    return t_new;
}


void Tensor::log10_() {
    if (isCPU()) {
        cpu_log10_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_log10_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("Log10 not ready for FPGA yet"); 
    }
#endif
}

Tensor* Tensor::log10(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->log10_();
    return t_new;
}


void Tensor::logn_(float n) {
    if (isCPU()) {
        cpu_logn_(this, n);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_logn_(this, n);
      }
#endif
#ifdef cFPGA
    else {
        msg("Log10 not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::logn(Tensor *A, float n){
    Tensor *t_new = A->clone();
    t_new->logn_(n);
    return t_new;
}

float Tensor::max(){
    if (isCPU()) {
        return cpu_max(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        return gpu_max(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("Max not ready for FPGA yet");
    }
#endif
    return -1.0f;  // Temp
}

float Tensor::min(){
    if (isCPU()) {
        return cpu_min(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        return gpu_min(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("min not ready for FPGA yet");
    }
#endif
    return -1.0f;  // Temp
}


void Tensor::mod_(float v){
    if (isCPU()) {
        cpu_mod_(this, v);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_mod_(this, v);
      }
#endif
#ifdef cFPGA
    else {
        msg("mod not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::mod(Tensor *A, float v){
    Tensor *t_new = A->clone();
    t_new->mod_(v);
    return t_new;
};

void Tensor::mult_(float v) {
    if (isCPU()) {
        cpu_mult_(this, v);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_mult_(this, v);
      }
#endif
#ifdef cFPGA
    else {
        //tensor_op_hls(this, v,FPGAMULT);
        //printf("FPGA::MULT\n");
        Tensor *nA=new Tensor(this->getShape(),DEV_CPU);
        fpga_copy_from_fpga(this, nA->ptr);
        cpu_mult_(nA, v);
        fpga_copy_to_fpga(nA->ptr, this);
        delete nA;
    }
#endif
}

Tensor* Tensor::mult(Tensor *A, float v){
    Tensor *t_new = A->clone();
    t_new->mult_(v);
    return t_new;
}


void Tensor::mult2D(Tensor *A, int tA, Tensor *B, int tB, Tensor *C, int incC) {
    ///////////////////////////////////////
    //// MULT2D C=A*B
    //// tA means transpose A {0,1}
    //// tB means transpose B {0,1}
    //// tC 1 means C+=A*B (increment over C)
    //// Dimensions and types must be compatible
    //// Only for 2D Tensors
    ///////////////////////////////////////

    BlockProfiler prof_(mult2d_ps);

    if ((A->device != B->device) || (A->device != C->device)) msg("Tensors in different devices", "Tensor::mult2D");
    if ((A->ndim != 2) || (B->ndim != 2) || (C->ndim != 2)) msg("Only 2D tensors", "Tensor::mult2D");
    if (!tA) {
        if (!tB) {
            if ((A->shape[1] != B->shape[0]) || (A->shape[0] != C->shape[0]) || (B->shape[1] != C->shape[1]))
                msg("Incompatible dims", "Tensor::mult2D");
        } else if ((A->shape[1] != B->shape[1]) || (A->shape[0] != C->shape[0]) || (B->shape[0] != C->shape[1]))
            msg("Incompatible dims", "Tensor::mult2D");
    } else {
        if (!tB) {
            if ((A->shape[0] != B->shape[0]) || (A->shape[1] != C->shape[0]) || (B->shape[1] != C->shape[1]))
                msg("Incompatible dims", "Tensor::mult2D");
        } else if ((A->shape[0] != B->shape[1]) || (A->shape[1] != C->shape[0]) || (B->shape[0] != C->shape[1]))
            msg("Incompatible dims", "Tensor::mult2D");
    }


    C->tsem->lock();

    if (A->isCPU()) {
        cpu_mult2D(A, tA, B, tB, C, incC);
    }

#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_mult2D(A,tA,B,tB,C,incC);
      }
#endif
#ifdef cFPGA
    else {
       //fpga_mult2D(A,tA,B,tB,C,incC);
       Tensor *nA=new Tensor(A->getShape(),DEV_CPU);
       Tensor *nB=new Tensor(B->getShape(),DEV_CPU);
       Tensor *nC=new Tensor(C->getShape(),DEV_CPU);
       fpga_copy_from_fpga(A, nA->ptr);
       fpga_copy_from_fpga(B, nB->ptr);
       fpga_copy_from_fpga(C, nC->ptr);
       cpu_mult2D(nA, tA, nB, tB, nC, incC);
       //fpga_gemx_mult2D_CPU(nA,tA, nB, tB,nC, incC);
       fpga_copy_to_fpga(nA->ptr, A);
       fpga_copy_to_fpga(nB->ptr, B);
       fpga_copy_to_fpga(nC->ptr, C);
       delete nA;
       delete nB;
       delete nC; 
    }
#endif
    C->tsem->unlock();
}


void Tensor::el_mult(Tensor *A, Tensor *B, Tensor *C, int incC) {
    ///////////////////////////////////////
    //// Element Mult C=A.*B
    //// incC 1 means C+=A.*B (increment over C)
    //// Dimensions must be compatible
    ///////////////////////////////////////
    C->tsem->lock();
    if ((A->device != B->device) || (A->device != C->device)) msg("Tensors in different devices", "Tensor::el_mult");
    if ((!eqsize(A, B)) || (!eqsize(A, C))) {
        A->info();
        B->info();
        C->info();
        msg("Incompatible dims", "Tensor::el_mult");
    }

    if (A->isCPU()) {
        cpu_el_mult(A, B, C, incC);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
         gpu_el_mult(A,B,C,incC);
      }
#endif
#ifdef cFPGA
    else {
        fpga_el_div_mult(A,B,C,incC, 1);
     }
#endif
    C->tsem->unlock();
}

void Tensor::neg_(){ mult_(-1.0f); }

Tensor* Tensor::neg(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->neg_();
    return t_new;
};


void Tensor::normalize_(float min, float max){
    // Normalize in range: 423 from [23, 562], to range [-1, 1] => 0.4842
    // (max2-min2)/(max1-min1) * (x-min1) + min2
    if (isCPU()) {
        cpu_normalize_(this, min, max);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_normalize_(this, min, max); // TODO: fix
      }
#endif
#ifdef cFPGA
    else if (isFPGA())
      {
        fpga_tensor_normalize(this, min, max);
      }
#endif
}

static Tensor* normalize(Tensor *A, float min=0.0f, float max=1.0f);


void Tensor::pow_(float exp) {
    // To compute the power, std uses real floating-point number with the formurla: e^(y*log_(x))
    // Quite inefficient (x100 slower) in g++ except for pow_(x, 2) which is inlined as x*x
    // speed: 0.057887s
    if (isCPU()) {
        cpu_pow_(this, exp);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_pow_(this, exp);
      }
#endif
#ifdef cFPGA
    else {
        msg("pow not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::pow(Tensor *A, float exp){
    Tensor *t_new = A->clone();
    t_new->pow_(exp);
    return t_new;
}

void Tensor::powb_(float base) {
    // Similar to pow (tensor^exp) but here we revert the order (base^tensor)
    if (isCPU()) {
        cpu_powb_(this, base);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_powb_(this, base);
      }
#endif
#ifdef cFPGA
    else {
        msg("powb not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::powb(Tensor *A, float base){
    Tensor *t_new = A->clone();
    t_new->powb_(base);
    return t_new;
}

void Tensor::reciprocal_() {
    if (isCPU()) {
        cpu_reciprocal_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_reciprocal_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("reciprocal not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::reciprocal(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->reciprocal_();
    return t_new;
}

void Tensor::remainder_(float v) {
    if (isCPU()) {
        cpu_remainder_(this, v);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_remainder_(this, v);
      }
#endif
#ifdef cFPGA
    else {
        msg("reminder not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::remainder(Tensor *A, float v){
    Tensor *t_new = A->clone();
    t_new->remainder_(v);
    return t_new;
}

void Tensor::round_(){
    if (isCPU()) {
        cpu_round_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_round_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("Round not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::round(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->round_();
    return t_new;
}

void Tensor::rsqrt_(){
    if (isCPU()) {
        cpu_rsqrt_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_rsqrt_(this);
      }
#endif
#ifdef cFPGA
    else {
       msg("rsqrt not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::rsqrt(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->rsqrt_();
    return t_new;
}

void Tensor::sigmoid_(){
    if (isCPU()) {
        cpu_sigmoid_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_sigmoid_(this);
      }
#endif
#ifdef cFPGA
    else {
       msg("sigmoid not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::sigmoid(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->sigmoid_();
    return t_new;
}

void Tensor::sign_(){
    if (isCPU()) {
        cpu_sign_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_sign_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("Sign not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::sign(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->sign_();
    return t_new;
}

void Tensor::sign(Tensor *A, Tensor *B) {
    ///////////////////////////////////////
    /// Get sign (+-) of all values
    //////////////////////////////////////
    B->tsem->lock();

    if (!Tensor::eqsize(A, B))
        msg("Tensors with different shape", "Tensor::sign");
    if (A->device != B->device) msg("Tensors in different devices", "Tensor::sign");

    if (A->isCPU()) {
        cpu_sign2(A, B);
    }
#ifdef cGPU
    else if (A->isGPU())
      {

      }
#endif
#ifdef cFPGA
    else {
       msg("sign not ready for FPGA yet");
    }
#endif
    B->tsem->unlock();

}

void Tensor::sin_(){
    if (isCPU()) {
        cpu_sin_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_sin_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("sin not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::sin(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->sin_();
    return t_new;
}

void Tensor::sinh_(){
    if (isCPU()) {
        cpu_sinh_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_sinh_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("sinh not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::sinh(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->sinh_();
    return t_new;
}


void Tensor::sqr_() {
    // pow(x, 2) == x*x  To know more, read comments in pow_'s function
    // speed: 0.000497s
    if (isCPU()) {
        cpu_sqr_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_sqr_(this);
      }
#endif
#ifdef cFPGA
    else if (isFPGA())
     {
        //tensor_op_hls(this, 0, FPGASQR);
        Tensor *nA=new Tensor(this->getShape(),DEV_CPU);
        fpga_copy_from_fpga(this, nA->ptr);
        cpu_sqr_(nA);
        fpga_copy_to_fpga(nA->ptr, this);
        delete nA;
     }
#endif
}

Tensor* Tensor::sqr(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->sqr_();
    return t_new;
}


void Tensor::sqrt_() {
    if (isCPU()) {
        cpu_sqrt_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_sqrt_(this);
      }
#endif
#ifdef cFPGA
    else if (isFPGA())
      {
        //tensor_op_hls(this,0,FPGASQRT); 
        Tensor *nA=new Tensor(this->getShape(),DEV_CPU);
        fpga_copy_from_fpga(this, nA->ptr);
        cpu_sqrt_(nA);
        fpga_copy_to_fpga(nA->ptr, this);
        delete nA;
      }
#endif
}

Tensor* Tensor::sqrt(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->sqrt_();
    return t_new;
}


void Tensor::sub_(float v) { add_(-v); }

Tensor* Tensor::sub(Tensor *A, Tensor *B){
    Tensor *t_new = A->clone();
    add(1.0f, A, -1.0f, B, t_new, 0);
    return t_new;
}

float Tensor::sum() {
    if (isCPU()) {
        return cpu_sum(this);
    }
#ifdef cGPU
    else if (isGPU())
    {
        return gpu_sum(this);
    }
#endif
#ifdef cFPGA
    else if (isFPGA())
    {
      return fpga_total_sum(this);
      // Tensor *nA=new Tensor(this->getShape(),DEV_CPU);
      // fpga_copy_from_fpga(this, nA->ptr);
      // float aux= cpu_sum(nA);
      // delete nA;
      // return aux;
    }
#endif
    return 0;
}

//Tensor* Tensor::sum(Tensor *A){}

void Tensor::sum2D_rowwise(Tensor *A, Tensor *B, Tensor *C) {
    ///////////////////////////////////////
    //// sum2D_rowise C=A.rowise+B
    //// Dimensions and types must be compatible
    //// A is 2D Tensor
    //// B is 1D Tensor
    ///////////////////////////////////////
    if ((A->device != B->device) || (A->device != C->device))
        msg("Tensors in different devices", "Tensor::sum2D_rowwise");
    if ((A->ndim != 2) || (B->ndim != 1) || (C->ndim != 2)) msg("sum2D_rowwise dims");
    if ((!eqsize(A, C)) || (A->shape[1] != B->shape[0])) msg("Incompatible dims", "Tensor::sum2D_rowwise");

    C->tsem->lock();
    if (A->isCPU()) {
        cpu_sum2D_rowwise(A, B, C);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_sum2D_rowwise(A,B,C);
      }
#endif
#ifdef cFPGA
    else {
        fpga_sum2D_rowwise(A,B,C);
        // Tensor *nA=new Tensor(A->getShape(),DEV_CPU);
        // Tensor *nB=new Tensor(B->getShape(),DEV_CPU);
        // Tensor *nC=new Tensor(C->getShape(),DEV_CPU);
        // fpga_copy_from_fpga(A, nA->ptr);
        // fpga_copy_from_fpga(B, nB->ptr);
        // fpga_copy_from_fpga(C, nC->ptr);
        // cpu_sum2D_rowwise(nA, nB, nC);
        // fpga_copy_to_fpga(nA->ptr, A);
        // fpga_copy_to_fpga(nB->ptr, B);
        // fpga_copy_to_fpga(nC->ptr, C);
        // delete nA;
        // delete nB;
        // delete nC;
    }
#endif
    C->tsem->unlock();
}


void Tensor::reduce_sum2D(Tensor *A, Tensor *B, int axis, int incB) {
    ///////////////////////////////////////
    //// reduce_sum2D B=reduce_sum2D(A)
    //// Dimensions and types must be compatible
    //// A is 2D Tensor
    //// B is 1D Tensor
    //// axis is the dimension to be sumed
    ///////////////////////////////////////
    if (A->device != B->device) msg("Tensors in different devices", "Tensor::reduce_sum2D");
    if ((A->ndim - 1) != B->ndim) msg("Incorrect dims", "Tensor::reduce_sum2D");
    if ((A->shape[1 - axis] != B->shape[0])) msg("Incompatible dims", "Tensor::reduce_sum2D");

    B->tsem->lock();
    if (A->isCPU()) {
        cpu_reduce_sum2D(A, B, axis, incB);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_reduce_sum2D(A,B,axis,incB);
      }
#endif
#ifdef cFPGA
    else if (A->isFPGA())
     {
       fpga_reduce_sum2D(A,B,axis,incB);
       //printf("FPGA::REDUCESUM2D\n");
       // Tensor *nA=new Tensor(A->getShape(),DEV_CPU);
       // Tensor *nB=new Tensor(B->getShape(),DEV_CPU);
       // fpga_copy_from_fpga(A, nA->ptr);
       // fpga_copy_from_fpga(B, nB->ptr);
       // cpu_reduce_sum2D(nA, nB, axis, incB);
       // fpga_copy_to_fpga(nA->ptr, A);
       // fpga_copy_to_fpga(nB->ptr, B);
       // delete nA;
       // delete nB;
     }
#endif
    B->tsem->unlock();
}

void Tensor::reduceTosum(Tensor *A, Tensor *B, int axis) {
    //
    // Sum all the axis of A in B
    //
    // TODO: Review cost (l1/l2)
    B->tsem->lock();

    if (A->device != B->device) msg("Tensors in different devices", "Tensor::transpose");

    B->fill_(0.0);
    if (A->isCPU()) {
        cpu_reduceTosum(A, B, axis);
    }
#ifdef cGPU
    else if (A->isGPU())
      {

      }
#endif
#ifdef cFPGA
    else {
    
    }
#endif
    B->tsem->unlock();

}


void Tensor::sum2D_colwise(Tensor *A, Tensor *B, Tensor *C) {
    ///////////////////////////////////////
    //// sum2D_colwise C=A.colwise+B
    //// Dimensions and types must be compatible
    //// A is 2D Tensor
    //// B is 1D Tensor
    ///////////////////////////////////////
    if ((A->device != B->device) || (A->device != C->device))
        msg("Tensors in different devices", "Tensor::sum2D_colwise");
    if ((A->ndim != 2) || (B->ndim != 1) || (C->ndim != 2)) msg("sum2D_colwise dims");
    if ((!eqsize(A, C)) || (A->shape[0] != B->shape[0])) msg("Incompatible dims", "Tensor::sum2D_colwise");

    C->tsem->lock();
    if (A->isCPU()) {
        cpu_sum2D_colwise(A, B, C);
    }
#ifdef cGPU
    else if (A->isGPU())
      {
        gpu_sum2D_colwise(A,B,C);
      }
#endif
#ifdef cFPGA
    else {
        msg("Sum2Dcolwise not ready for FPGA yet");
    }
#endif
    C->tsem->unlock();
}

float Tensor::sum_abs() {
    if (isCPU()) {
        return cpu_sum_abs(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
         //return gpu_sum_abs(this);
      }
#endif
#ifdef cFPGA
    else {
         msg("sumabs not ready for FPGA yet");
    }
#endif
    return 0;
}

//Tensor* Tensor::sum_abs(Tensor *A){}

void Tensor::tan_(){
    if (isCPU()) {
        cpu_tan_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
       gpu_tan_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("Tan not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::tan(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->tan_();
    return t_new;
}

void Tensor::tanh_(){
    if (isCPU()) {
        cpu_tanh_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_tanh_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("tanh not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::tanh(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->tanh_();
    return t_new;
}


void Tensor::trunc_(){
    if (isCPU()) {
        cpu_trunc_(this);
    }
#ifdef cGPU
    else if (isGPU())
      {
        gpu_trunc_(this);
      }
#endif
#ifdef cFPGA
    else {
        msg("trunc not ready for FPGA yet");
    }
#endif
}

Tensor* Tensor::trunc(Tensor *A){
    Tensor *t_new = A->clone();
    t_new->trunc_();
    return t_new;
}


Tensor* Tensor::interpolate(float factor1, Tensor *A, float factor2, Tensor *B){
    Tensor* C = Tensor::zeros(A->getShape());
    Tensor::add(factor1, A, factor2, B, C, 1);
    return C;
}
