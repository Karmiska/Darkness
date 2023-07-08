#pragma once

#include "engine/graphics/SwapChainImplIf.h"
#include "containers/memory.h"
#include <utility>

namespace engine
{
    class Queue;
    class TextureRTV;
	class TextureSRV;
	class TextureRTVOwner;
    class Device;
    class Semaphore;
    class Fence;
    enum class GraphicsApi;
    class SwapChain
    {
    public:
        Semaphore& backBufferReadySemaphore();

        TextureRTV renderTarget(int index);
		TextureSRV renderTargetSRV(int index);
		TextureRTVOwner& renderTargetOwner(int index);
        unsigned int currentBackBufferIndex() const;
        void present();

        bool needRefresh() const;
        Size size() const;

        void resize(Device& device, Size size);

        bool vsync() const;
        void vsync(bool enabled);

        implementation::SwapChainImplIf* native() { return m_impl.get(); }
        const implementation::SwapChainImplIf* native() const { return m_impl.get(); }

        void recreate();
    private:
        friend class Device;
        SwapChain(
            const Device& device, 
            Queue& commandQueue, 
            GraphicsApi api,
            bool fullscreen = false, 
            bool vsync = true,
            SwapChain* oldSwapChain = nullptr);

        const Device& m_device;
        Queue& m_queue;
        engine::unique_ptr<implementation::SwapChainImplIf> m_impl;
        GraphicsApi m_api;
        bool m_fullscreen;
        bool m_vsync;
    };
}
