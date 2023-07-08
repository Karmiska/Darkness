#include "engine/rendering/tools/PrefixSum.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include <algorithm>

namespace engine
{
    PrefixSum::WorkItem::WorkItem(Device& device, size_t size)
        : m_device{ &device }
        , inputUAV{ device.createBufferUAV(engine::BufferDescription()
            .format(Format::R32_UINT)
            .elements(roundUpToMultiple(size, PrefixSumShaderThreadGroupSize))
            .name("Prefix sum input buffer")
            .usage(engine::ResourceUsage::GpuReadWrite)) }
        , inputSRV{ device.createBufferSRV(inputUAV) }

        , sumsUAV{ device.createBufferUAV(engine::BufferDescription()
            .format(Format::R32_UINT)
            .elements(roundUpToMultiple(size, PrefixSumShaderThreadGroupSize) / PrefixSumShaderThreadGroupSize)
            .name("Prefix sum input buffer")
            .usage(engine::ResourceUsage::GpuReadWrite)) }
        , sumsSRV{ device.createBufferSRV(sumsUAV) }

        , prefixsumsUAV{ device.createBufferUAV(engine::BufferDescription()
            .format(Format::R32_UINT)
            .elements(roundUpToMultiple(size, PrefixSumShaderThreadGroupSize) / PrefixSumShaderThreadGroupSize)
            .name("Prefix sum input buffer")
            .usage(engine::ResourceUsage::GpuReadWrite)) }
        , prefixsumsSRV{ device.createBufferSRV(prefixsumsUAV) }

        , outputUAV{ device.createBufferUAV(engine::BufferDescription()
            .format(Format::R32_UINT)
            .elements(roundUpToMultiple(size, PrefixSumShaderThreadGroupSize))
            .name("Prefix sum output buffer")
            .usage(engine::ResourceUsage::GpuReadWrite)) }
        , outputSRV{ device.createBufferSRV(outputUAV) }
    {}

    void PrefixSum::WorkItem::resize(size_t size)
    {
        if (size > inputUAV.resource().buffer().description().elements)
        {
            inputUAV = m_device->createBufferUAV(engine::BufferDescription()
                .format(Format::R32_UINT)
                .elements(roundUpToMultiple(size, PrefixSumShaderThreadGroupSize))
                .name("Prefix sum input buffer")
                .usage(engine::ResourceUsage::GpuReadWrite));
            inputSRV = m_device->createBufferSRV(inputUAV);

            sumsUAV = m_device->createBufferUAV(engine::BufferDescription()
                .format(Format::R32_UINT)
                .elements(roundUpToMultiple(size, PrefixSumShaderThreadGroupSize) / PrefixSumShaderThreadGroupSize)
                .name("Prefix sum input buffer")
                .usage(engine::ResourceUsage::GpuReadWrite));
            sumsSRV = m_device->createBufferSRV(sumsUAV);

            outputUAV = m_device->createBufferUAV(engine::BufferDescription()
                .format(Format::R32_UINT)
                .elements(roundUpToMultiple(size, PrefixSumShaderThreadGroupSize))
                .name("Prefix sum output buffer")
                .usage(engine::ResourceUsage::GpuReadWrite));
            outputSRV = m_device->createBufferSRV(outputUAV);
        }
    }

    PrefixSum::PrefixSum(Device& device)
        : m_device{ device }
        , m_prefixSum{ device.createPipeline<shaders::PrefixScan>() }
        , m_prefixSumUpSweep{ device.createPipeline<shaders::PrefixScanUpSweep>() }
        , smallSumsUAV{ device.createBufferUAV(engine::BufferDescription()
            .format(Format::R32_UINT)
            .elements(PrefixSumShaderThreadGroupSize)
            .name("Prefix sum input buffer")
            .usage(engine::ResourceUsage::GpuReadWrite)) }
        , smallSumsSRV{ device.createBufferSRV(smallSumsUAV) }

        , smallSumsOutputUAV{ device.createBufferUAV(engine::BufferDescription()
            .format(Format::R32_UINT)
            .elements(PrefixSumShaderThreadGroupSize)
            .name("Prefix sum input buffer")
            .usage(engine::ResourceUsage::GpuReadWrite)) }
        , smallSumsOutputSRV{ device.createBufferSRV(smallSumsOutputUAV) }
    {}

    void PrefixSum::perform(
        CommandList& cmd,
        const BufferSRV src,
        const BufferUAV dst)
    {
        auto srcSize = src.buffer().description().elements;
        int count = static_cast<int>(workCount(srcSize));
        

        if(count == 1)
        {
            {
                m_prefixSum.cs.src = src;
                m_prefixSum.cs.dst = dst;
                m_prefixSum.cs.workBuffer = smallSumsUAV;
                m_prefixSum.cs.outputsums = true;
                m_prefixSum.cs.count.x = static_cast<uint32_t>(srcSize);
                m_prefixSum.cs.inclusive.x = 0u;
                cmd.bindPipe(m_prefixSum);
                cmd.dispatch(roundUpToMultiple(srcSize, PrefixSumShaderThreadGroupSize) / PrefixSumShaderThreadGroupSize, 1ull, 1ull);
            }

            {
                m_prefixSum.cs.src = smallSumsSRV;
                m_prefixSum.cs.dst = smallSumsOutputUAV;
                m_prefixSum.cs.outputsums = false;
                m_prefixSum.cs.count.x = static_cast<uint32_t>(std::max(srcSize / PrefixSumShaderThreadGroupSize, 1ull));
                m_prefixSum.cs.inclusive.x = 0u;
                cmd.bindPipe(m_prefixSum);
                cmd.dispatch(1, 1, 1);
            }

            m_prefixSumUpSweep.cs.dst = dst;
            m_prefixSumUpSweep.cs.workBuffer = smallSumsOutputSRV;
            m_prefixSumUpSweep.cs.count.x = static_cast<uint32_t>(srcSize);
            cmd.bindPipe(m_prefixSumUpSweep);
            cmd.dispatch(roundUpToMultiple(srcSize, PrefixSumShaderThreadGroupSize) / PrefixSumShaderThreadGroupSize, 1ull, 1ull);
        }
        else
        {
            resizeWorkBuffers(srcSize);

            m_prefixSum.cs.src =        src;
            m_prefixSum.cs.dst =        dst;
            m_prefixSum.cs.workBuffer = m_work[0].inputUAV;
            m_prefixSum.cs.outputsums = true;
            m_prefixSum.cs.count.x = static_cast<uint32_t>(srcSize);
            m_prefixSum.cs.inclusive.x = 0u;
            cmd.bindPipe(m_prefixSum);
            cmd.dispatch(roundUpToMultiple(srcSize, PrefixSumShaderThreadGroupSize) / PrefixSumShaderThreadGroupSize, 1ull, 1ull);

            size_t currentSize = srcSize / PrefixSumShaderThreadGroupSize;
            engine::vector<size_t> counts(count);
            for (int i = 0; i < count-1; ++i)
            {
                m_prefixSum.cs.src =        m_work[i].inputSRV;
                m_prefixSum.cs.dst =        m_work[i].outputUAV;
                m_prefixSum.cs.workBuffer = m_work[i + 1].inputUAV;
                m_prefixSum.cs.outputsums = true;
                m_prefixSum.cs.count.x = static_cast<uint32_t>(currentSize);
                m_prefixSum.cs.inclusive.x = 0u;
                cmd.bindPipe(m_prefixSum);
                cmd.dispatch(roundUpToMultiple(currentSize, 64ull) / 64ull, 1ull, 1ull);
                counts[i] = currentSize;
                currentSize /= PrefixSumShaderThreadGroupSize;
            }

            for (int i = count - 1; i > 0; --i)
            {
                {
                    m_prefixSum.cs.src = m_work[i].sumsSRV;
                    m_prefixSum.cs.dst = m_work[i].prefixsumsUAV;
                    m_prefixSum.cs.workBuffer = m_work[i + 1].inputUAV;
                    m_prefixSum.cs.outputsums = true;
                    m_prefixSum.cs.count.x = static_cast<uint32_t>(currentSize);
                    m_prefixSum.cs.inclusive.x = 0u;
                    cmd.bindPipe(m_prefixSum);
                    cmd.dispatch(roundUpToMultiple(currentSize, 64ull) / 64ull, 1ull, 1ull);
                    counts[i] = currentSize;
                    currentSize /= PrefixSumShaderThreadGroupSize;
                }

                {
                    m_prefixSumUpSweep.cs.dst = m_work[i - 1].outputUAV;
                    m_prefixSumUpSweep.cs.workBuffer = m_work[i].sumsSRV;
                    m_prefixSumUpSweep.cs.count.x = static_cast<uint32_t>(counts[i]);
                    cmd.bindPipe(m_prefixSumUpSweep);
                    cmd.dispatch(roundUpToMultiple(currentSize, 64ull) / 64ull, 1ull, 1ull);
                }
            }

            {
                m_prefixSumUpSweep.cs.dst = dst;
                m_prefixSumUpSweep.cs.workBuffer = m_work[0].outputSRV;
                m_prefixSumUpSweep.cs.count.x = static_cast<uint32_t>(srcSize);
                cmd.bindPipe(m_prefixSumUpSweep);
                cmd.dispatch(roundUpToMultiple(currentSize, 64ull) / 64ull, 1ull, 1ull);
            }
        }
    }

    void PrefixSum::resizeWorkBuffers(size_t size)
    {
        int count = static_cast<int>(workCount(size));

        size_t workBufferSize = size / PrefixSumShaderThreadGroupSize;
        for (int i = 0; i < count; ++i)
        {
            if (i >= m_work.size())
            {
                m_work.emplace_back(WorkItem(m_device, workBufferSize));
            }
            else
            {
                m_work[i].resize(size);
            }
            workBufferSize /= PrefixSumShaderThreadGroupSize;
        }
    }

    size_t PrefixSum::workCount(size_t size) const
    {
        int workCount = 1;
        int currentSize = static_cast<int>(size / PrefixSumShaderThreadGroupSize);
        while (currentSize > PrefixSumShaderThreadGroupSize)
        {
            ++workCount;
            currentSize /= PrefixSumShaderThreadGroupSize;
        }
        return workCount;
    }
}
