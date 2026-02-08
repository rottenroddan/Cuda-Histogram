#include "Timer.hpp"

#include <cuda_runtime.h>
#include <functional>
#include <iostream>
#include <memory>
#include <random>

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
    std::uniform_int_distribution dis(0, maxVal);

    const auto arr = new int[size];
    for (size_t i = 0; i < size; ++i) {
        arr[i] = dis(gen);
    }
    return arr;
}

template<typename T, typename U>
void solveNaiveHistogram(const T& data, const size_t size, const U& histogram) {
    for (size_t i = 0; i < size; ++i) {
        ++histogram[data[i]];
    }
}

template <typename F>
void test_datasets(F f, const size_t dataSize, const int maxVal, std::string testName) {
    const auto randomData = std::shared_ptr<int[]>(generateRandomIntArray(dataSize, maxVal));
    const auto historgram = std::shared_ptr<int[]>(new int[maxVal+1]);

    TIMING_BEGIN(testName);
    f(randomData.get(), dataSize, historgram.get());
    TIMING_END(testName);
    validate(historgram, historgram, dataSize);

}

int main() {
    test_datasets([](int* data, size_t size, int* histogram) {
            solveNaiveHistogram(data, size, histogram);
        }, 100000, 256, "Naive_Single_Thread_100_000");





    std::cout << "Naive: " << timing::totals().at("Naive_Single_Thread_100_000") << std::endl;

    return 0;
}