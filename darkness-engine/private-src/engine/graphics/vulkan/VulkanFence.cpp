#include "engine/graphics/vulkan/VulkanFence.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanInstance.h"

#include "engine/graphics/Device.h"
#include "engine/graphics/vulkan/VulkanDevice.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        FenceImplVulkan::FenceImplVulkan(const DeviceImplIf* device, const char* name)
            : m_fence{ nullptr }
            , m_device{ static_cast<const DeviceImplVulkan*>(device) }
            , m_fenceValue{ 0 }
        {
            m_fence = m_device->m_fencePool.acquire();
            ASSERT(m_fence != nullptr, "Failed to get fence");

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(m_fence);
            debInfo.pObjectName = name;
            debInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;
            auto result = SetDebugUtilsObjectNameEXT(
                static_cast<const DeviceImplVulkan*>(device)->device(), &debInfo);
            ASSERT(result == VK_SUCCESS);

            auto res = GetSemaphoreCounterValueKHR(*m_device->m_device, m_fence, &m_fenceValue);
            ASSERT(res == VK_SUCCESS, "Failed to get initial fence value");

#ifndef RELEASE            
#ifdef _UNICODE
            static WCHAR resourceName[1024] = {};
            size_t numCharacters;
            if (name)
                mbstowcs_s(&numCharacters, resourceName, name, 1024);
#else
            wchar_t resourceName[1024] = {};
            size_t numCharacters;
            mbstowcs_s(&numCharacters, resourceName, name, 1024);
#endif

            //m_fence->SetName(resourceName);
            /*VkDebugMarkerObjectNameInfoEXT nameInfo = {};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
            // Type of the object to be named
            nameInfo.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT;
            // Handle of the object cast to unsigned 64-bit integer
            nameInfo.object = (uint64_t)m_fence;
            // Name to be displayed in the offline debugging application
            nameInfo.pObjectName = "Primary Command Buffer";
            pfnDebugMarkerSetObjectName(device, &nameInfo);*/
#endif

        }

        FenceImplVulkan::~FenceImplVulkan()
        {
            if (m_fence)
            {
                m_device->m_fencePool.release(m_fence);
            }
        }

        VkSemaphore& FenceImplVulkan::native()
        {
            return m_fence;
        }

        const VkSemaphore& FenceImplVulkan::native() const
        {
            return m_fence;
        }

        void FenceImplVulkan::increaseCPUValue()
        {
            m_fenceValue++;
        }

        FenceValue FenceImplVulkan::currentCPUValue() const
        {
            return m_fenceValue;
        }

        FenceValue FenceImplVulkan::currentGPUValue() const
        {
            FenceValue completedValue;
            auto res = GetSemaphoreCounterValueKHR(*m_device->m_device, m_fence, &completedValue);
            ASSERT(res == VK_SUCCESS, "Failed to get initial fence value");
            return completedValue;
        }

        void FenceImplVulkan::blockUntilSignaled()
        {
            const uint64_t waitValue = m_fenceValue;

            VkSemaphoreWaitInfo waitInfo;
            waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            waitInfo.pNext = NULL;
            waitInfo.flags = 0;
            waitInfo.semaphoreCount = 1;
            waitInfo.pSemaphores = &m_fence;
            waitInfo.pValues = &waitValue;

            WaitSemaphoresKHR(m_device->device(), &waitInfo, UINT64_MAX);
        }

        void FenceImplVulkan::blockUntilSignaled(engine::FenceValue value)
        {
            const uint64_t waitValue = value;

            VkSemaphoreWaitInfo waitInfo;
            waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            waitInfo.pNext = NULL;
            waitInfo.flags = 0;
            waitInfo.semaphoreCount = 1;
            waitInfo.pSemaphores = &m_fence;
            waitInfo.pValues = &waitValue;

            WaitSemaphoresKHR(m_device->device(), &waitInfo, UINT64_MAX);
        }

        void FenceImplVulkan::reset()
        {
            m_fenceValue = currentGPUValue();
        }

        bool FenceImplVulkan::signaled() const
        {
            return currentGPUValue() >= m_fenceValue;
        }

        bool FenceImplVulkan::signaled(FenceValue value) const
        {
            return currentGPUValue() >= value;
        }
    }
}
