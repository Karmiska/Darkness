#include "engine/rendering/PrefixSumClusters.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"

using namespace engine;

namespace engine
{
    PrefixSumClusters::PrefixSumClusters(Device& device)
        : m_buckets{ device.createPipeline<shaders::PrefixSumBuckets>() }
        , m_bucketResults{ device.createPipeline<shaders::PrefixSumBucketResults>() }
        , m_addbuckets{ device.createPipeline<shaders::PrefixSumAddBuckets>() }
    {}

    void PrefixSumClusters::prefixSum(
        Device& device,
        CommandList& cmd,
        BufferSRV clusters,
        BufferSRV clusterCounts,
        Buffer args,
        BufferUAV results)
    {
        if (!m_bucketBufferUAV || m_bucketBufferUAV.resource().desc().elements != clusters.desc().elements)
        {
            m_bucketBufferUAV = device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(uint32_t))
                .elements(clusters.desc().elements)
                .format(Format::R32_UINT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("PrefixSumClusters bucket buffer (uint)"));
            m_bucketBufferSRV = device.createBufferSRV(m_bucketBufferUAV);

            m_bucketSumBufferUAV = device.createBufferUAV(BufferDescription()
                .elementSize(sizeof(uint32_t))
                .elements(clusters.desc().elements)
                .format(Format::R32_UINT)
                .usage(ResourceUsage::GpuReadWrite)
                .name("PrefixSumClusters bucket buffer (uint)"));
            m_bucketSumBufferSRV = device.createBufferSRV(m_bucketSumBufferUAV);
        }

        m_buckets.cs.clusterCountBuffer = clusterCounts;
        m_buckets.cs.frustumCullingOutput = clusters;
        m_buckets.cs.output = m_bucketBufferUAV;
        cmd.bindPipe(m_buckets);
        cmd.dispatchIndirect(args, 0);

        m_bucketResults.cs.clusterCountBuffer = clusterCounts;
        m_bucketResults.cs.input = m_bucketBufferSRV;
        m_bucketResults.cs.output = m_bucketSumBufferUAV;
        cmd.bindPipe(m_bucketResults);
        cmd.dispatchIndirect(args, 0);

        m_addbuckets.cs.clusterCountBuffer = clusterCounts;
        m_addbuckets.cs.input1 = m_bucketBufferSRV;
        m_addbuckets.cs.input2 = m_bucketSumBufferSRV;
        m_addbuckets.cs.output = results;
        cmd.bindPipe(m_addbuckets);
        cmd.dispatchIndirect(args, 0);
    }
}
