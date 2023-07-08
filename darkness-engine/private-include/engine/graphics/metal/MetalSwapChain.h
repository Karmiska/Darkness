#pragma once

struct MetalSwapChain{};

namespace engine
{
    class Device;
    class Queue;
    class Texture;
    class TextureRTV;
    class Semaphore;
    class Fence;
    class SwapChain;
    
    namespace implementation
    {
        class SwapChainImpl
        {
        public:
            SwapChainImpl(
                          const Device& device,
                          Queue& queue,
                          bool fullscreen = false,
                          bool vsync = true,
                          SwapChain* oldSwapChain = nullptr);
            ~SwapChainImpl();
            
            SwapChainImpl(const SwapChainImpl&) = delete;
            SwapChainImpl(SwapChainImpl&&) = delete;
            SwapChainImpl& operator=(const SwapChainImpl&) = delete;
            SwapChainImpl& operator=(SwapChainImpl&&) = delete;
            
            TextureRTV renderTarget(int index);
            size_t chainLength() const;
            unsigned int currentBackBufferIndex(const Semaphore& semaphore) const;
            unsigned int currentBackBufferIndex(const Fence& fence) const;
            void present();
            std::pair<int, int> size() const;
            
            
            uint32_t bufferCount() const;
            
            MetalSwapChain& native();
            const MetalSwapChain& native() const;
            
            bool needRefresh() const;
        private:
            const Device& m_device;
            MetalSwapChain m_swapChain;
            bool m_vsync;
            mutable bool m_needRefresh;
            engine::vector<TextureRTV> m_views;
        };
    }
}
