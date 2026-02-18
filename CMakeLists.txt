cmake_minimum_required(VERSION 4.1 FATAL_ERROR)
project(CudaHistograms CXX CUDA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CUDA_STANDARD 20)

add_library(CpuHistograms Common.cpp
        NaiveThreadedHistogram.cpp NaiveHistogramSolver.cpp
        ThreadedChunkedHistogram.cpp
)

if(MSVC)
    target_compile_options(CpuHistograms PRIVATE
            $<$<CONFIG:Release>:/O2>
            $<$<CONFIG:Release>:/DNDEBUG>
            $<$<CONFIG:Debug>:/Od>
            $<$<CONFIG:Debug>:/Zi>
            /W4
            /permissive-
            /Zc:__cplusplus
            /WX
            /arch:AVX512
    )
endif()

add_executable(CudaHistograms  main.cu)

target_link_libraries(CudaHistograms
        PRIVATE CpuHistograms
)

execute_process(COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

add_compile_definitions(GIT_HASH="${GIT_HASH}")

set_target_properties(CudaHistograms PROPERTIES
        CUDA_SEPARABLE_COMPILATION ON)
