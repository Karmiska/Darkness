#pragma once

#include <cstdint>

namespace engine
{
    class Semaphore;
    class TextureRTV;
    class TextureSRV;
    class TextureRTVOwner;
    class Device;

    struct Size
    {
        std::uint32_t width;
        std::uint32_t height;
    };

    namespace implementation
    {
        class SwapChainImplIf
        {
        public:
            virtual ~SwapChainImplIf() {};

            virtual Semaphore& backBufferReadySemaphore() = 0;

            virtual TextureRTV renderTarget(int index) = 0;
            virtual TextureSRV renderTargetSRV(int index) = 0;
            virtual TextureRTVOwner& renderTargetOwner(int index) = 0;
            virtual unsigned int currentBackBufferIndex() const = 0;
            virtual void present() = 0;

            virtual bool needRefresh() = 0;
            virtual Size size() const = 0;

            virtual void resize(Device& device, Size size) = 0;

            virtual bool vsync() const = 0;
            virtual void vsync(bool enabled) = 0;
        };
    }
}
