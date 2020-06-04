/*
* FPGA support for EDDL Library - European Distributed Deep Learning Library.
* Version: 0.6
* copyright (c) 2020, Universidad Politécnica de Valencia (UPV), GAP research group
* Date: June 2020
* Author: GAP Research Group (UPV), contact: carlherlu@gap.upv.es, jflich@disca.upv.es
* All rights reserved
*/

#include "eddl/hardware/fpga/fpga_hw.h"

// emulation switches of functions (via cpu)
// when set the function is run on the cpu
char fpga_set_cpuemu_range_     = 1;
char fpga_set_cpuemu_eye_       = 1;

// -----------------------------------------------------------------
// range
//
void fpga_cpuemu_range_(Tensor *A, float min, float step) {
    printf("fpga_cpuemu_range_ not implemented yet\n");
    exit(1);
}

void fpga_range(Tensor *A, float min, float step){
    _profile_fpga(_FPGA_RANGE, 0);
    if (fpga_set_cpuemu_range_ == 1) {
        fpga_cpuemu_range_(A, min, step);
    } else {
        printf("fpga_range_ not implemented yet\n"); exit(1);
    }
    _profile_fpga(_FPGA_RANGE, 1);
}

// -----------------------------------------------------------------
// eye
//
void fpga_cpuemu_eye_(Tensor *A, int offset) {
    printf("fpga_cpuemu_eye_ not implemented yet\n");
    exit(1);
}

void fpga_eye(Tensor *A, int offset){
    _profile_fpga(_FPGA_EYE, 0);
    if (fpga_set_cpuemu_eye_ == 1) {
        fpga_cpuemu_eye_(A, offset);
    } else {
        printf("fpga_eye not implemented yet\n"); exit(1);
    }
    _profile_fpga(_FPGA_EYE, 1);
}