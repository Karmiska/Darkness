#include "engine/rendering/culling/ModelDataLine.h"
#include "engine/rendering/BufferSettings.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"

namespace engine
{
    #include "shaders/core/shared_types/ClusterInstanceData.hlsli"
    #include "shaders/core/shared_types/ClusterExecuteIndirect.hlsli"

    CountBuffers::CountBuffers(Device& device, size_t elements)
        : buffer{
            device.createBuffer(BufferDescription()
                .elementSize(sizeof(uint32_t))
                .elements(elements)
				.indirectArgument(true)
                .format(Format::R32_UINT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("Count buffer")
            ) }
        , uav{ device.createBufferUAV(buffer) }
        , srv{ device.createBufferSRV(buffer) }
    {}

    ClusterDataLine::ClusterDataLine() {}
    ClusterDataLine::ClusterDataLine(Device& device)
        : clustersUAV{}
        , clustersSRV{}
        , clusterCount{ device }
        , clusterIndexUAV{}
        , clusterIndexSRV{}
    {
        clusterIndexUAV = device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(uint32_t))
                .elements(1)
                .format(Format::R32_UINT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("ModelDataLine cluster index"));
        clusterIndexSRV = device.createBufferSRV(clusterIndexUAV);
    }

    void ClusterDataLine::resize(Device& device, size_t elements, bool shrink)
    {
        auto currentSize = clustersUAV.resource().valid() ? clustersUAV.resource().desc().elements : 0;

        if (elements > 0)
        {
            if (elements > currentSize || (shrink && (elements < currentSize)))
            {
                BufferUAVOwner      _clustersUAV;
                BufferSRVOwner      _clustersSRV;

                _clustersUAV = device.createBufferUAV(BufferDescription()
                    .elementSize(sizeof(ClusterInstanceData))
                    .elements(elements)
                    .usage(ResourceUsage::GpuReadWrite)
                    .structured(true)
                    .name("ModelDataLine clusters"));

                _clustersSRV = device.createBufferSRV(_clustersUAV);

                auto toCopy = std::min(currentSize, elements);
                if(toCopy)
                {
                    auto cmd = device.createCommandList("ClusterDataLine resize");
                    cmd.copyBuffer(
                        clustersSRV.resource().buffer(),
                        _clustersUAV.resource().buffer(),
                        toCopy);
                    device.submitBlocking(cmd);
                }

                clustersUAV = _clustersUAV;
                clustersSRV = _clustersSRV;
            }
        }
        else if(currentSize != 0)
        {
            clustersUAV = {};
            clustersSRV = {};
        }
    }

    void ClusterDataLine::reset(CommandList& cmd)
    {
        cmd.clearBuffer(clusterCount.uav);
        if (clusterIndexUAV)
            cmd.clearBuffer(clusterIndexUAV);
    }

    IndexDataLine::IndexDataLine()
    {}

    IndexDataLine::IndexDataLine(Device& device)
        : indexBuffer{}
        , indexBufferUAV{}
        , indexCount{ device }
        , indexIndexUAV{}
        , indexIndexSRV{}
    {
        indexIndexUAV = device.createBufferUAV(BufferDescription()
            .elementSize(sizeof(uint32_t))
            .elements(1)
            .format(Format::R32_UINT)
            .usage(ResourceUsage::GpuReadWrite)
            .name("ModelDataLine index index (ie. index within index)"));
        indexIndexSRV = device.createBufferSRV(indexIndexUAV);
    }

    IndexDataLine::IndexDataLine(Device& device, size_t elements)
        : indexBuffer{}
        , indexBufferUAV{}
        , indexCount{ device }
        , indexIndexUAV{}
        , indexIndexSRV{}
    {
        if (elements > 0)
        {
            indexBuffer = device.createBufferIBV(BufferDescription()
                .usage(ResourceUsage::GpuReadWrite)
                .format(Format::R32_UINT)
                .name("ModelDataLine index buffer")
                .elements(elements)
                .elementSize(formatBytes(Format::R32_UINT)));

            indexBufferUAV = device.createBufferUAV(indexBuffer);
        }

        indexIndexUAV = device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(uint32_t))
                .elements(1)
                .format(Format::R32_UINT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("ModelDataLine index index (ie. index within index)"));
        indexIndexSRV = device.createBufferSRV(indexIndexUAV);
    }

    void IndexDataLine::reset(CommandList& cmd)
    {
        cmd.clearBuffer(indexCount.uav);
        if (indexIndexUAV)
            cmd.clearBuffer(indexIndexUAV);
    }

    void IndexDataLine::resize(Device& device, size_t elements, bool shrink)
    {
        auto currentSize = indexBuffer.resource().valid() ? indexBuffer.resource().desc().elements : 0;

        if (elements > 0)
        {
            if (elements > currentSize || (shrink && (elements < currentSize)))
            {
                BufferIBVOwner      _indexBuffer;
                BufferUAVOwner      _indexBufferUAV;

                _indexBuffer = device.createBufferIBV(BufferDescription()
                    .usage(ResourceUsage::GpuReadWrite)
                    .format(Format::R32_UINT)
                    .name("ModelDataLine index buffer")
                    .elements(elements)
                    .elementSize(formatBytes(Format::R32_UINT)));

                _indexBufferUAV = device.createBufferUAV(_indexBuffer);

                auto toCopy = std::min(currentSize, elements);
                if(toCopy)
                {
                    auto cmd = device.createCommandList("ClusterDataLine resize");
                    cmd.copyBuffer(
                        indexBuffer.resource().buffer(),
                        _indexBuffer.resource().buffer(),
                        toCopy);
                    device.submitBlocking(cmd);
                }

                indexBuffer = _indexBuffer;
                indexBufferUAV = _indexBufferUAV;
            }
        }
        else if (currentSize != 0)
        {
            indexBuffer = {};
            indexBufferUAV = {};
        }
    }

    DrawDataLine::DrawDataLine()
    {}

    DrawDataLine::DrawDataLine(Device& device)
        : clusterRendererExecuteIndirectArgumentsBuffer{ device.createBuffer(BufferDescription()
            .elementSize(sizeof(ClusterExecuteIndirectArgs))
            .elements(100)
            .usage(ResourceUsage::GpuReadWrite)
            .structured(true)
			.indirectArgument(true)
            .name("ModelDataLine execute indirect draw args")
        ) }
        , clusterRendererExecuteIndirectArgumentsBufferUAV{ device.createBufferUAV(clusterRendererExecuteIndirectArgumentsBuffer) }
        , clusterRendererExecuteIndirectCount{ device, 4 }
    {}

    void DrawDataLine::reset(CommandList& cmd)
    {
        cmd.clearBuffer(clusterRendererExecuteIndirectCount.uav);
    }

    ModelDataLine::ModelDataLine(
        Device& device,
        uint32_t content)
        : clusters{ createCluster(device, content) }
        , indexes{ createIndex(device, content) }
        , draws{ createDraw(device, content) }
    {}

    void ModelDataLine::reset(CommandList& cmd)
    {
        clusters.reset(cmd);
        indexes.reset(cmd);
        draws.reset(cmd);
    }

    ClusterDataLine ModelDataLine::createCluster(Device& device, uint32_t content)
    {
        if (content & ModelDataLine::ModelDataLineContent::Clusters)
            return { device };
        else
            return {};
    }

    IndexDataLine ModelDataLine::createIndex(Device& device, uint32_t content)
    {
        if (content & ModelDataLine::ModelDataLineContent::Indexes)
            return { device };
        else
            return {};
    }

    DrawDataLine ModelDataLine::createDraw(Device& device, uint32_t content)
    {
        if (content & ModelDataLine::ModelDataLineContent::Draws)
            return { device };
        else
            return {};
    }

}
