#pragma once

#include "engine/graphics/ResourceOwners.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/CommonNoDep.h"
#include "shaders/core/tools/PrefixSumBuckets.h"
#include "shaders/core/tools/PrefixSumBucketResults.h"
#include "shaders/core/tools/PrefixSumAddBuckets.h"
#include "containers/vector.h"
#include "containers/string.h"

namespace engine
{
    class Device;
    class PrefixSumClusters
    {
    public:
        PrefixSumClusters(Device& device);
        void prefixSum(
            Device& device,
            CommandList& cmd,
            BufferSRV clusters,
            BufferSRV clusterCounts,
            Buffer args,
            BufferUAV results);
    private:
        engine::Pipeline<shaders::PrefixSumBuckets> m_buckets;
        engine::Pipeline<shaders::PrefixSumBucketResults> m_bucketResults;
        engine::Pipeline<shaders::PrefixSumAddBuckets> m_addbuckets;

        BufferUAVOwner m_bucketBufferUAV;
        BufferSRVOwner m_bucketBufferSRV;

        BufferUAVOwner m_bucketSumBufferUAV;
        BufferSRVOwner m_bucketSumBufferSRV;
    };
}
