#pragma once

#include "engine/graphics/Device.h"
#include "engine/graphics/Resources.h"

/*
Design targets
- hole free

*/

#pragma pack(push, 4)
struct DeltaContainer
{
    uint32_t from;
    uint32_t to;
};
#pragma pack(pop)

namespace engine
{
    class SyncAllocator
    {
    public:
        SyncAllocator(
            Device& device,
            const BufferDescription& desc);

        uintptr_t allocate(size_t blocks);
        void free(uintptr_t block, size_t count);

        void setDirty(uintptr_t block, size_t count);

        void synchronize();

    private:
        Device& m_device;
        
        // CPU buffer
        BufferSRVOwner m_cpuBuffer;

        // GPU buffer and UAV
        BufferSRVOwner m_gpuBuffer;
        BufferUAVOwner m_gpuBufferUAV;

        // buffer for change deltas
        BufferSRVOwner m_cpuDeltaSRV;

        // map setup
        uint32_t m_blockSizeBytes;
        void* m_cpuBufferPtr;
        DeltaContainer* m_deltaBufferPtr;


        // logical addressing
        uintptr_t m_left;
        uintptr_t m_right;
        uintptr_t m_max;



        inline uint32_t blockBytes(size_t count);

        void handleDirty();
        void handleMove();
    };
}
