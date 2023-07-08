#pragma once

#include "engine/graphics/SwapChainImplIf.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/SwapChain.h"
#include "containers/vector.h"

namespace engine
{
    class Device;
    class Queue;
    class Texture;
    class TextureOwner;
    class TextureRTV;
    class TextureSRV;
    class TextureRTVOwner;
    class TextureSRVOwner;
    class Semaphore;
    class Fence;
    class SwapChain;
    struct Size;

    namespace implementation
    {
        class BufferViewImpl;
        class SwapChainImplVulkan : public SwapChainImplIf
        {
        public:
            SwapChainImplVulkan(
                const Device& device, 
                Queue& queue, 
                bool fullscreen = false, 
                bool vsync = true,
                SwapChain* oldSwapChain = nullptr);
            ~SwapChainImplVulkan();

            SwapChainImplVulkan(const SwapChainImplVulkan&) = delete;
            SwapChainImplVulkan(SwapChainImplVulkan&&) = delete;
            SwapChainImplVulkan& operator=(const SwapChainImplVulkan&) = delete;
            SwapChainImplVulkan& operator=(SwapChainImplVulkan&&) = delete;

            TextureRTV renderTarget(int index) override;
            TextureSRV renderTargetSRV(int index) override;
            TextureRTVOwner& renderTargetOwner(int index) override;
            std::size_t chainLength() const;
            unsigned int currentBackBufferIndex() const override;
            void present() override;
            Size size() const;

            Semaphore& backBufferReadySemaphore() override;

            struct SwapChainDetails
            {
                VkSurfaceCapabilitiesKHR capabilities;
                engine::vector<VkSurfaceFormatKHR> formats;
                engine::vector<VkPresentModeKHR> presentModes;
            };
            static SwapChainDetails getDetails(
                const VkPhysicalDevice& device,
                const VkSurfaceKHR& surface);

            const VkSurfaceFormatKHR& surfaceFormat() const;
            const VkPresentModeKHR& presentMode() const;
            const VkExtent2D& extent() const;
            uint32_t bufferCount() const;

            VkSwapchainKHR& native();
            const VkSwapchainKHR& native() const;

            bool needRefresh() override;

            void resize(Device& device, Size size);

            bool vsync() const
            {
                return m_vsync;
            };
            void vsync(bool enabled)
            {
                if (m_vsync != enabled)
                    m_vsyncChange = true;
                m_vsync = enabled;
            };

            bool vsyncChange() const { return m_vsyncChange; }
            void vsyncChange(bool value) { m_vsyncChange = value; }
        private:
            const Device& m_device;
            engine::shared_ptr<VkSwapchainKHR> m_swapChain;
            int m_videoCardMemory;
            char m_videoCardDescription[128];
            bool m_vsync;
            mutable bool m_needRefresh;
            bool m_vsyncChange;

            VkSurfaceFormatKHR m_surfaceFormat;
            VkPresentModeKHR m_presentMode;
            VkExtent2D m_extent;
            uint32_t m_bufferCount;
            uint32_t m_backBufferIndex;

            engine::vector<TextureOwner> m_views;
            engine::vector<TextureRTVOwner> m_viewsRTV;
            engine::vector<TextureSRVOwner> m_viewsSRV;

            engine::vector<Semaphore> m_semaphores;
            int m_currentSemaphore;

            VkSwapchainCreateInfoKHR m_createInfo;

            VkSurfaceFormatKHR pickSurfaceFormat(const engine::vector<VkSurfaceFormatKHR>& formats) const;
            VkPresentModeKHR pickPresentMode(const engine::vector<VkPresentModeKHR> modes) const;
            VkExtent2D pickExtent(const Device& device, const VkSurfaceCapabilitiesKHR& capabilities) const;
            uint32_t pickBufferCount(const VkSurfaceCapabilitiesKHR& capabilities) const;

            void nextFrameBuffer();
        };
    }
}
