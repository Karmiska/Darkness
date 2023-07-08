#pragma once

#if 0
#include <cstdlib>
#include <new>
#include <atomic>
#include "tools/ToolsCommon.h"

using namespace std;

// no inline, required by [replacement.functions]/3
void* operator new(std::size_t sz);
//void* operator new  (std::size_t count, std::align_val_t al);

void operator delete(void* ptr) noexcept;
//void operator delete  (void* ptr, std::align_val_t al) noexcept;

namespace memory_allocation
{
    // block is an allocation
    template<size_t N>
    class Block
    {
    public:
        Block()
        {
            m_memory = std::malloc(N);
        }
        ~Block()
        {
            free(m_memory);
        }

        void* allocate(size_t bytes)
        {
            
        }
    private:
        void* m_memory;
        uint64_t m_usageMask[roundUpToMultiple(N, 64) / 64];
    };

    // bucket is a list of blocks


    class MemoryManager
    {
    public:
        MemoryManager();
        ~MemoryManager();

        //static MemoryManager& instance()
        //{
        //    static MemoryManager manager;
        //    return manager;
        //}

        void* allocate(std::size_t sz);
        void* allocate(std::size_t count, std::align_val_t al);

        void free(void* ptr) noexcept;
        void free(void* ptr, std::align_val_t al) noexcept;

    private:
        void* m_massivePool;
        std::atomic<size_t> m_usedSoFar;
    };
}
#endif