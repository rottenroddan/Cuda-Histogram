//
// Created by Steven Roddan on 2/11/2026.
//

#ifndef CUDAHISTOGRAMS_THREADEDREDUCEDHISTOGRAM_HPP
#define CUDAHISTOGRAMS_THREADEDREDUCEDHISTOGRAM_HPP
#include <barrier>
#include <malloc.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "Common.hpp"
#include "Timer.hpp"

inline void solveThreadedReducedHistogram(const std::shared_ptr<int[]>& data, const std::shared_ptr<int[]>& histogram, const size_t dataSize, const size_t maxVal, const size_t threadAmount) {
    std::vector<std::thread> threads;

    constexpr size_t cacheLineBytes = std::hardware_constructive_interference_size;
    constexpr size_t elementsPerCacheLine = cacheLineBytes / sizeof(int);
    const size_t cacheLinesPerThread = (maxVal + elementsPerCacheLine - 1) / elementsPerCacheLine;

    const size_t reducedSize = cacheLinesPerThread * threadAmount * elementsPerCacheLine * sizeof(int);
    std::shared_ptr<int[]> reducedHistogram(static_cast<int*>(_aligned_malloc(reducedSize, cacheLineBytes)),[](int* p) { _aligned_free(p);});
    memset(reducedHistogram.get(), 0, reducedSize);

    const size_t elementsPerThread = (dataSize + threadAmount - 1) / threadAmount;

    //std::barrier barrier(threadAmount);
    for (size_t t = 0; t < threadAmount; ++t) {
        threads.emplace_back([&data, &reducedHistogram, elementsPerThread, t, cacheLinesPerThread, elementsPerCacheLine] {
            for (uint64_t i = 0; i < elementsPerThread; ++i) {
                ++reducedHistogram[t * cacheLinesPerThread * elementsPerCacheLine + data[t * elementsPerThread + i]];
            }
    //         // barrier.arrive_and_wait();
    //         //
    //         // barrier.arrive_and_drop();
        });
    }



    for (auto& thread : threads) {
        thread.join();
    }



    for (size_t t = 0; t < threadAmount; ++t) {
        for (size_t i = 0; i < maxVal; ++i) {
            histogram[i] += reducedHistogram[t * cacheLinesPerThread * elementsPerCacheLine + i];
            //std::cout << "i: " << i << "idx: " << t * cacheLinesPerThread * elementsPerCacheLine + i << "\n";
        }
    }
    // std::cout << "Waht" << std::endl;
}

void profile_threaded_reduced_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                    size_t dataSize, int maxVal, size_t threadAmount, const std::string &testName);


#endif //CUDAHISTOGRAMS_THREADEDREDUCEDHISTOGRAM_HPP