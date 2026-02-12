//
// Created by Steven Roddan on 2/9/2026.
//

#ifndef CUDAHISTOGRAMS_NAIVETHREADEDHISTOGRAM_HPP
#define CUDAHISTOGRAMS_NAIVETHREADEDHISTOGRAM_HPP

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>


#include "Common.hpp"
#include "Timer.hpp"
inline std::mutex mtx;

template<typename T, typename U>
void solveThreadedNaiveHistogram(const T &data, const U &histogram, const size_t dataSize) {
    std::vector<std::thread> threads;
    size_t sizePerThread = (dataSize + CPU_THREADS - 1) / CPU_THREADS;

    for (size_t t = 0; t < CPU_THREADS; ++t) {
        threads.emplace_back([sizePerThread, &data, &histogram, t, dataSize]() {
            for (size_t i = 0; i < sizePerThread && i + t * sizePerThread < dataSize; ++i) {
                std::lock_guard lock(mtx);
                ++histogram[data[i + t * sizePerThread]];
            }
        });
    }

    for (auto &thread: threads) {
        thread.join();
    }
}

void profile_threaded_naive_cpu_histogram(const std::shared_ptr<int[]> &data,
                                          const std::shared_ptr<int[]> &truthHistogram,
                                          const std::shared_ptr<int[]> &testHistogram,
                                          size_t dataSize, int maxVal, const std::string &testName);

#endif //CUDAHISTOGRAMS_NAIVETHREADEDHISTOGRAM_HPP