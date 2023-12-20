#pragma once

#include <cstdint>
#include <set>
#include <limits>

namespace ecs
{
    using EntityAddress = uint64_t;
    using EntityId = uint64_t;

    using ComponentTypeId = uint64_t;
    using ComponentArcheTypeId = uint64_t;
    const ComponentTypeId InvalidTypeId = std::numeric_limits<uint64_t>::max();
    const ComponentArcheTypeId InvalidArcheTypeId = 0xffff;
    static ComponentTypeId GlobalComponentTypeId = 0;

    constexpr uint64_t EntityAddressArcheTypeMask = 0xffff000000000000;
    constexpr uint64_t EntityAddressChunkMask = 0x0000ffffffff0000;
    constexpr uint64_t EntityAddressEntityMask = 0x000000000000ffff;

    constexpr size_t PreferredChunkSizeBytes = 64 * 1024;
    static const size_t ChunkAlignment = 64;
    static const size_t ChunkDataAlignment = 16;

    // when allocating new chunks, zeroing the memory is a slow operation.
    // one can avoid that by allocating large enough chunk storage allocation,
    // because when ChunkStorage is created, it creates a bunch of allocations in it's constructor
    // which is called by Ecs constructor. (so all memory can be allocated beforehand if need be)
    constexpr bool ZeroChunkMemory = false;
    constexpr size_t ChunkStorageAllocationSize = 16ull * 1024ull * 1024ull;
    constexpr size_t PreallocatedChunkStorageSizeBytes = 0;// 5ull * 1024ull * 1024ull * 1024ull;

    static_assert((ChunkStorageAllocationSize % PreferredChunkSizeBytes) == 0);

    #define MaximumEcsTypes 1024
    #define MaximumEcsArcheTypes 1024
}