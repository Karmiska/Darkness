#pragma once

#include <cstdint>
#include "containers/vector.h"

namespace tools
{
    class ByteRange
    {
    public:
        ByteRange()
            : start{ 0 }
            , stop{ 0 }
            , elementSize{ 1 }
        {}

        ByteRange(uintptr_t& _start, uintptr_t& _stop)
            : start{ _start }
            , stop{ _stop }
            , elementSize{ sizeof(uintptr_t) }
        {}

        template<typename T>
        ByteRange(engine::vector<T>& range)
            : start{ reinterpret_cast<const uintptr_t>(range.data()) }
            , stop{ start + (range.size() * sizeof(T)) }
            , elementSize{ sizeof(T) }
        {}

        template<typename T>
        ByteRange(const engine::vector<T>& range)
            : start{ reinterpret_cast<const uintptr_t>(range.data()) }
            , stop{ start + (range.size() * sizeof(T)) }
            , elementSize{ sizeof(T) }
        {}

        template<typename T>
        ByteRange(T* _start, T* _stop)
            : start{ reinterpret_cast<const uintptr_t>(_start) }
            , stop{ reinterpret_cast<const uintptr_t>(_stop) }
            , elementSize{ sizeof(T) }
        {}

        template<typename T>
        ByteRange(T& _start, T& _stop)
            : start{ reinterpret_cast<const uintptr_t>(_start) }
            , stop{ reinterpret_cast<const uintptr_t>(_stop) }
            , elementSize{ sizeof(T) }
        {}

        template<typename T>
        ByteRange(const T* _start, const T* _stop)
            : start{ reinterpret_cast<const uintptr_t>(_start) }
            , stop{ reinterpret_cast<const uintptr_t>(_stop) }
            , elementSize{ sizeof(T) }
        {}

        template<typename T>
        ByteRange(const T& _start, const T& _stop)
            : start{ static_cast<const uintptr_t>(_start) }
            , stop{ static_cast<const uintptr_t>(_stop) }
            , elementSize{ sizeof(T) }
        {}

        ByteRange(const ByteRange& range)
            : start{ range.start }
            , stop{ range.stop }
            , elementSize{ range.elementSize }
        {}

        ByteRange& operator=(const ByteRange& range)
        {
            start = range.start;
            stop = range.stop;
            elementSize = range.elementSize;
            return *this;
        }

        size_t sizeBytes() const
        {
            return stop - start;
        }

        size_t size() const
        {
            return length();
        }

        size_t length() const
        {
            return (stop - start) / elementSize;
        }

        uint8_t& operator[](const size_t index)
        {
            return *(reinterpret_cast<uint8_t*>(start) + index);
        }

        const uint8_t& operator[](const size_t index) const
        {
            return *(reinterpret_cast<uint8_t*>(start) + index);
        }

        uintptr_t start;
        uintptr_t stop;
        size_t elementSize;
    };
}
