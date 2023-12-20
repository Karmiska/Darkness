#include "ecs/ArcheTypeStorage.h"

namespace ecs
{
    bool ChunkElementCount(TypeStorage& componentTypeStorage, ArcheTypeStorage& archeTypeStorage, ComponentArcheTypeId archeType, size_t elements, size_t maxSize)
    {
        auto archeTypeInfo = archeTypeStorage.archeTypeInfo(archeType);

        size_t bytesUsed = 0;
        for (auto&& type : archeTypeInfo.set)
        {
            auto typeInfo = componentTypeStorage.typeInfo(type);
            auto minAlignment = std::max(typeInfo.alignment, ChunkDataAlignment);

            bytesUsed = roundUpToMultiple(bytesUsed, minAlignment);
            bytesUsed += typeInfo.typeSizeBytes * elements;
            bytesUsed = roundUpToMultiple(bytesUsed, minAlignment);
        }
        bytesUsed += sizeof(EntityId) * elements;
        return bytesUsed <= maxSize;
    }

    size_t ChunkElementCount(TypeStorage& componentTypeStorage, ArcheTypeStorage& archeTypeStorage, ComponentArcheTypeId archeType, size_t maxSize)
    {
        int left = 0;
        int right = maxSize;
        while (left < right)
        {
            auto middle = left + ((right - left) / 2);
            if (left == middle)
                break;

            if (ChunkElementCount(componentTypeStorage, archeTypeStorage, archeType, middle, maxSize))
            {
                left = middle;
            }
            else
            {
                right = middle;
            }
        }

        if (ChunkElementCount(componentTypeStorage, archeTypeStorage, archeType, left + 1, maxSize))
        {
            ASSERT(false, "nope");
        }
        if (!ChunkElementCount(componentTypeStorage, archeTypeStorage, archeType, left, maxSize))
        {
            ASSERT(false, "nope again");
        }

        return left;
    }
}
