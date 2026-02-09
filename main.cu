#include "TableStats.hpp"

#include <cuda_runtime.h>
#include <functional>
#include <iostream>
#include <locale>
#include <memory>
#include <random>
#include <sstream>

#define CPU_THREADS 8

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

void clear(int* arr, const size_t size) {
    memset(arr, 0, size * sizeof(int));
}

template<typename T, typename U>
void validate(const T& truth, const U& test, const size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (truth[i] != test[i]) {
            std::cout << "Error at index " << i << " -> Truth: " << truth[i] << " vs " << test[i] << std::endl;
            abort();
        }
    }
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

template<typename T, typename U>
void solveNaiveHistogram(const T& data, const U& histogram, const size_t size) {
    for (size_t i = 0; i < size; ++i) {
        ++histogram[data[i]];
    }
}

std::mutex mtx;

template<typename T, typename U>
void solveThreadedNaiveHistogram(const T& data, const U& histogram, const size_t dataSize) {
    std::vector<std::thread> threads;
    size_t sizePerThread = (dataSize + CPU_THREADS - 1) / CPU_THREADS;

    for (size_t t = 0; t < CPU_THREADS; ++t) {
        threads.emplace_back([sizePerThread, &data, &histogram, t, dataSize]() {
            for (size_t i = 0; i < sizePerThread && i + t*sizePerThread < dataSize; ++i) {
                std::lock_guard lock(mtx);
                ++histogram[data[i + t*sizePerThread]];
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

template <typename F>
void time_histogram(F f, const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                        const size_t dataSize, const int maxVal, const std::string &testName) {
    TIMING_BEGIN(testName, dataSize);
    f(data.get(), testHistogram.get(), dataSize);
    TIMING_END(testName, dataSize);
    validate(truthHistogram, testHistogram, maxVal);
}

void profile_naive_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                    const size_t dataSize, const int maxVal, const std::string &testName) {
    clear(testHistogram.get(), maxVal); // clear last test if any.
    time_histogram(
        [&](const auto& d, const auto& hist, const size_t size) {
            solveNaiveHistogram(d, hist, size);
        },
        data,
        truthHistogram,
        testHistogram,
        dataSize,
        maxVal,
        testName
    );
}

void profile_threaded_naive_cpu_histogram(const std::shared_ptr<int[]> &data, const std::shared_ptr<int[]> &truthHistogram, const std::shared_ptr<int[]> &testHistogram,
                                    const size_t dataSize, const int maxVal, const std::string &testName) {
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
            //break;
        }
        //break;
    }

    TimingTablePrinter::print();
    std::cout << "Total Iterations each: " << iterations << std::endl;
    return 0;
}