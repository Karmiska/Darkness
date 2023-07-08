#pragma once

#include "containers/memory.h"
#include <map>
struct MetalDevice{};
struct MetalSurface{};

namespace platform
{
    class Window;
}

namespace engine
{
    class Buffer;
    class DescriptorHandle;
    class PixelBuffer;
    class ColorBuffer;
    class DepthBuffer;
    class ShadowBuffer;
    class ByteAddressBuffer;
    class StructuredBuffer;
    class TypedBuffer;
    enum class Format;
    
    namespace implementation
    {
        class DeviceImpl
        {
        public:
            DeviceImpl(engine::shared_ptr<platform::Window> window);
            ~DeviceImpl();
            
            DeviceImpl(const DeviceImpl&) = delete;
            DeviceImpl(DeviceImpl&&) = delete;
            DeviceImpl& operator=(const DeviceImpl&) = delete;
            DeviceImpl& operator=(DeviceImpl&&) = delete;
            
            const platform::Window& window() const;
            int width() const;
            int height() const;
            
            void createRTV(Buffer& buffer, DescriptorHandle& rtvHandle);
            
            void waitForIdle();
            
        private:
            engine::shared_ptr<platform::Window> m_window;
            
        };
    }
}
