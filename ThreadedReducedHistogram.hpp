//
// Created by Steven Roddan on 2/11/2026.
//

#ifndef CUDAHISTOGRAMS_THREADEDREDUCEDHISTOGRAM_HPP
#define CUDAHISTOGRAMS_THREADEDREDUCEDHISTOGRAM_HPP
#include <barrier>
#include <cassert>
#include <immintrin.h>
#include <malloc.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

#include "Common.hpp"
#include "Timer.hpp"

inline void setThreadCoreAffinity(size_t threadIndex) {
    const DWORD_PTR mask = 1ULL << (threadIndex % 8);
    SetThreadAffinityMask(GetCurrentThread(), mask);
}

template<bool AMD_Thread_Affinity_Test = false, bool Explicit_Prefetch_Test = false, bool Unrolling_Test = false>
void solveThreadedReducedHistogram(const std::shared_ptr<int[]>& data, const std::shared_ptr<int[]>& histogram, const size_t dataSize, const size_t maxVal, const size_t threadAmount) {
    assert(threadAmount != 0 && (threadAmount & (threadAmount - 1)) == 0);
    std::vector<std::thread> threads;

    constexpr size_t cacheLineBytes = std::hardware_constructive_interference_size;
    constexpr size_t elementsPerCacheLine = cacheLineBytes / sizeof(int);
    const size_t cacheLinesPerThread = (maxVal + elementsPerCacheLine - 1) / elementsPerCacheLine;

    const size_t reducedSize = cacheLinesPerThread * threadAmount * elementsPerCacheLine * sizeof(int);
    std::shared_ptr<int[]> reducedHistogram(static_cast<int*>(_aligned_malloc(reducedSize, cacheLineBytes)),[](int* p) { _aligned_free(p);});
    memset(reducedHistogram.get(), 0, reducedSize);

    const size_t elementsPerThread = (dataSize + threadAmount - 1) / threadAmount;

    std::barrier barrier(threadAmount);
    for (size_t t = 0; t < threadAmount; ++t) {
        threads.emplace_back([=, &data, &reducedHistogram, &barrier]() {
            if constexpr (AMD_Thread_Affinity_Test) {
                setThreadCoreAffinity(t);
            }

            const size_t start = t * elementsPerThread;
            const size_t end   = std::min(start + elementsPerThread, dataSize);
            const size_t count = end - start;

            // initial reduction. All threads perform this
            if constexpr (Unrolling_Test) {
                for (size_t i = 0; i < count; i+=4) {
                    for (size_t k = 0; k < 4 && i + k < count; ++k) {
                        const size_t idx = t * cacheLinesPerThread * elementsPerCacheLine + data[start + i + k];
                        if constexpr (Explicit_Prefetch_Test) {
                            if (i + k + std::hardware_destructive_interference_size < count) {
                                const size_t prefetchIdx = t * cacheLinesPerThread * elementsPerCacheLine + data[start + i + std::hardware_destructive_interference_size / sizeof(int) + k];
                                _mm_prefetch(reinterpret_cast<const char*>(&reducedHistogram[prefetchIdx]), _MM_HINT_T0);
                            }
                        }
                        ++reducedHistogram[idx];
                    }
                }
            } else {
                for (size_t i = 0; i < count; ++i) {
                    const size_t idx = t * cacheLinesPerThread * elementsPerCacheLine + data[start + i];
                    if constexpr (Explicit_Prefetch_Test) {
                        if (i + std::hardware_destructive_interference_size < count) {
                                const size_t prefetchIdx = t * cacheLinesPerThread * elementsPerCacheLine + data[start + i + std::hardware_destructive_interference_size / sizeof(int)];
                                _mm_prefetch(reinterpret_cast<const char*>(&reducedHistogram[prefetchIdx]), _MM_HINT_T0);
                            }
                    }
                    ++reducedHistogram[idx];
                }
            }

            size_t reductionThreads = threadAmount / 2;
            barrier.arrive_and_wait();
            while (t < reductionThreads) {
                for (size_t i = 0; i < maxVal; ++i) {
                    const size_t threadOffset = t * cacheLinesPerThread * elementsPerCacheLine;
                    reducedHistogram[threadOffset + i] +=  reducedHistogram[(t + reductionThreads) * cacheLinesPerThread * elementsPerCacheLine + i];
                }

                reductionThreads = reductionThreads / 2;
                barrier.arrive_and_wait();
            }
            barrier.arrive_and_drop();
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (size_t i = 0; i < maxVal; ++i) {
        histogram[i] = reducedHistogram[i];
    }
}

template<bool AMD_Thread_Affinity_Test = false, bool Explicit_Prefetch_Test = false, bool Unrolling_Test = false>
void profile_threaded_reduced_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                                   const size_t dataSize, const int maxVal, const size_t threadAmount, const std::string &testName) {
    clear(testHistogram.get(), maxVal); // clear last test if any.
    TIMING_BEGIN(testName, static_cast<size_t>(maxVal), dataSize, threadAmount);
    solveThreadedReducedHistogram<AMD_Thread_Affinity_Test, Explicit_Prefetch_Test>(data, testHistogram, dataSize, maxVal, threadAmount);
    TIMING_END(testName, static_cast<size_t>(maxVal), dataSize, threadAmount);
    validate(truthHistogram, testHistogram, maxVal);
}


#endif //CUDAHISTOGRAMS_THREADEDREDUCEDHISTOGRAM_HPP