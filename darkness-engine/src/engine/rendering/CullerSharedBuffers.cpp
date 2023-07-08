#include "engine/rendering/CullerSharedBuffers.h"
#include "engine/graphics/Device.h"
#include "engine/rendering/BufferSettings.h"

namespace engine
{
    #include "shaders/core/shared_types/ClusterInstanceData.hlsli"

    CullerSharedBuffers::CullerSharedBuffers(Device& device)
        : sharedOcclusionCullingInputUAV{
            device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(ClusterInstanceData))
                .elements(MaxClusterInstanceCount)
                .usage(ResourceUsage::GpuReadWrite)
                .structured(true)
                .name("Culler shared occlusion culling input")
            ) }
        , sharedOcclusionCullingInputSRV{ device.createBufferSRV(sharedOcclusionCullingInputUAV) }

        , sharedOcclusionCullingOutputUAV{
            device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(ClusterInstanceData))
                .elements(MaxClusterInstanceCount)
                .usage(ResourceUsage::GpuReadWrite)
                .structured(true)
                .name("Culler shared occlusion culling output")
            ) }
        , sharedOcclusionCullingOutputSRV{ device.createBufferSRV(sharedOcclusionCullingOutputUAV) }

        , sharedClusterIndexBuffer{
            device.createBufferIBV(BufferDescription()
            .usage(ResourceUsage::GpuReadWrite)
                .format(Format::R32_UINT)
                .name("Culler shared index Buffer")
                .elements(MaxIndexCount)
                .elementSize(formatBytes(Format::R32_UINT)))
        }
        , sharedClusterIndexBufferUAV{ device.createBufferUAV(sharedClusterIndexBuffer) }
    {}
}
