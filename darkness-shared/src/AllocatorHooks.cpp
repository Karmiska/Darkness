
#if 0
#include "AllocatorHooks.h"
#include "tools/Debug.h"
#include "tools/ToolsCommon.h"

static memory_allocation::MemoryManager MemoryManagerInstance;
static std::atomic<size_t> usedSoFar;

namespace memory_allocation
{
    MemoryManager::MemoryManager()
        : m_massivePool{ std::malloc(1024ull * 1024ull * 1024ull * 2ull) }
        , m_usedSoFar{ 0 }
    {
        memset(m_massivePool, 0, 1024ull * 1024ull * 1024ull * 2ull);
    }

    MemoryManager::~MemoryManager()
    {
        std::free(m_massivePool);
    }

    void* MemoryManager::allocate(std::size_t sz)
    {
        size_t current = m_usedSoFar.load();
        size_t newpos = current + sz;
        while (!m_usedSoFar.compare_exchange_weak(
            current,
            newpos,
            std::memory_order_release,
            std::memory_order_relaxed))
        {
            current = m_usedSoFar.load();
            newpos = current + sz;
        };

        void* res = static_cast<uint8_t*>(m_massivePool) + current;
        return res;
        //return allocate(sz, static_cast<std::align_val_t>(sizeof(unsigned long long)));
        //void* res = reinterpret_cast<unsigned char*>(m_massivePool) + m_usedSoFar;
        //m_usedSoFar += sz;
        //if (m_usedSoFar > 1024ull * 1024ull * 1024ull * 10ull)
        //{
        //    int oho = 1;
        //}
        //return res;
        // 
        //std::printf("global op new called, size = %zu\n", sz);

        //usedSoFar += sz;
        //if (sz == 0)
        //    ++sz; // avoid std::malloc(0) which may return nullptr on success
        //
        //if (void* ptr = std::malloc(sz))
        //    return ptr;
        //
        //throw std::bad_alloc{}; // required by [new.delete.single]/3
    }

    void* MemoryManager::allocate(std::size_t count, std::align_val_t al)
    {
        //usedSoFar += count;
        //size_t current = reinterpret_cast<size_t>(reinterpret_cast<unsigned char*>(m_massivePool) + m_usedSoFar);
        //size_t newPos = roundUpToMultiple(current, static_cast<size_t>(al));
        //void* res = reinterpret_cast<void*>(newPos);
        //m_usedSoFar = roundUpToMultiple(m_usedSoFar + (newPos - current) + count, static_cast<size_t>(al));
        //if (m_usedSoFar > 1024ull * 1024ull * 1024ull * 10ull)
        //{
        //    int oho = 1;
        //}
        //return res;

        //std::printf("global op new called, size = %zu\n", count);
        if (count == 0)
            ++count; // avoid std::malloc(0) which may return nullptr on success

        if (void* ptr = _aligned_malloc(count, static_cast<size_t>(al)))
            return ptr;

        throw std::bad_alloc{}; // required by [new.delete.single]/3
    }

    void MemoryManager::free(void* ptr) noexcept
    {
        //std::puts("global op delete called");
        //std::free(ptr);
    }

    void MemoryManager::free(void* ptr, std::align_val_t al) noexcept
    {
        _aligned_free(ptr);
    }
}

// no inline, required by [replacement.functions]/3
void* operator new(std::size_t sz)
{
    return MemoryManagerInstance.allocate(sz);
}

//void* operator new  (std::size_t count, std::align_val_t al)
//{
//    return MemoryManager::instance().allocate(count, al);
//}

void operator delete(void* ptr) noexcept
{
    //MemoryManagerInstance.free(ptr);
}

//void operator delete  (void* ptr, std::align_val_t al) noexcept
//{
//    MemoryManager::instance().free(ptr, al);
//}
#endif