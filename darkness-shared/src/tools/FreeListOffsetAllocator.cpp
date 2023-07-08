#include "tools/FreeListOffsetAllocator.h"

namespace tools
{
    FreeListOffsetAllocator::FreeListOffsetAllocator()
        : m_currentBlock{ 0 }
    {}

    size_t FreeListOffsetAllocator::allocate()
    {
        if (m_stack.size() > 0)
        {
            auto result = m_stack.top();
            m_stack.pop();
            return result;
        }

        auto result = m_currentBlock;
        ++m_currentBlock;
        return result;
    }

    engine::vector<size_t> FreeListOffsetAllocator::allocate(size_t blocks)
    {
        engine::vector<size_t> result(blocks);
        size_t count = 0;
        while (count < blocks)
            result[count++] = allocate();
        return result;
    }

    void FreeListOffsetAllocator::free(size_t block)
    {
        m_stack.push(block);
    }

    void FreeListOffsetAllocator::free(engine::vector<size_t>& blocks)
    {
        for (auto&& block : blocks)
            free(block);
    }

    // =================================

    FreeListContinuousOffsetAllocator::FreeListContinuousOffsetAllocator(size_t maximumSize)
        : m_maximumSize{ maximumSize }
        , m_currentBlock{ 0 }
    {}

    ContinousHandle FreeListContinuousOffsetAllocator::allocate()
    {
        auto existing = m_map.find(1);
        if (existing != m_map.end() && existing->second.size() > 0)
        {
            auto result = existing->second.top();
            existing->second.pop();
            return { result, 1 };
        }

        auto result = m_currentBlock;
        ++m_currentBlock;
        ASSERT(m_currentBlock <= m_maximumSize, "FreeListContinuousOffsetAllocator ran out of space");
        return { result, 1 };
    }

    ContinousHandle FreeListContinuousOffsetAllocator::allocate(size_t blocks)
    {
        auto existing = m_map.find(blocks);
        if (existing != m_map.end() && existing->second.size() > 0)
        {
            auto result = existing->second.top();
            existing->second.pop();
            return { result, blocks };
        }

        auto result = m_currentBlock;
        m_currentBlock += blocks;
        ASSERT(m_currentBlock <= m_maximumSize, "FreeListContinuousOffsetAllocator ran out of space");
        return { result, blocks };
    }

    void FreeListContinuousOffsetAllocator::free(ContinousHandle block)
    {
        /*auto existing = m_map.find(block.length);
        if (existing != m_map.end())
        {
            existing->second.push(block.offset);
            return;
        }*/

        m_map[block.length].push(block.offset);
    }
}