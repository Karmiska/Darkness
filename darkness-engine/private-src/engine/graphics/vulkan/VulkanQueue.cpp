#include "engine/graphics/vulkan/VulkanQueue.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanCommandList.h"
#include "engine/graphics/vulkan/VulkanSemaphore.h"
#include "engine/graphics/vulkan/VulkanSwapChain.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanFence.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/SwapChain.h"
#include "engine/graphics/Fence.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        QueueImplVulkan::QueueImplVulkan(const Device& device, CommandListType type, const char* /*queueName*/)
            : m_queue{ nullptr }
            , m_device{ device }
            , m_queueInfo{ engine::make_unique<QueueInfo>(static_cast<const DeviceImplVulkan*>(device.native())->createQueue(type)) }
            , m_needRefresh{ false }
        {
            ASSERT(m_queueInfo->queueFamilyIndex != InvalidFamilyIndex);
            
            vkGetDeviceQueue(
                static_cast<const DeviceImplVulkan*>(device.native())->device(),
                static_cast<uint32_t>(m_queueInfo->queueFamilyIndex),
                static_cast<uint32_t>(m_queueInfo->queueIndex),
                &m_queue);
            ASSERT(m_queue != nullptr);
        }

        QueueImplVulkan::~QueueImplVulkan()
        {
            static_cast<const DeviceImplVulkan*>(m_device.native())->destroyQueue(*m_queueInfo.get());
        }

        const VkQueue& QueueImplVulkan::native() const
        {
            return m_queue;
        }

        void QueueImplVulkan::submit(engine::vector<CommandList>& /*commandLists*/)
        {
            ASSERT(false);
        }

        void QueueImplVulkan::submit(CommandList& commandList)
        {
            VkPipelineStageFlags m_waitStages[1];
            m_waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSubmitInfo m_submitInfo = {};
            m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            m_submitInfo.waitSemaphoreCount = 1;

            ASSERT(false, "some semaphore shit");
            /*m_submitInfo.pWaitSemaphores = &SemaphoreImplGet::impl(
                CommandListImplGet::impl(commandList).finishedSemaphore()).native();*/
            m_submitInfo.pWaitDstStageMask = m_waitStages;
            m_submitInfo.commandBufferCount = 1;
            m_submitInfo.pCommandBuffers = &static_cast<const CommandListImplVulkan*>(commandList.native())->native();
            m_submitInfo.signalSemaphoreCount = 0;
            m_submitInfo.pSignalSemaphores = nullptr;
            //m_submitInfo.signalSemaphoreCount = 1;
            //m_submitInfo.pSignalSemaphores = &CommandListImplGet::impl(commandList).barrier();
                

            auto result = vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE);
            //auto result = vkQueueSubmit(m_queue, 1, &CommandListImplGet::impl(commandList).barrier(), VK_NULL_HANDLE);
            ASSERT(result == VK_SUCCESS);

            //vkQueueWaitIdle(m_queue);
        }

        void QueueImplVulkan::submit(engine::vector<CommandList>& /*commandLists*/, Fence& /*fence*/)
        {
            ASSERT(false);
        }

        void QueueImplVulkan::submit(CommandList& commandList, Fence& fence)
        {
            //vkCmdEndRenderPass(CommandListImplGet::impl(commandList).native());
            auto& commandListImpl = *const_cast<CommandListImplVulkan*>(static_cast<const CommandListImplVulkan*>(commandList.native()));
            if (commandListImpl.isOpen())
            {
                commandListImpl.applyBarriers();
                commandListImpl.resolveQueries();
                commandListImpl.end();
            }

            VkPipelineStageFlags m_waitStages[1];
            m_waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSubmitInfo m_submitInfo = {};
            m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            m_submitInfo.waitSemaphoreCount = 0;
            m_submitInfo.pWaitSemaphores = nullptr;
            m_submitInfo.pWaitDstStageMask = m_waitStages;
            m_submitInfo.commandBufferCount = 1;
            m_submitInfo.pCommandBuffers = &commandListImpl.native();
            m_submitInfo.signalSemaphoreCount = 1;
            m_submitInfo.pSignalSemaphores = &static_cast<FenceImplVulkan*>(fence.native())->native();
            
            auto fenceValue = fence.currentCPUValue();

            VkTimelineSemaphoreSubmitInfo tsSubmitInfo = {};
            tsSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
            tsSubmitInfo.waitSemaphoreValueCount = 0;
            tsSubmitInfo.pWaitSemaphoreValues = nullptr;
            tsSubmitInfo.signalSemaphoreValueCount = 1;
            tsSubmitInfo.pSignalSemaphoreValues = &fenceValue;

            m_submitInfo.pNext = &tsSubmitInfo;
                
            auto result = vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE);
            ASSERT(result == VK_SUCCESS);

            //vkQueueWaitIdle(m_queue);
        }

        void QueueImplVulkan::submit(engine::vector<CommandList>& /*commandLists*/ , Semaphore& /*semaphore*/)
        {
            ASSERT(false);
        }

        void QueueImplVulkan::submit(CommandList& commandList, Semaphore& semaphore)
        {
            //vkCmdEndRenderPass(CommandListImplGet::impl(commandList).native());
            auto& commandListImpl = *const_cast<CommandListImplVulkan*>(static_cast<const CommandListImplVulkan*>(commandList.native()));
            if (commandListImpl.isOpen())
            {
                commandListImpl.applyBarriers();
                commandListImpl.resolveQueries();
                commandListImpl.end();
            }

            VkPipelineStageFlags m_waitStages[1];
            m_waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSubmitInfo m_submitInfo = {};
            m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            m_submitInfo.waitSemaphoreCount = 0;
            m_submitInfo.pWaitSemaphores = nullptr;
            m_submitInfo.pWaitDstStageMask = m_waitStages;
            m_submitInfo.commandBufferCount = 1;
            m_submitInfo.pCommandBuffers = &commandListImpl.native();
            m_submitInfo.signalSemaphoreCount = 1;
            m_submitInfo.pSignalSemaphores = &static_cast<SemaphoreImplVulkan*>(semaphore.native())->native();

            auto result = vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE);
            ASSERT(result == VK_SUCCESS);

            //vkQueueWaitIdle(m_queue);
        }

        void QueueImplVulkan::submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore)
        {
            //vkCmdEndRenderPass(CommandListImplGet::impl(commandList).native());
            auto& commandListImpl = *const_cast<CommandListImplVulkan*>(static_cast<const CommandListImplVulkan*>(commandList.native()));
            if (commandListImpl.isOpen())
            {
                commandListImpl.applyBarriers();
                commandListImpl.resolveQueries();
                commandListImpl.end();
            }

            VkPipelineStageFlags m_waitStages[1];
            if (commandList.type() == CommandListType::Direct)
                m_waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            else
                m_waitStages[0] = VK_PIPELINE_STAGE_TRANSFER_BIT;

            VkSubmitInfo m_submitInfo = {};
            m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            m_submitInfo.waitSemaphoreCount = 1;
            m_submitInfo.pWaitSemaphores = &static_cast<SemaphoreImplVulkan*>(waitSemaphore.native())->native();
            m_submitInfo.pWaitDstStageMask = m_waitStages;
            m_submitInfo.commandBufferCount = 1;
            m_submitInfo.pCommandBuffers = &commandListImpl.native();
            m_submitInfo.signalSemaphoreCount = 1;
            m_submitInfo.pSignalSemaphores = &static_cast<SemaphoreImplVulkan*>(signalSemaphore.native())->native();

            auto result = vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE);
            ASSERT(result == VK_SUCCESS);
        }

        void QueueImplVulkan::submit(CommandList& commandList, Semaphore& waitSemaphore, Semaphore& signalSemaphore, Fence& fence)
        {
            //vkCmdEndRenderPass(CommandListImplGet::impl(commandList).native());
            auto& commandListImpl = *const_cast<CommandListImplVulkan*>(static_cast<const CommandListImplVulkan*>(commandList.native()));
            if (commandListImpl.isOpen())
            {
                commandListImpl.applyBarriers();
                commandListImpl.resolveQueries();
                commandListImpl.end();
            }

            VkPipelineStageFlags m_waitStages[1];
            m_waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSemaphore semaphores[2];
            semaphores[0] = static_cast<SemaphoreImplVulkan*>(signalSemaphore.native())->native();
            semaphores[1] = static_cast<FenceImplVulkan*>(fence.native())->native();

            VkSubmitInfo m_submitInfo = {};
            m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            m_submitInfo.waitSemaphoreCount = 1;
            m_submitInfo.pWaitSemaphores = &static_cast<SemaphoreImplVulkan*>(waitSemaphore.native())->native();
            m_submitInfo.pWaitDstStageMask = m_waitStages;
            m_submitInfo.commandBufferCount = 1;
            m_submitInfo.pCommandBuffers = &commandListImpl.native();
            m_submitInfo.signalSemaphoreCount = 2;
            m_submitInfo.pSignalSemaphores = &semaphores[0];

            auto result = vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE);
            ASSERT(result == VK_SUCCESS);
        }

        void QueueImplVulkan::submit(CommandList& commandList, Semaphore& semaphore, Fence& fence)
        {
            //vkCmdEndRenderPass(CommandListImplGet::impl(commandList).native());
            auto& commandListImpl = *const_cast<CommandListImplVulkan*>(static_cast<const CommandListImplVulkan*>(commandList.native()));
            if (commandListImpl.isOpen())
            {
                commandListImpl.applyBarriers();
                commandListImpl.resolveQueries();
                commandListImpl.end();
            }

            VkPipelineStageFlags m_waitStages[1];
            m_waitStages[0] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            VkSemaphore semaphores[2];
            semaphores[0] = static_cast<SemaphoreImplVulkan*>(semaphore.native())->native();
            semaphores[1] = static_cast<FenceImplVulkan*>(fence.native())->native();

            VkSubmitInfo m_submitInfo = {};
            m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            m_submitInfo.waitSemaphoreCount = 0;
            m_submitInfo.pWaitSemaphores = nullptr;
            m_submitInfo.pWaitDstStageMask = m_waitStages;
            m_submitInfo.commandBufferCount = 1;
            m_submitInfo.pCommandBuffers = &commandListImpl.native();
            m_submitInfo.signalSemaphoreCount = 2;
            m_submitInfo.pSignalSemaphores = &semaphores[0];

            auto result = vkQueueSubmit(m_queue, 1, &m_submitInfo, VK_NULL_HANDLE);
            ASSERT(result == VK_SUCCESS);

            //vkQueueWaitIdle(m_queue);
        }

        void QueueImplVulkan::waitForIdle() const
        {
            vkQueueWaitIdle(m_queue);
        }

        void QueueImplVulkan::signal(const Semaphore& semaphore)
        {
            unsigned long long value = 1;

            VkTimelineSemaphoreSubmitInfo timelineInfo1;
            timelineInfo1.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
            timelineInfo1.pNext = NULL;
            timelineInfo1.waitSemaphoreValueCount = 0;
            timelineInfo1.pWaitSemaphoreValues = nullptr;
            timelineInfo1.signalSemaphoreValueCount = 1;
            timelineInfo1.pSignalSemaphoreValues = &value;

            VkSubmitInfo info1;
            info1.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info1.pNext = &timelineInfo1;
            info1.waitSemaphoreCount = 0;
            info1.pWaitSemaphores = nullptr;
            info1.signalSemaphoreCount = 1;
            info1.pSignalSemaphores = &static_cast<const SemaphoreImplVulkan*>(semaphore.native())->native();
            info1.commandBufferCount = 0;
            info1.pCommandBuffers = nullptr;

            vkQueueSubmit(m_queue, 1, &info1, VK_NULL_HANDLE);
        }

        void QueueImplVulkan::signal(const Fence& fence, unsigned long long value)
        {
            VkTimelineSemaphoreSubmitInfo timelineInfo1;
            timelineInfo1.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
            timelineInfo1.pNext = NULL;
            timelineInfo1.waitSemaphoreValueCount = 0;
            timelineInfo1.pWaitSemaphoreValues = nullptr;
            timelineInfo1.signalSemaphoreValueCount = 1;
            timelineInfo1.pSignalSemaphoreValues = &value;

            VkSubmitInfo info1;
            info1.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info1.pNext = &timelineInfo1;
            info1.waitSemaphoreCount = 0;
            info1.pWaitSemaphores = nullptr;
            info1.signalSemaphoreCount = 1;
            info1.pSignalSemaphores = &static_cast<const FenceImplVulkan*>(fence.native())->native();
            info1.commandBufferCount = 0;
            info1.pCommandBuffers = nullptr;

            vkQueueSubmit(m_queue, 1, &info1, VK_NULL_HANDLE);
        }

        void QueueImplVulkan::present(
            Semaphore& /*signalSemaphore*/,
            SwapChain& swapChain,
            unsigned int chainIndex)
        {
            //VkSemaphore signalSemaphores[] = { static_cast<SemaphoreImplVulkan*>(signalSemaphore.native())->native() };

            VkSemaphore signalSemaphores[] = { 
                static_cast<SemaphoreImplVulkan*>(const_cast<SwapChainImplVulkan*>(static_cast<SwapChainImplVulkan*>(swapChain.native()))->backBufferReadySemaphore().native())->native() };

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapChains[] = { static_cast<SwapChainImplVulkan*>(swapChain.native())->native() };
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapChains;

            uint32_t imageIndex = static_cast<uint32_t>(chainIndex);
            presentInfo.pImageIndices = &imageIndex;

            VkResult res = vkQueuePresentKHR(m_queue, &presentInfo);
            if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
            {
                m_needRefresh = true;
                res = VK_SUCCESS;
                swapChain.recreate();
            }
            else if (res != VK_SUCCESS)
            {
                ASSERT(res == VK_SUCCESS);
            }
            else
                const_cast<SwapChainImplVulkan*>(static_cast<SwapChainImplVulkan*>(swapChain.native()))->present();

            if (static_cast<SwapChainImplVulkan*>(swapChain.native())->vsyncChange())
            {
                static_cast<SwapChainImplVulkan*>(swapChain.native())->vsyncChange(false);
                swapChain.recreate();
            }
        }

        bool QueueImplVulkan::needRefresh() const
        {
            return m_needRefresh;
        }

        engine::Vector3<size_t> QueueImplVulkan::transferGranularity() const
        {
            return m_queueInfo->minImageTransferGranularity;
        }
    }
}
