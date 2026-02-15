
#include "NaiveHistogramSolver.hpp"
#include "NaiveThreadedHistogram.hpp"
#include "ThreadedChunkedHistogram.hpp"
#include "ThreadedReducedHistogram.hpp"
#include "TableStats.hpp"

#include <cuda_runtime.h>
#include <functional>
#include <iostream>
#include <locale>
#include <memory>
#include <random>
#include <sstream>



constexpr size_t iterations = 5;
constexpr size_t smallDataSize = 100000;
constexpr size_t mediumDataSize = smallDataSize * 100;
constexpr size_t largeDataSize = mediumDataSize * 100;
//constexpr size_t hugeDataSize = largeDataSize * 10;


std::vector V_TEST_SIZES = {smallDataSize, mediumDataSize, largeDataSize};
std::vector V_BIN_SIZES = {16, 32, 128, 256, 512, 1024};
//std::vector V_BIN_SIZES = {128, 256};
std::vector V_THREAD_COUNTS = {1, 2, 4, 8, 16, 32, 64};
//std::vector V_THREAD_COUNTS = {8, 16, 32};

std::string comma_separate(const size_t value) {
    std::ostringstream oss;
    oss.imbue(std::locale(""));
    oss << value;
    return oss.str();
}

int* generateRandomIntArray(const size_t size, const int maxVal) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution dis(0, maxVal-1);

    const auto arr = new int[size];
    for (size_t i = 0; i < size; ++i) {
        arr[i] = dis(gen);
    }
    return arr;
}

int main() {
    const auto start = std::chrono::system_clock::now();
    const std::time_t start_time = std::chrono::system_clock::to_time_t(start);
    std::cout << "Testing started at " << std::ctime(&start_time);
    std::cout << "Iterations: " << iterations << std::endl;

    for (const auto& testSize : V_TEST_SIZES) {
        for (const auto& binSize : V_BIN_SIZES) {
            std::cout << "Testing [" << "BinSize: " << binSize << ", " << "DataSize: " << testSize << "] -> status..." << std::flush;
            const std::shared_ptr<int[]> data(generateRandomIntArray(testSize, binSize), std::default_delete<int[]>());
            const std::shared_ptr<int[]> truthHistogram(new int[binSize]);
            const std::shared_ptr<int[]> testHistogram(new int[binSize]);

            // hope my naive implementation works ;)
            clear(truthHistogram.get(), binSize);
            solveNaiveHistogram(data, truthHistogram, testSize);

            for (int i = 0; i < iterations; ++i) {
                if (i > 0) {
                    std::cout << "," << std::flush;
                }

                profile_naive_cpu_histogram(data, truthHistogram, testHistogram, testSize, binSize, "Baseline");

                // this test relies on lock_guards for threads accessing the histogram, it is purely dumb
                // and takes a long time to run.
                if (testSize == *V_TEST_SIZES.begin()) {
                    profile_threaded_naive_cpu_histogram(data, truthHistogram, testHistogram, testSize, binSize, "Naive Threaded CPU-Histogram");
                }

                for (const auto& threadCount : V_THREAD_COUNTS ) {
                    // profile_threaded_chunked_cpu_histogram(data, truthHistogram, testHistogram,
                    //                                         testSize, binSize, threadCount, "Threaded Chunked CPU-Histogram (False Sharing possible)");
                    profile_threaded_reduced_cpu_histogram(data, truthHistogram, testHistogram,
                                                            testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram");

                    profile_threaded_reduced_cpu_histogram<true, false, false>(data, truthHistogram, testHistogram,
                                                            testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram With CORE Affinity (8)");
                    profile_threaded_reduced_cpu_histogram<false, true, true>(data, truthHistogram, testHistogram,
                                                            testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram With Explicit PreFetch & Unrolling");
                    profile_threaded_reduced_cpu_histogram<false, false, true>(data, truthHistogram, testHistogram,
                                                            testSize, binSize, threadCount, "Threaded Reduction CPU-Histogram With Unrolling");
                }
                std::cout << i + 1;
            }
            std::cout << "finished." << std::endl;
        }
    }
    const auto end = std::chrono::system_clock::now();

    TimingTablePrinter::print();
    std::cout << "Total Iterations each: " << iterations << std::endl;
    const std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Testing finished at " << std::ctime(&end_time);
    return 0;
}