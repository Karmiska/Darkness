#pragma once

#include "engine/graphics/ResourceOwners.h"
#include "engine/rendering/ModelResourceAllocator.h"
#include "containers/memory.h"

namespace engine
{
    class Device;
    class CommandList;

    struct CountBuffers
    {
        CountBuffers() {};
        CountBuffers(Device& device, size_t elements = 1);
        BufferOwner                      buffer;
        BufferUAVOwner                   uav;
        BufferSRVOwner                   srv;
    };

    struct ClusterDataLine
    {
        ClusterDataLine();
        ClusterDataLine(Device& device);
        void reset(CommandList& cmd);
        void resize(Device& device, size_t elements, bool shrink = false);

        BufferUAVOwner      clustersUAV;
        BufferSRVOwner      clustersSRV;
        
        CountBuffers        clusterCount;

        BufferUAVOwner      clusterIndexUAV;
        BufferSRVOwner      clusterIndexSRV;
    };

    struct IndexDataLine
    {
        IndexDataLine();
        IndexDataLine(Device& device);
        IndexDataLine(Device& device, size_t elements);
        void reset(CommandList& cmd);
        void resize(Device& device, size_t elements, bool shrink = false);

        BufferIBVOwner      indexBuffer;
        BufferUAVOwner      indexBufferUAV;

        CountBuffers		indexCount;

        BufferUAVOwner      indexIndexUAV;
        BufferSRVOwner      indexIndexSRV;
    };

    struct DrawDataLine
    {
        DrawDataLine();
        DrawDataLine(Device& device);
        void reset(CommandList& cmd);

        BufferOwner         clusterRendererExecuteIndirectArgumentsBuffer;
        BufferUAVOwner      clusterRendererExecuteIndirectArgumentsBufferUAV;

        CountBuffers		clusterRendererExecuteIndirectCount;
    };

    struct ModelDataLine
    {
        enum ModelDataLineContent
        {
            Clusters = 0x1,
            Indexes = 0x2,
            Draws = 0x4
        };
        ModelDataLine(
            Device& device,
            uint32_t content = 
                ModelDataLineContent::Clusters |
                ModelDataLineContent::Indexes | 
                ModelDataLineContent::Draws);

        void reset(CommandList& cmd);

        ClusterDataLine clusters;
        IndexDataLine indexes;
        DrawDataLine draws;

        ClusterDataLine createCluster(Device& device, uint32_t content);
        IndexDataLine createIndex(Device& device, uint32_t content);
        DrawDataLine createDraw(Device& device, uint32_t content);
    };

}
