#include "engine/graphics/Semaphore.h"
#include "engine/graphics/CommonNoDep.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12Semaphore.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanSemaphore.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalSemaphore.h"
#endif


using namespace tools;
using namespace engine::implementation;

namespace engine
{
    Semaphore::Semaphore(const Device& device, GraphicsApi api)
        : m_impl{}
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::make_unique<SemaphoreImplDX12>(device);
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::make_unique<SemaphoreImplVulkan>(device);
    }

    void Semaphore::reset()
    {
        m_impl->reset();
    }

    engine::FenceValue Semaphore::currentGPUValue() const
    {
        ASSERT(false, "shouldn't be used");
        return 0;// m_impl->currentGPUValue();
    }
}
