#include "engine/graphics/metal/MetalDevice.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "engine/graphics/metal/MetalBuffer.h"
#include "engine/graphics/metal/MetalDescriptorHandle.h"

#include "engine/graphics/Buffer.h"
#include "engine/graphics/DescriptorHandle.h"
#include "platform/window/OsxWindow.h"
#include "tools/Debug.h"

using namespace platform::implementation;

namespace engine
{
    namespace implementation
    {
        DeviceImpl::DeviceImpl(std::shared_ptr<platform::Window> window)
        : m_window{ window }
        {}
        
        DeviceImpl::~DeviceImpl()
        {}
        
        const platform::Window& DeviceImpl::window() const
        {
            return *m_window;
        }
        
        int DeviceImpl::width() const
        {
            return 0;
        }
        
        int DeviceImpl::height() const
        {
            return 0;
        }
        
        void DeviceImpl::createRTV(Buffer& buffer, DescriptorHandle& rtvHandle)
        {}
        
        void DeviceImpl::waitForIdle()
        {}
    }
}
