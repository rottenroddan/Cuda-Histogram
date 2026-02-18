
#include "NaiveHistogramSolver.hpp"
#include "NaiveThreadedHistogram.hpp"
#include "ThreadedChunkedHistogram.hpp"
#include "ThreadPool.hpp"
#include "ThreadedReducedHistogram.hpp"
#include "TableStats.hpp"

#include <cuda_runtime.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
#include <memory>
#include <random>
#include <sstream>

constexpr size_t iterations = 10;
constexpr size_t smallDataSize = 100000;
constexpr size_t mediumDataSize = smallDataSize * 100;
constexpr size_t largeDataSize = mediumDataSize * 100;
// constexpr size_t hugeDataSize = largeDataSize * 10;

std::vector V_TEST_SIZES = {largeDataSize};
std::vector V_BIN_SIZES = {512, 1024, 2048, 4096};
//std::vector V_BIN_SIZES = {128, 256};
std::vector V_THREAD_COUNTS = {16, 32, 64};
//std::vector V_THREAD_COUNTS = {8, 16, 32};

std::string comma_separate(const size_t value) {
    std::ostringstream oss;
    oss.imbue(std::locale(""));
    oss << value;
    return oss.str();
}

template<typename T>
void generateRandomIntArray(const T& ptr,  const size_t size, const int maxVal) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dis(0, maxVal-1);

    for (size_t i = 0; i < size; ++i) {
        ptr[i] = dis(gen);
    }
}

int x = 0;
std::mutex test_mutex;

void foo() {
    {
        std::lock_guard lock(test_mutex);
        ++x;
    }
}

int main() {
    const auto start = std::chrono::system_clock::now();
    const std::time_t start_time = std::chrono::system_clock::to_time_t(start);
    std::cout << "Testing started at " << std::ctime(&start_time);
    std::cout << "Iterations: " << iterations << std::endl;


    for (const auto& testSize : V_TEST_SIZES) {
        const std::shared_ptr<int[]> data(new int[testSize], std::default_delete<int[]>());
        for (const auto& binSize : V_BIN_SIZES) {
            std::cout << "Testing [" << "BinSize: " << binSize << ", " << "DataSize: " << testSize << "] -> status..." << std::flush;
            generateRandomIntArray(data, testSize, binSize);
            const std::shared_ptr<int[]> truthHistogram(new int[binSize]);
            const std::shared_ptr<int[]> testHistogram(new int[binSize]);

            // hope my naive implementation works ;)
            clear(truthHistogram.get(), binSize);

            solveNaiveHistogram(data, truthHistogram, testSize);

            profile_naive_cpu_histogram(data, truthHistogram, testHistogram, testSize, binSize, "Baseline", iterations);

            // this test relies on lock_guards for threads accessing the histogram, it is purely dumb
            // and takes a long time to run.
            // if (testSize == *V_TEST_SIZES.begin()) {
            //     profile_threaded_naive_cpu_histogram(data, truthHistogram, testHistogram, testSize, binSize, "Naive Threaded CPU-Histogram", iterations);
            // }

            for (const auto& threadCount : V_THREAD_COUNTS ) {
                ThreadPool pool(threadCount);

                profile_threaded_reduced_cpu_histogram(data, truthHistogram, testHistogram,
                                                        testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram", iterations, pool);

                // AMD ICD 0 test
                profile_threaded_reduced_cpu_histogram<true, false, false>(data, truthHistogram, testHistogram,
                                                        testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram With CORE Affinity (8)", iterations, pool);
                // Prefetching and Loop Unrolling
                profile_threaded_reduced_cpu_histogram<false, true, false>(data, truthHistogram, testHistogram,
                                                        testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram With Explicit PreFetch", iterations, pool);
                // Prefetching and Loop Unrolling
                profile_threaded_reduced_cpu_histogram<false, true, true>(data, truthHistogram, testHistogram,
                                                        testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram With Explicit PreFetch & Unrolling", iterations, pool);
                // Unrolling
                profile_threaded_reduced_cpu_histogram<false, false, true>(data, truthHistogram, testHistogram,
                                                        testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram With Unrolling", iterations, pool);
                // Pin threads so caching isn't lost.
                profile_threaded_reduced_cpu_histogram<false, false, false, true>(data, truthHistogram, testHistogram,
                                                        testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram With Pinned threads", iterations, pool);

            }
            std::cout << "finished." << std::endl;
        }
    }
    const auto end = std::chrono::system_clock::now();


    std::ofstream outFile("thread_pool_results_on_host_reduction_algorithm.txt");
    if (!outFile) {
        std::cerr << "Could not open file for writing!" << std::endl;
        return 1;
    }

    outFile << GIT_HASH << "\n";
    outFile << "Iterations: " << iterations << "\n";
    TimingTablePrinter::print(outFile);
    std::cout << "Total Iterations each: " << iterations << std::endl;
    const std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Testing finished at " << std::ctime(&end_time);
    return 0;
}