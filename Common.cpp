//
// Created by Steven Roddan on 2/9/2026.
//

#include <cstring>
#include "Common.hpp"

void clear(int* arr, const size_t size) {
    std::memset(arr, 0, size * sizeof(int));
}
