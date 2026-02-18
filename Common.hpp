//
// Created by Steven Roddan on 2/9/2026.
//

#ifndef CUDAHISTOGRAMS_UTIL_HPP
#define CUDAHISTOGRAMS_UTIL_HPP

#include <iostream>

#define CPU_THREADS 16

template<typename T>
void pre_touch(T* ptr, size_t count) {
    volatile T sink = 0;
    for (size_t i = 0; i < count; ++i)
        sink += ptr[i];
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

void clear(int* arr, size_t size);

#endif //CUDAHISTOGRAMS_UTIL_HPP