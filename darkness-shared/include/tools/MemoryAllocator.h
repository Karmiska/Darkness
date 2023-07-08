#pragma once

#include "tools/Allocator.h"
#include "tools/ByteRange.h"
#include "tools/ToolsCommon.h"
#include "containers/vector.h"
#include <functional>
#include <map>
#include "containers/unordered_map.h"

namespace tools
{
    size_t gpuAllocationStrategy(size_t elementSize, size_t currentElements, size_t newElements);

    constexpr size_t BiggestAlignment = pow<20>(2);
	constexpr uintptr_t MemoryAllocatorInvalidPtr = 0xffffffffffffffff;
    class MemoryAllocator : public OffsetAllocatorInterface
    {
    public:
        MemoryAllocator() = default;
        MemoryAllocator(ByteRange range, size_t biggestAlignment = BiggestAlignment);

        void* allocate(size_t bytes) override;
        void* allocate(size_t bytes, size_t align) override;
        void free(void* ptr) override;
        size_t offset(void* ptr) const override;
        void* ptrFromOffset(size_t offset) const override;

		void resize(ByteRange range);
    public:
        // for testing purposes
        engine::vector<engine::vector<ByteRange>>& reservedAllocations() { return m_reservedAllocations; }
        engine::vector<engine::unordered_map<uintptr_t, ByteRange>>& freeAllocations() { return m_freeAllocations; }

    private:
        ByteRange m_originalRange;
        size_t m_biggestAlignment;

        ByteRange getClosestRange(size_t bytes, size_t align);
        void insertBack(ByteRange&& range, size_t slot);

        size_t bestOptionalAlignment(size_t size);

        engine::vector<engine::vector<ByteRange>> m_reservedAllocations;
        engine::vector<engine::unordered_map<uintptr_t, ByteRange>> m_freeAllocations;

        std::map<const void*, short> m_reserveInfo;
    };
}
