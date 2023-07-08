#include "engine/graphics/Queue.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/SwapChain.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/CommonNoDep.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/Fence.h"
#include "engine/graphics/dx12/DX12Queue.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/Fence.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/vulkan/VulkanQueue.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalQueue.h"
#endif


using namespace tools;
using namespace engine::implementation;

namespace engine
{
    Queue::Queue(Device& device, CommandListType type, GraphicsApi api, const char* queueName)
        : m_impl{}
        , m_device{ device }
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::make_unique<QueueImplDX12>(device, type, queueName);
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::make_unique<QueueImplVulkan>(device, type, queueName);
    }

    /*void Queue::submit(engine::vector<CommandList>& commandLists)
    {
        m_impl->submit(commandLists);

        for (auto&& commandList : commandLists)
            handleShaderDebug(commandList);
    }*/

    void Queue::submit(CommandList& commandList)
    {
        m_impl->submit(commandList);
        handleShaderDebug(commandList);
    }

    /*void Queue::submit(engine::vector<CommandList>& commandLists, Fence& fence)
    {
        m_impl->submit(commandLists, fence);

        for (auto&& commandList : commandLists)
            handleShaderDebug(commandList);
    }*/

    void Queue::submit(CommandList& commandList, Fence& fence)
    {
        m_impl->submit(commandList, fence);
        handleShaderDebug(commandList);
    }

    void Queue::submit(CommandList& commandList, Semaphore& semaphore)
    {
        m_impl->submit(commandList, semaphore);
        handleShaderDebug(commandList);
    }

    void Queue::submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore)
    {
        m_impl->submit(commandList, waitSemaphore, signalSemaphore);
        handleShaderDebug(commandList);
    }

    void Queue::submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore, Fence& fence)
    {
        m_impl->submit(commandList, waitSemaphore, signalSemaphore, fence);
        handleShaderDebug(commandList);
    }

    void Queue::submit(CommandList& commandList, Semaphore& semaphore, Fence& fence)
    {
        m_impl->submit(commandList, semaphore, fence);
        handleShaderDebug(commandList);
    }

    void Queue::waitForIdle() const
    {
        m_impl->waitForIdle();
    }

    void Queue::signal(const Semaphore& semaphore)
    {
        m_impl->signal(semaphore);
    }

    void Queue::signal(const Fence& fence, unsigned long long value)
    {
        m_impl->signal(fence, value);
    }

    uint64_t Queue::timeStampFrequency() const
    {
        return m_impl->timeStampFrequency();
    }

    void Queue::present(
        Semaphore& signalSemaphore,
        SwapChain& swapChain,
        unsigned int chainIndex)
    {
        m_impl->present(signalSemaphore, swapChain, chainIndex);
    }

    bool Queue::needRefresh() const
    {
        return m_impl->needRefresh();
    }

    engine::Vector3<size_t> Queue::transferGranularity() const
    {
        return m_impl->transferGranularity();
    }

    void Queue::handleShaderDebug(CommandList& commandList)
    {
        CPU_MARKER(commandList.api(), "Handle shader debug");
        
        bool thereWasShaderDebug = false;
        for (auto&& debugBuffer : commandList.debugBuffers())
        {
            if (!m_debugOutputCpu)
            {
                m_debugOutputCpu = m_device.createBuffer(BufferDescription()
                    .elementSize(sizeof(ShaderDebugOutput))
                    .elements(100000)
                    .name("shaderDebut cpu buffer")
                    .structured(true)
                    .usage(ResourceUsage::GpuToCpu)
                );
                m_debugOutputCpuCount = m_device.createBuffer(BufferDescription()
                    .elementSize(sizeof(uint32_t))
                    .elements(1)
                    .name("shaderDebut buffer counter")
                    .usage(ResourceUsage::GpuToCpu)
                );
            }

            // copy shader debug structure count
            {
                auto cmd = m_device.createCommandList("Queue::handleShaderDebug structure counts");
                
                {
                    CPU_MARKER(cmd.api(), "Copy shader debug structure count");
                    GPU_MARKER(cmd, "Copy shader debug structure count");

                    cmd.copyStructureCounter(debugBuffer, m_debugOutputCpuCount, 0);
                }
                auto fence = m_device.createFence("Queue shader debug structure copy fence");
                fence.increaseCPUValue();
                submit(cmd, fence);
                fence.blockUntilSignaled();
            }

            // copy shader debug output to cpu buffer
            uint32_t counter = 0;
            {
                {
                    auto contents = static_cast<uint32_t*>(m_debugOutputCpuCount.resource().map(m_device));
                    memcpy(&counter, contents, sizeof(uint32_t));
                    m_debugOutputCpuCount.resource().unmap(m_device);

                    if(counter > 0u && counter < static_cast<uint32_t>(m_debugOutputCpu.resource().description().elements))
                    {
                        auto cmd = m_device.createCommandList("Queue::handleShaderDebug debug buffer");
                        {
                            CPU_MARKER(cmd.api(), "Copy shader debug buffer");
                            GPU_MARKER(cmd, "Copy shader debug buffer");
                            cmd.copyBuffer(debugBuffer.resource().buffer(), m_debugOutputCpu, counter, 0, 0);
                        }
                        auto fence = m_device.createFence("Queue shader debug output cpu fence");
                        fence.increaseCPUValue();
                        submit(cmd, fence);
                        fence.blockUntilSignaled();
                    }
                    else if (counter > 0)
                    {
                        LOG_ERROR("Debug output counter was bigger than debug buffer: %u", counter);
                    }
                }
                
            }

            // output shader debug contents
            if(counter > 0u && counter < static_cast<uint32_t>(m_debugOutputCpu.resource().description().elements))
            {
                auto debugContents = static_cast<ShaderDebugOutput*>(m_debugOutputCpu.resource().map(m_device));
                for (uint32_t i = 0; i < counter; ++i)
                {
                    switch (debugContents[i].itemType)
                    {
                    case 1: LOG("ShaderDebug: %s", debugContents[i].uvalue > 0 ? "true" : "false"); break;
                    case 2: LOG("ShaderDebug: %i", static_cast<int>(debugContents[i].uvalue)); break;
                    case 3: LOG("ShaderDebug: %u", static_cast<uint32_t>(debugContents[i].uvalue)); break;
                    case 4: LOG("ShaderDebug: %f", debugContents[i].fvalue); break;
                    }
                }
                m_debugOutputCpu.resource().unmap(m_device);
            }

            if (counter > 0u && counter < static_cast<uint32_t>(m_debugOutputCpu.resource().description().elements))
                thereWasShaderDebug = true;
        }
        commandList.debugBuffers().clear();

        if (thereWasShaderDebug)
            LOG("SHADER DEBUG NEXT");
    }
}
