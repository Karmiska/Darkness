#include "engine/graphics/Fence.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommonNoDep.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12Fence.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanFence.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalFence.h"
#endif


using namespace tools;
using namespace engine::implementation;

namespace engine
{
    Fence::Fence(const Device& device, const char* name, GraphicsApi api)
        : m_impl{}
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::make_unique<FenceImplDX12>(device.native(), name);
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::make_unique<FenceImplVulkan>(device.native(), name);
    }

    void Fence::increaseCPUValue()
    {
        m_impl->increaseCPUValue();
    }

    FenceValue Fence::currentCPUValue() const
    {
        return m_impl->currentCPUValue();
    }

    FenceValue Fence::currentGPUValue() const
    {
        return m_impl->currentGPUValue();
    }

    void Fence::blockUntilSignaled()
    {
        m_impl->blockUntilSignaled();
    }

    void Fence::blockUntilSignaled(FenceValue value)
    {
        m_impl->blockUntilSignaled(value);
    }

    bool Fence::signaled() const
    {
        return m_impl->signaled();
    }

    bool Fence::signaled(FenceValue value) const
    {
        return m_impl->signaled(value);
    }

    void Fence::reset()
    {
        m_impl->reset();
    }
}
