#pragma once

#include "engine/graphics/Pipeline.h"
#include "shaders/core/tools/PrefixScan.h"
#include "shaders/core/tools/PrefixScanUpSweep.h"
#include "engine/graphics/ResourceOwners.h"

#include "containers/vector.h"

namespace engine
{
    class CommandList;
    class BufferSRV;
    class BufferUAV;

    constexpr size_t PrefixSumShaderThreadGroupSize = 64ull;

    class PrefixSum
    {
    public:
        PrefixSum(Device& device);

        void perform(
            CommandList& cmd,
            const BufferSRV src,
            const BufferUAV dst);

    private:
        Device& m_device;
        engine::Pipeline<shaders::PrefixScan>           m_prefixSum;
        engine::Pipeline<shaders::PrefixScanUpSweep>    m_prefixSumUpSweep;

        class WorkItem
        {
        public:
            WorkItem(Device& device, size_t size);
            void resize(size_t size);

            BufferUAVOwner inputUAV;
            BufferSRVOwner inputSRV;

            BufferUAVOwner sumsUAV;
            BufferSRVOwner sumsSRV;

            BufferUAVOwner prefixsumsUAV;
            BufferSRVOwner prefixsumsSRV;

            BufferUAVOwner outputUAV;
            BufferSRVOwner outputSRV;
        private:
            Device* m_device;
        };

        engine::vector<WorkItem> m_work;

        BufferUAVOwner smallSumsUAV;
        BufferSRVOwner smallSumsSRV;

        BufferUAVOwner smallSumsOutputUAV;
        BufferSRVOwner smallSumsOutputSRV;

        void resizeWorkBuffers(size_t size);
        size_t workCount(size_t size) const;
    };
}
