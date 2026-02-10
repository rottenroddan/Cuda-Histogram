//
// Created by Steven Roddan on 2/9/2026.
//

#ifndef CUDAHISTOGRAMS_HOSTTESTER_HPP
#define CUDAHISTOGRAMS_HOSTTESTER_HPP

#include <iostream>
#include "Timer.hpp"

template<typename T, typename U>
void validate(const T& truth, const U& test, const size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (truth[i] != test[i]) {
            std::cout << "Error at index " << i << " -> Truth: " << truth[i] << " vs " << test[i] << std::endl;
            abort();
        }
    }
}

template <typename F>
void time_histogram(F f, const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                        const size_t dataSize, const int maxVal, const std::string &testName) {
    TIMING_BEGIN(testName, dataSize);
    f(data.get(), testHistogram.get(), dataSize);
    TIMING_END(testName, dataSize);
    validate(truthHistogram, testHistogram, maxVal);
}

#endif //CUDAHISTOGRAMS_HOSTTESTER_HPP