#pragma once

#include "engine/graphics/ResourceOwners.h"

namespace engine
{
    class Device;

    class CullerSharedBuffers
    {
    public:
        CullerSharedBuffers(Device& device);

        // Occlusion culling input
        BufferUAVOwner sharedOcclusionCullingInputUAV;
        BufferSRVOwner sharedOcclusionCullingInputSRV;

        // Occlusion culling output
        BufferUAVOwner sharedOcclusionCullingOutputUAV;
        BufferSRVOwner sharedOcclusionCullingOutputSRV;

        // Index expand
        BufferIBVOwner sharedClusterIndexBuffer;
        BufferUAVOwner sharedClusterIndexBufferUAV;
    };
}
