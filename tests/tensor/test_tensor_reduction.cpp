#include <gtest/gtest.h>
#include <random>
#include <string>
#include <ctime>

#include "eddl/tensor/tensor.h"
#include "eddl/tensor/tensor_reduction.h"
#include "eddl/tensor/nn/tensor_nn.h"
#include "eddl/descriptors/descriptors.h"


using namespace std;



TEST(TensorTestSuite, tensor_math_reduction_max) {
    // Test #1
    Tensor *t1_ref = new Tensor({5.0f, 6.0f, 9.0f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({
                                    1.0f, 4.0f, 4.0f,
                                    5.0f, 4.0f, 8.0f,
                                    2.0f, -3.0f, 9.0f,
                                    1.0f, 6.0f, 0.0f}, {4, 3}, DEV_CPU);

    Tensor *new_t = t1->max({0}, false);
    new_t->print();
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}

TEST(TensorTestSuite, tensor_math_reduction_argmax) {
    // Test #1
    Tensor *t1_ref = new Tensor({1.0f, 3.0f, 2.0f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({
                                    1.0f, 4.0f, 4.0f,
                                    5.0f, 4.0f, 8.0f,
                                    2.0f, -3.0f, 9.0f,
                                    1.0f, 6.0f, 0.0f}, {4, 3}, DEV_CPU);

    Tensor *new_t = t1->argmax({0}, false);
    new_t->print();
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}


TEST(TensorTestSuite, tensor_math_reduction_min) {
    // Test #1
    Tensor *t1_ref = new Tensor({1.0f, -3.0f, 0.0f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({
                                    1.0f, 4.0f, 4.0f,
                                    5.0f, 4.0f, 8.0f,
                                    2.0f, -3.0f, 9.0f,
                                    1.0f, 6.0f, 0.0f}, {4, 3}, DEV_CPU);

    Tensor *new_t = t1->min({0}, false);
    new_t->print();
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}

TEST(TensorTestSuite, tensor_math_reduction_argmin) {
    // Test #1
    Tensor *t1_ref = new Tensor({0.0f, 2.0f, 3.0f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({
                                    1.0f, 4.0f, 4.0f,
                                    5.0f, 4.0f, 8.0f,
                                    2.0f, -3.0f, 9.0f,
                                    1.0f, 6.0f, 0.0f}, {4, 3}, DEV_CPU);

    Tensor *new_t = t1->argmin({0}, false);
    new_t->print();
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}

TEST(TensorTestSuite, tensor_math_reduction_sum) {
    // Test #1
    Tensor *t1_ref = new Tensor({4.0f, 4.0f,
                                 4.0f, 4.0f,
                                 4.0f, 4.0f}, {3, 2}, DEV_CPU);
    Tensor *t1 = Tensor::ones({3, 2, 4}, DEV_CPU);

    Tensor *new_t = t1->sum({2}, false);
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}


TEST(TensorTestSuite, tensor_math_reduction_sum_abs) {
    // Test #1
    Tensor *t1_ref = new Tensor({10.0f, 11.0f, 7.0f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({-4.0f, 7.0f, 3.0f,
                             6.0f, 4.0f, -4.0f}, {2, 3}, DEV_CPU);

    Tensor *new_t = t1->sum_abs({0}, false);
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}

TEST(TensorTestSuite, tensor_math_reduction_prod) {
    // Test #1
    Tensor *t1_ref = new Tensor({24.0f, 28.0f, 12.0f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({4.0f, 7.0f, 3.0f,
                             6.0f, 4.0f, 4.0f}, {2, 3}, DEV_CPU);

    Tensor *new_t = t1->prod({0}, false);
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}

TEST(TensorTestSuite, tensor_math_reduction_mean) {
    // Test #1
    Tensor *t1_ref = new Tensor({5.0f, 5.5f, 3.5f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({4.0f, 7.0f, 3.0f,
                             6.0f, 4.0f, 4.0f}, {2, 3}, DEV_CPU);

    Tensor *new_t = t1->mean({0}, false);
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}

TEST(TensorTestSuite, tensor_math_reduction_var) {
    // Test #1
    Tensor *t1_ref = new Tensor({2.0f, 4.5f, 0.5f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({4.0f, 7.0f, 3.0f,
                             6.0f, 4.0f, 4.0f}, {2, 3}, DEV_CPU);

    Tensor *new_t = t1->var({0}, false, true);
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}


TEST(TensorTestSuite, tensor_math_reduction_std) {
    // Test #1
    Tensor *t1_ref = new Tensor({1.4142f, 2.1213f, 0.7071f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({4.0f, 7.0f, 3.0f,
                             6.0f, 4.0f, 4.0f}, {2, 3}, DEV_CPU);

    Tensor *new_t = t1->std({0}, false, true);
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}



TEST(TensorTestSuite, tensor_math_reduction_mode) {
    // Test #1
    Tensor *t1_ref = new Tensor({1.0f, 4.0f, 8.0f},  {3}, DEV_CPU);
    Tensor *t1 = new Tensor({
                             1.0f, 4.0f, 4.0f,
                             5.0f, 4.0f, 8.0f,
                             2.0f, 3.0f, 8.0f,
                             1.0f, 4.0f, 8.0f}, {4, 3}, DEV_CPU);

    Tensor *new_t = t1->mode({0}, false);
    ASSERT_TRUE(Tensor::equivalent(t1_ref, new_t, 10e-4));
}
