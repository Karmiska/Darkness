#include "ecs/ArcheTypeStorage.h"

namespace ecs
{
    bool ChunkEntityCount(TypeStorage& componentTypeStorage, const ArcheTypeStorage::ArcheTypeContainer& archeTypeInfo, size_t elements, size_t maxSize)
    {
        size_t bytesUsed = 0;
        for (auto&& type : archeTypeInfo.set)
        {
            auto typeInfo = componentTypeStorage.typeInfo(type);
            auto alignment = std::max(typeInfo.alignment, ChunkDataAlignment);
            bytesUsed = roundUpToMultiple(bytesUsed, alignment);
            bytesUsed += typeInfo.typeSizeBytes * elements;
        }
        bytesUsed += sizeof(EntityId) * elements;
        return bytesUsed <= maxSize;
    }

    size_t ChunkEntityCount(TypeStorage& componentTypeStorage, ArcheTypeStorage& archeTypeStorage, ComponentArcheTypeId archeType, size_t maxSize)
    {
        auto archeTypeInfo = archeTypeStorage.archeTypeInfo(archeType);
        size_t maxAlignmentBytes = 0;
        for (auto&& type : archeTypeInfo.set)
        {
            auto typeInfo = componentTypeStorage.typeInfo(type);
            auto alignment = std::max(typeInfo.alignment, ChunkDataAlignment) - 1; // no padding needed if full size alignment. hence -1
            maxAlignmentBytes += alignment;
        }

        auto estimatedEntityCount = (maxSize - maxAlignmentBytes) / (archeTypeInfo.sizeBytes + sizeof(EntityId));
        while (ChunkEntityCount(componentTypeStorage, archeTypeInfo, estimatedEntityCount, maxSize))
            ++estimatedEntityCount;

        return estimatedEntityCount - 1;
    }
}
