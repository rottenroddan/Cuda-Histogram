//
// Created by Steven Roddan on 2/9/2026.
//

#ifndef CUDAHISTOGRAMS_NAIVEHISTOGRAMSOLVER_HPP
#define CUDAHISTOGRAMS_NAIVEHISTOGRAMSOLVER_HPP

#include <memory>
#include <string>
#include "Common.hpp"
#include "Timer.hpp"

template<typename T, typename U>
void solveNaiveHistogram(const T& data, const U& histogram, const size_t size) {
    for (size_t i = 0; i < size; ++i) {
        ++histogram[data[i]];
    }
}


void profile_naive_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                    size_t dataSize, int maxVal, const std::string &testName);

#endif //CUDAHISTOGRAMS_NAIVEHISTOGRAMSOLVER_HPP