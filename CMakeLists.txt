cmake_minimum_required(VERSION 4.1 FATAL_ERROR)
project(CudaHistograms CXX CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_STANDARD 20)

add_library(CpuHistograms Common.cpp
        NaiveThreadedHistogram.cpp NaiveHistogramSolver.cpp)

add_executable(CudaHistograms  main.cu)

target_link_libraries(CudaHistograms
        PRIVATE CpuHistograms
)

set_target_properties(CudaHistograms PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON)
