//
// Created by Steven Roddan on 2/11/2026.
//

#include "ThreadedReducedHistogram.hpp"

void profile_threaded_reduced_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                            const size_t dataSize, const int maxVal, const size_t threadAmount, const std::string &testName) {
    clear(testHistogram.get(), maxVal); // clear last test if any.
    TIMING_BEGIN(testName, static_cast<size_t>(maxVal), dataSize);
    solveThreadedReducedHistogram(data, testHistogram, dataSize, maxVal, threadAmount);
    TIMING_END(testName, static_cast<size_t>(maxVal), dataSize);
    validate(truthHistogram, testHistogram, maxVal);
}