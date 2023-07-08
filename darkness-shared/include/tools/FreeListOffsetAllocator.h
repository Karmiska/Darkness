#pragma once

#include "tools/Debug.h"
//#include "containers/vector.h"
#include "containers/unordered_map.h"
#include <stack>
#include "tools/ByteRange.h"

namespace tools
{
    class FreeListOffsetAllocator
    {
    public:
        FreeListOffsetAllocator();
        size_t allocate();
        engine::vector<size_t> allocate(size_t blocks);
        void free(size_t block);
        void free(engine::vector<size_t>& blocks);

    private:
        size_t m_currentBlock;
        std::stack<size_t> m_stack;
    };

    constexpr size_t InvalidContinousHandle = std::numeric_limits<size_t>::max();
    struct ContinousHandle
    {
        size_t offset = InvalidContinousHandle;
        size_t length = 0;
    };

    class FreeListContinuousOffsetAllocator
    {
    public:
        FreeListContinuousOffsetAllocator(size_t maximumSize);

        ContinousHandle allocate();
        ContinousHandle allocate(size_t blocks);

        void free(ContinousHandle block);

    private:
        size_t m_maximumSize;
        size_t m_currentBlock;
        engine::unordered_map<size_t, std::stack<size_t>> m_map;
    };
}