#pragma once

#include "engine/graphics/SwapChainImplIf.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/SwapChain.h"
#include "engine/graphics/ResourceOwners.h"
#include "engine/graphics/Semaphore.h"
#include "tools/ComPtr.h"

#ifndef _DURANGO
struct IDXGISwapChain3;
using SwapChainType = IDXGISwapChain3;
#else
using SwapChainType = IDXGISwapChain1;
#endif

namespace engine
{
    class Device;
    class Queue;
    class Texture;
    class TextureRTV;
    class Fence;
    class SwapChain;

    namespace implementation
    {
        class BufferViewImpl;
        class SwapChainImplDX12 : public SwapChainImplIf
        {
        public:
            SwapChainImplDX12(
                const Device& device, 
                Queue& commandQueue, 
                bool fullscreen = false, 
                bool vsync = true,
                SwapChain* oldSwapChain = nullptr);

            Semaphore& backBufferReadySemaphore() override;

            TextureRTV renderTarget(int index) override;
			TextureSRV renderTargetSRV(int index) override;
			TextureRTVOwner& renderTargetOwner(int index) override;
            unsigned int currentBackBufferIndex() const;
            void present();

            bool needRefresh() override;
            Size size() const override;

            void resize(Device& device, Size size) override;

            bool vsync() const override
            {
                return m_vsync;
            };
            void vsync(bool enabled) override
            {
                m_vsync = enabled;
            };

			SwapChainType* native();
            const SwapChainType* native() const;
            size_t chainLength() const;

        private:
			tools::ComPtr<SwapChainType> m_swapChain;
			Semaphore m_backBufferReadySemaphore;
            Size m_size;
            
			void releaseSwapChainTextures(Device& device);
			void createSwapChainTextures(const Device& device);

            engine::vector<TextureOwner> m_swapChainTextures;
            engine::vector<TextureRTVOwner> m_swapChainTextureRTVs;
			engine::vector<TextureSRVOwner> m_swapChainTextureSRVs;
            int m_videoCardMemory;
            char m_videoCardDescription[128];
            bool m_vsync;
            bool m_needRefresh;
			DXGI_FORMAT m_format;
#ifdef _DURANGO
			uint32_t m_backBufferIndex;
#endif
        };
    }
}
