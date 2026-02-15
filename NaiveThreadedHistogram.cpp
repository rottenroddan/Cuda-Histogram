//
// Created by Steven Roddan on 2/9/2026.
//

#include "NaiveThreadedHistogram.hpp"

void profile_threaded_naive_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                    const size_t dataSize, const int maxVal, const std::string &testName) {
    clear(testHistogram.get(), maxVal); // clear last test if any.
    TIMING_BEGIN(testName, static_cast<size_t>(maxVal), dataSize, 1);
    solveThreadedNaiveHistogram(data, testHistogram, dataSize);
    TIMING_END(testName, static_cast<size_t>(maxVal), dataSize, 1);
    validate(truthHistogram, testHistogram, maxVal);
}