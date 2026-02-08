cmake_minimum_required(VERSION 4.1 FATAL_ERROR)
project(CudaHistograms CXX CUDA)

set(CMAKE_CUDA_STANDARD 20)

add_executable(CudaHistograms main.cu)

set_target_properties(CudaHistograms PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON)
