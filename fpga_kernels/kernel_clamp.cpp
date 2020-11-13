#include <math.h>
#include <stdio.h>
extern "C" {

#define DATA_SIZE 4096
#define BUFFER_SIZE 1024

// TRIPCOUNT identifiers
const unsigned int c_chunk_sz = BUFFER_SIZE;
const unsigned int c_size = DATA_SIZE;

void k_clamp(float *A, float *B, float min, float max, long int size) {

  #pragma HLS INTERFACE m_axi port=A offset=slave bundle=gmem
  #pragma HLS INTERFACE m_axi port=B offset=slave bundle=gmem
  #pragma HLS INTERFACE s_axilite port=A  bundle=control
  #pragma HLS INTERFACE s_axilite port=B  bundle=control
  #pragma HLS INTERFACE s_axilite port=min bundle=control
  #pragma HLS INTERFACE s_axilite port=max bundle=control
  #pragma HLS INTERFACE s_axilite port=size bundle=control
  #pragma HLS INTERFACE s_axilite port=return bundle=control

  float buffer_a[BUFFER_SIZE];
  float buffer_b[BUFFER_SIZE];

  for (int i=0; i<size; i=i+BUFFER_SIZE) {

    #pragma HLS LOOP_TRIPCOUNT min=c_size/c_chunk_sz max=c_size/c_chunk_sz
    int chunk_size = BUFFER_SIZE;
    // boundary checks
    if ((i + BUFFER_SIZE) > size)
      chunk_size = size - i;

    // burst read of data vector from global memory
    read1:
    for (int j=0; j<chunk_size; j++) {
      #pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz max=c_chunk_sz
      buffer_a[j] = A[i + j];
    }

    /*for (int i = 0; i < A->size; ++i){
      if (A->ptr[i] < min){
        B->ptr[i] = min;
      } else if(A->ptr[i] > max){
        B->ptr[i] = max;
      }else {
        B->ptr[i] = A->ptr[i];
      }
    }*/
    clamp:
    for (int j=0; j<chunk_size; j++) {
      #pragma HLS PIPELINE II=1
      #pragma HLS UNROLL FACTOR=2
      #pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz max=c_chunk_sz
      // perform operation
      // NO support for native_fabs 
      if (buffer_a[j] < min) buffer_b[j] = min;
      else if (buffer_a[j] > max) buffer_b[j] = max;
      else buffer_b[j] = buffer_a[j];

    }

    // burst write the result
    write:
    for (int j=0; j<chunk_size; j++) {
      #pragma HLS LOOP_TRIPCOUNT min=c_chunk_sz max=c_chunk_sz
      B[i+j] = buffer_b[j];
    }
  }
} // end kernel function

} // end extern "C"