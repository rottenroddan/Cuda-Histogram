//
// Created by Steven Roddan on 2/9/2026.
//

#include "NaiveThreadedHistogram.hpp"

void profile_threaded_naive_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                    size_t dataSize, int maxVal, const std::string &testName) {
    clear(testHistogram.get(), maxVal); // clear last test if any.
    time_histogram(
        [&](const auto& d, const auto& hist, const size_t size) {
            solveThreadedNaiveHistogram(d, hist, size);
        },
        data,
        truthHistogram,
        testHistogram,
        dataSize,
        maxVal,
        testName
    );
}