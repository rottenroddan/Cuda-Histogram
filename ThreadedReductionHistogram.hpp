//
// Created by Steven Roddan on 2/9/2026.
//

#ifndef CUDAHISTOGRAMS_THREADEDREDUCTIONHISTOGRAM_HPP
#define CUDAHISTOGRAMS_THREADEDREDUCTIONHISTOGRAM_HPP

template<typename T, typename U>
void solveThreadedReductionHistogram(const T& data, const U& histogram, const size_t dataSize) {
    std::vector<std::thread> threads;
    size_t sizePerThread = (dataSize + CPU_THREADS - 1) / CPU_THREADS;

    for (auto& thread : threads) {
        thread.join();
    }
}

void profile_threaded_naive_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                    size_t dataSize, int maxVal, const std::string &testName);


#endif //CUDAHISTOGRAMS_THREADEDREDUCTIONHISTOGRAM_HPP