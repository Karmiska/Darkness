#pragma once

#include <type_traits>
#include <cstdint>

template<size_t N, class T>
inline constexpr T pow(const T x)
{
    T res = x;
    for (size_t i = 0; i < N-1; ++i)
        res *= x;
    return res;
}

size_t alignToIndex(size_t align);
size_t indexToAlign(size_t index);
bool isPowerOfTwo(size_t align);

template<typename T>
const T roundUpToMultiple(const T& value, size_t align)
{
    return static_cast<const T>(((static_cast<const size_t>(value) + static_cast<const int>(value >= 0) * (align - 1)) / align) * align);
}

template<typename T>
const T* roundUpToMultiple(const T* value, size_t align)
{
    return reinterpret_cast<const T*>(((reinterpret_cast<const size_t>(value) + static_cast<const int>(value >= 0) * (align - 1)) / align) * align);
}

template<typename T>
const T roundDownToMultiple(const T& value, size_t align)
{
    return static_cast<T>(static_cast<T>(value / align) * align);
}

template<typename T>
const T roundUpToPow2(const T& value)
{
    T val = value;
    if (!(val & (val - 1))) {
        return (val);
    }

    while (val & (val - 1)) {
        val = val & (val - 1);
    }

    val = val << 1;
    return val;
}
