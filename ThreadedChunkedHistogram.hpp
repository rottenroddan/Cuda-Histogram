//
// Created by Steven Roddan on 2/9/2026.
//

#ifndef CUDAHISTOGRAMS_THREADEDREDUCTIONHISTOGRAM_HPP
#define CUDAHISTOGRAMS_THREADEDREDUCTIONHISTOGRAM_HPP
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "Common.hpp"
#include "Timer.hpp"

template<typename T, typename U>
void solveThreadedChunkedHistogram(const T& data, const U& histogram, const size_t dataSize, const size_t maxVal, const size_t threadAmount) {
    std::vector<std::thread> threads;
    size_t sizePerThread = /*(dataSize + CPU_THREADS - 1) / CPU_THREADS*/ dataSize;
    size_t threadRange = (maxVal + threadAmount - 1) / threadAmount;

    for (size_t t = 0; t < threadAmount; ++t) {
        threads.emplace_back([&data, &histogram, sizePerThread, threadRange, t] ()
            {
                for (size_t i = 0; i < sizePerThread; i++) {
                    [[likely]]
                    if (data[i] >= threadRange * t && data[i] < threadRange * (t + 1)) {
                        ++histogram[data[i]];
                    }
                }
            }
        );
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

void profile_threaded_chunked_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                    size_t dataSize, int maxVal, size_t threadAmount, const std::string &testName);


#endif //CUDAHISTOGRAMS_THREADEDREDUCTIONHISTOGRAM_HPP