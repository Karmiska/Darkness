#include "engine/graphics/SwapChain.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/CommonNoDep.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12SwapChain.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanSwapChain.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalSwapChain.h"
#endif

namespace engine
{
    SwapChain::SwapChain(
        const Device& device, 
        Queue& queue, 
        GraphicsApi api,
        bool fullscreen, 
        bool vsync,
        SwapChain* oldSwapChain)
        : m_device{ device }
        , m_queue{ queue }
        , m_impl{}
        , m_api{ api }
        , m_fullscreen{ fullscreen}
        , m_vsync{ vsync }
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::make_unique<implementation::SwapChainImplDX12>(
                device, queue, fullscreen, vsync, oldSwapChain);
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::make_unique<implementation::SwapChainImplVulkan>(
                device, queue, fullscreen, vsync, oldSwapChain);
    }

    void SwapChain::recreate()
    {
        if (m_api == GraphicsApi::DX12)
            m_impl = engine::make_unique<implementation::SwapChainImplDX12>(
                m_device, m_queue, m_fullscreen, m_vsync, nullptr);
        else if (m_api == GraphicsApi::Vulkan)
            m_impl = engine::make_unique<implementation::SwapChainImplVulkan>(
                m_device, m_queue, m_fullscreen, m_vsync, this);
    }

    Semaphore& SwapChain::backBufferReadySemaphore()
    {
        return m_impl->backBufferReadySemaphore();
    }

    TextureRTV SwapChain::renderTarget(int index)
    {
        return m_impl->renderTarget(index);
    }

	TextureSRV SwapChain::renderTargetSRV(int index)
	{
		return m_impl->renderTargetSRV(index);
	}

	TextureRTVOwner& SwapChain::renderTargetOwner(int index)
	{
		return m_impl->renderTargetOwner(index);
	}

    unsigned int SwapChain::currentBackBufferIndex() const
    {
        return m_impl->currentBackBufferIndex();
    }

    void SwapChain::present()
    {
        m_impl->present();
    }

    bool SwapChain::needRefresh() const
    {
        return m_impl->needRefresh();
    }

    Size SwapChain::size() const
    {
        return m_impl->size();
    }

    void SwapChain::resize(Device& device, Size size)
    {
        if (size.width == 0u || size.height == 0)
            return;

        m_impl->resize(device, size);
    }

    bool SwapChain::vsync() const
    {
        return m_impl->vsync();
    };
    void SwapChain::vsync(bool enabled)
    {
        m_vsync = enabled;
        m_impl->vsync(enabled);
    };
}
