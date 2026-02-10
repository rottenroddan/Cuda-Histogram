
#include "NaiveHistogramSolver.hpp"
#include "NaiveThreadedHistogram.hpp"
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

std::vector V_TEST_SIZES = {smallDataSize, mediumDataSize, largeDataSize};
std::vector V_BIN_SIZES = {256, 256*16, 256*16*16};

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
    for (const auto& testSize : V_TEST_SIZES) {
        for (const auto& binSize : V_BIN_SIZES) {
            const std::shared_ptr<int[]> data(generateRandomIntArray(testSize, binSize), std::default_delete<int[]>());
            const std::shared_ptr<int[]> truthHistogram(new int[binSize]);
            const std::shared_ptr<int[]> testHistogram(new int[binSize]);

            // hope my naive implementation works ;)
            clear(truthHistogram.get(), binSize);
            solveNaiveHistogram(data, truthHistogram, testSize);

            for (int i = 0; i < iterations; ++i) {
                profile_naive_cpu_histogram(data, truthHistogram, testHistogram, testSize, binSize, "Naive One Thread CPU-Histogram_" + std::to_string(binSize));

                // this test relies on lock_guards for threads accessing the histogram, it is purely dumb
                // and takes a long time to run.
                if (testSize == *V_TEST_SIZES.begin()) {
                    profile_threaded_naive_cpu_histogram(data, truthHistogram, testHistogram, testSize, binSize, "Naive Threaded CPU-Histogram_" + std::to_string(binSize));
                }
            }
        }
    }

    TimingTablePrinter::print();
    std::cout << "Total Iterations each: " << iterations << std::endl;
    return 0;
}