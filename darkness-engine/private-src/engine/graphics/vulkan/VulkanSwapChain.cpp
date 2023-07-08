#include "engine/graphics/Queue.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/SwapChain.h"

#include "engine/graphics/vulkan/VulkanQueue.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanResources.h"
#include "engine/graphics/vulkan/VulkanCommon.h"
#include "engine/graphics/vulkan/VulkanSemaphore.h"
#include "engine/graphics/vulkan/VulkanFence.h"

#include "engine/graphics/vulkan/VulkanSwapChain.h"

#include "platform/window/Window.h"
#ifdef _WIN32
#undef max
#include "platform/window/windows/WindowsWindow.h"
#endif

#include <cstdlib>
#include "engine/graphics/vulkan/VulkanHeaders.h"

#include "tools/Debug.h"

using namespace platform;

namespace engine
{
    namespace implementation
    {
        SwapChainImplVulkan::SwapChainImplVulkan(
            const Device& device, 
            Queue& /*queue*/, 
            bool /*fullscreen*/, 
            bool vsync,
            SwapChain* oldSwapChain)
            : m_device{ device }
            , m_vsync{ vsync }
            , m_swapChain{ vulkanPtr<VkSwapchainKHR>(static_cast<const DeviceImplVulkan*>(device.native())->device(), vkDestroySwapchainKHR) }
            , m_needRefresh{ false }
            , m_vsyncChange{ false }
            , m_backBufferIndex{ 0u }
            , m_currentSemaphore{ BackBufferCount-1 }
        {
            //std::lock_guard<std::mutex> lock(
            //    const_cast<DeviceImplVulkan*>(static_cast<const DeviceImplVulkan*>(device.native()))->window().resizeMutex());

            SwapChainDetails details = getDetails(
                static_cast<const DeviceImplVulkan*>(device.native())->physicalDevice(),
                static_cast<const DeviceImplVulkan*>(device.native())->surface());

            m_surfaceFormat = pickSurfaceFormat(details.formats);
            m_presentMode = pickPresentMode(details.presentModes);
            m_extent = pickExtent(device, details.capabilities);
            m_bufferCount = pickBufferCount(details.capabilities);

            m_createInfo = {};
            m_createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            m_createInfo.surface = static_cast<const DeviceImplVulkan*>(device.native())->surface();
            m_createInfo.minImageCount = m_bufferCount;
            m_createInfo.imageFormat = m_surfaceFormat.format;
            m_createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
            m_createInfo.imageExtent = m_extent;
            m_createInfo.imageArrayLayers = 1;
            m_createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            engine::vector<VulkanQueue> deviceQueues = static_cast<const DeviceImplVulkan*>(device.native())->deviceQueues();
            int graphicsFamily = -1;
            int presentFamily = -1;
            int queueIndex = 0;
            for (const auto& queue : deviceQueues)
            {
                if (queue.taken.size() > 0 && queue.flags & VK_QUEUE_GRAPHICS_BIT)
                    graphicsFamily = queueIndex;

                if (queue.taken.size() > 0 && queue.presentSupport)
                    presentFamily = queueIndex;

                if (graphicsFamily >= 0 && presentFamily >= 0)
                    break;

                ++queueIndex;
            }

            uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(graphicsFamily), static_cast<uint32_t>(presentFamily) };
            if (graphicsFamily != presentFamily) {
                m_createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                m_createInfo.queueFamilyIndexCount = 2;
                m_createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                m_createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                m_createInfo.queueFamilyIndexCount = 0; // Optional
                m_createInfo.pQueueFamilyIndices = nullptr; // Optional
            }
            m_createInfo.preTransform = details.capabilities.currentTransform;
            m_createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            m_createInfo.presentMode = m_presentMode;
            m_createInfo.clipped = VK_TRUE;

            if (oldSwapChain != nullptr)
            {
                m_createInfo.oldSwapchain = *static_cast<SwapChainImplVulkan*>(oldSwapChain->native())->m_swapChain;
            }
            else
            {
                m_createInfo.oldSwapchain = VK_NULL_HANDLE;
            }
            
            auto createResult = vkCreateSwapchainKHR(
                static_cast<const DeviceImplVulkan*>(device.native())->device(),
                &m_createInfo,
                nullptr,
                m_swapChain.get());
            ASSERT(createResult == VK_SUCCESS);

            uint32_t imageCount;
            vkGetSwapchainImagesKHR(static_cast<const DeviceImplVulkan*>(device.native())->device(), *m_swapChain, &imageCount, nullptr);

            engine::vector<VkImage> images(imageCount);
            vkGetSwapchainImagesKHR(static_cast<const DeviceImplVulkan*>(device.native())->device(), *m_swapChain, &imageCount, images.data());

            for (const auto& image : images)
            {
                const Device* devPtr = &m_device;
                m_views.emplace_back(
                    TextureOwner(static_pointer_cast<TextureImplIf>(engine::make_shared<TextureImplVulkan>(
                        engine::make_shared<VkImage>(image),
                        TextureDescription()
                        .format(fromVulkanFormat(m_surfaceFormat.format))
                        .usage(ResourceUsage::GpuRenderTargetReadWrite)
                        .width(m_extent.width)
                        .height(m_extent.height))),
                        [devPtr](engine::shared_ptr<TextureImplIf> im)
                        {
                            devPtr->m_returnedTextures.push(Device::ReturnedResourceTexture{ im, devPtr->m_submitFence.currentCPUValue() });
                            LOG("Cursious if this just works. GPU should be empty by now");
                            //m_returnedTextures.push(ReturnedResourceTexture{ im, m_submitFence.currentCPUValue() });
                        })
                );
            }

            for (auto&& tex : m_views)
            {
                m_viewsRTV.emplace_back(device.createTextureRTV(tex));
                m_viewsSRV.emplace_back(device.createTextureSRV(tex));
            }

            for (int i = 0; i < BackBufferCount; ++i)
            {
                m_semaphores.emplace_back(m_device.createSemaphore());
            }

            /*for (const auto& image : images)
            {
                //m_buffers.emplace_back(Buffer{ tools::make_impl<BufferImpl>(DeviceImplGet::impl(device), image, false) });
                m_buffers.emplace_back(device.
            }

            for (const auto& buffer : m_buffers)
            {
                m_views.emplace_back(
                    BufferView {
                        tools::make_unique_impl<BufferViewImpl>(
                            device, 
                            buffer, 
                            fromVulkanFormat(m_surfaceFormat.format),
                            static_cast<ImageAspectFlags>(ImageAspectFlagBits::Color))
                    }
                );
            }*/

            nextFrameBuffer();
        }

        SwapChainImplVulkan::SwapChainDetails SwapChainImplVulkan::getDetails(
            const VkPhysicalDevice& device,
            const VkSurfaceKHR& surface)
        {
            SwapChainDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
            if (formatCount > 0)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
            if (presentModeCount > 0)
            {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        VkSurfaceFormatKHR SwapChainImplVulkan::pickSurfaceFormat(const engine::vector<VkSurfaceFormatKHR>& formats) const
        {
            if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
                return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

            for (const auto& format : formats)
            {
                if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return format;
            }

            return formats[0];
        }

        VkPresentModeKHR SwapChainImplVulkan::pickPresentMode(const engine::vector<VkPresentModeKHR> modes) const
        {
            VkPresentModeKHR wantedModex = m_vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;
            for (const auto& mode : modes)
            {
                if (mode == wantedModex)
                {
                    return mode;
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D SwapChainImplVulkan::pickExtent(const Device& device, const VkSurfaceCapabilitiesKHR& /*capabilities*/) const
        {
            SwapChainDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                static_cast<const DeviceImplVulkan*>(device.native())->physicalDevice(),
                static_cast<const DeviceImplVulkan*>(device.native())->surface(), &details.capabilities);
            return details.capabilities.currentExtent;

            //if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
            //    return capabilities.currentExtent;
            //else
            //{
            //    VkExtent2D actualExtent{ 
            //        static_cast<uint32_t>(device.width()),
            //        static_cast<uint32_t>(device.height()) };
            //    actualExtent.width = capabilities.minImageExtent.width > actualExtent.width ? capabilities.minImageExtent.width : actualExtent.width;
            //    actualExtent.width = capabilities.maxImageExtent.width < actualExtent.width ? capabilities.maxImageExtent.width : actualExtent.width;
            //    actualExtent.height = capabilities.minImageExtent.height > actualExtent.height ? capabilities.minImageExtent.height : actualExtent.height;
            //    actualExtent.height = capabilities.maxImageExtent.height < actualExtent.height ? capabilities.maxImageExtent.height : actualExtent.height;
            //    return actualExtent;
            //}
        }

        uint32_t SwapChainImplVulkan::pickBufferCount(const VkSurfaceCapabilitiesKHR& capabilities) const
        {
            if (capabilities.maxImageCount == 0)
                return static_cast<uint32_t>(BackBufferCount);

            uint32_t imageCount{ BackBufferCount };
            imageCount = capabilities.minImageCount > imageCount ? capabilities.minImageCount : imageCount;
            imageCount = capabilities.maxImageCount < imageCount ? capabilities.maxImageCount : imageCount;
            return imageCount;
        }

        SwapChainImplVulkan::~SwapChainImplVulkan()
        {
            LOG("swapchain died");
            /*if (m_swapChain)
            {
                m_swapChain->Release();
                m_swapChain = NULL;
            }*/
        }

        const VkSurfaceFormatKHR& SwapChainImplVulkan::surfaceFormat() const
        {
            return m_surfaceFormat;
        }

        const VkPresentModeKHR& SwapChainImplVulkan::presentMode() const
        {
            return m_presentMode;
        }

        const VkExtent2D& SwapChainImplVulkan::extent() const
        {
            return m_extent;
        }

        Size SwapChainImplVulkan::size() const
        {
            Size size;
            size.width = m_extent.width;
            size.height = m_extent.height;
            return size;
        }

        uint32_t SwapChainImplVulkan::bufferCount() const
        {
            return m_bufferCount;
        }

        TextureRTV SwapChainImplVulkan::renderTarget(int index)
        {
            return m_viewsRTV[index].resource();
        }

        TextureSRV SwapChainImplVulkan::renderTargetSRV(int index)
        {
            return m_viewsSRV[index].resource();
        }

        TextureRTVOwner& SwapChainImplVulkan::renderTargetOwner(int index)
        {
            return m_viewsRTV[index];
        }

        std::size_t SwapChainImplVulkan::chainLength() const
        {
            return m_views.size();
        }

        Semaphore& SwapChainImplVulkan::backBufferReadySemaphore()
        {
            return m_semaphores[m_currentSemaphore];
        }

        unsigned int SwapChainImplVulkan::currentBackBufferIndex() const
        {
            /*uint32_t imageIndex;
            VkResult res = vkAcquireNextImageKHR(
                DeviceImplGet::impl(m_device).device(),
                *m_swapChain,
                std::numeric_limits<uint64_t>::max(),
                VK_NULL_HANDLE,
                FenceImplGet::impl(fence).native(),
                &imageIndex);

            if (res == VK_ERROR_OUT_OF_DATE_KHR)
            {
                m_needRefresh = true;
            }
            else
            {
                ASSERT(res == VK_SUCCESS);
            }

            return static_cast<unsigned int>(imageIndex);*/
            return m_backBufferIndex;
        }

        void SwapChainImplVulkan::resize(Device& /*device*/, Size /*size*/)
        {
            ASSERT(false, "resize not implemented");
#if 0
            m_views.clear();
            m_viewsRTV.clear();
            m_viewsSRV.clear();
            m_swapChain = nullptr;

            SwapChainDetails details = getDetails(
                static_cast<const DeviceImplVulkan*>(device.native())->physicalDevice(),
                static_cast<const DeviceImplVulkan*>(device.native())->surface());
            m_extent = pickExtent(device, details.capabilities);
            m_createInfo.imageExtent = m_extent;
            m_swapChain = vulkanPtr<VkSwapchainKHR>(static_cast<const DeviceImplVulkan*>(device.native())->device(), vkDestroySwapchainKHR);

            auto createResult = vkCreateSwapchainKHR(
                static_cast<const DeviceImplVulkan*>(device.native())->device(),
                &m_createInfo,
                nullptr,
                m_swapChain.get());
            ASSERT(createResult == VK_SUCCESS);

            uint32_t imageCount;
            vkGetSwapchainImagesKHR(static_cast<const DeviceImplVulkan*>(device.native())->device(), *m_swapChain, &imageCount, nullptr);

            engine::vector<VkImage> images(imageCount);
            vkGetSwapchainImagesKHR(static_cast<const DeviceImplVulkan*>(device.native())->device(), *m_swapChain, &imageCount, images.data());

            for (const auto& image : images)
            {
                m_views.emplace_back(
                    TextureOwner(static_pointer_cast<TextureImplIf>(engine::make_shared<TextureImplVulkan>(
                        engine::make_shared<VkImage>(image),
                        TextureDescription()
                        .format(fromVulkanFormat(m_surfaceFormat.format))
                        .usage(ResourceUsage::GpuRenderTargetReadWrite)
                        .width(m_extent.width)
                        .height(m_extent.height))),
                        [&](engine::shared_ptr<TextureImplIf> im)
                        {
                            LOG("Cursious if this just works. GPU should be empty by now");
                            //m_returnedTextures.push(ReturnedResourceTexture{ im, m_submitFence.currentCPUValue() });
                        })
                );
            }

            for (auto&& tex : m_views)
            {
                m_viewsRTV.emplace_back(device.createTextureRTV(tex));
                m_viewsSRV.emplace_back(device.createTextureSRV(tex));
            }

            for(int i = 0; i < BackBufferCount; ++i)
                m_semaphores[i].reset();
            
            nextFrameBuffer();
#endif
        }

        void SwapChainImplVulkan::nextFrameBuffer()
        {
            ++m_currentSemaphore;
            m_currentSemaphore %= BackBufferCount;

            uint32_t imageIndex;
            VkResult res = vkAcquireNextImageKHR(
                static_cast<const DeviceImplVulkan*>(m_device.native())->device(),
                *m_swapChain,
                std::numeric_limits<uint64_t>::max(),
                static_cast<SemaphoreImplVulkan*>(m_semaphores[m_currentSemaphore].native())->native(),
                VK_NULL_HANDLE,
                &imageIndex);

            if (res == VK_ERROR_OUT_OF_DATE_KHR)
            {
                m_needRefresh = true;
            }
            else
            {
                ASSERT(res == VK_SUCCESS);
            }

            m_backBufferIndex = static_cast<unsigned int>(imageIndex);
        }

        void SwapChainImplVulkan::present()
        {
            nextFrameBuffer();
            /*if (m_vsync)
                ASSERT(SUCCEEDED(m_swapChain->Present(1, 0)));
            else
                ASSERT(SUCCEEDED(m_swapChain->Present(0, 0)));*/
        }

        VkSwapchainKHR& SwapChainImplVulkan::native()
        {
            return *m_swapChain;
        }

        const VkSwapchainKHR& SwapChainImplVulkan::native() const
        {
            return *m_swapChain;
        }

        bool SwapChainImplVulkan::needRefresh()
        {
            bool res = m_needRefresh;
            m_needRefresh = false;
            return res;
        }
    }
}
