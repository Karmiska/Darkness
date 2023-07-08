#include "engine/graphics/vulkan/VulkanBarrier.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanSemaphore.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/vulkan/VulkanCommandList.h"
#include "engine/graphics/vulkan/VulkanResources.h"
#include "engine/graphics/vulkan/VulkanConversions.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        BarrierImplVulkan::BarrierImplVulkan(
            const CommandList& commandList,
            ResourceBarrierFlags /*flags*/,
            const TextureRTV& /*resource*/,
            ResourceState /*before*/,
            ResourceState /*after*/,
            unsigned int /*subResource*/,
            const Semaphore& /*waitSemaphore*/,
            const Semaphore& /*signalSemaphore*/)
            //: m_barrier{ new D3D12_RESOURCE_BARRIER() }
            //, m_commandList{ commandList }
            : m_commandList{ commandList }
            , m_submitInfo{}
        {
            /*
            VkMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            barrier.dstAccessMask*/

            /*m_waitSem[0] = SemaphoreImplGet::impl(waitSemaphore).native();
            m_signalSemaphores[0] = SemaphoreImplGet::impl(signalSemaphore).native();
            m_waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            m_submitInfo.waitSemaphoreCount = 1;
            m_submitInfo.pWaitSemaphores = m_waitSem;
            m_submitInfo.pWaitDstStageMask = m_waitStages;
            m_submitInfo.commandBufferCount = 1;
            m_submitInfo.pCommandBuffers = &CommandListImplGet::impl(commandList).native();
            m_submitInfo.signalSemaphoreCount = 1;
            m_submitInfo.pSignalSemaphores = m_signalSemaphores;

            auto test =  CommandListImplGet::impl(m_commandList).native(); //.setResourceBarrier(m_submitInfo);
            test.*/

            ASSERT(false, "Are we even using these?");
        }

        void BarrierImplVulkan::update(
            ResourceState /*before*/,
            ResourceState /*after*/)
        {
        }

        BarrierImplVulkan::~BarrierImplVulkan()
        {
        }
    }
}
