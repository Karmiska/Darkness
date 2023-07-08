#include "engine/graphics/Sampler.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12Sampler.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanSampler.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalSampler.h"
#endif

using namespace tools;
using namespace engine::implementation;

namespace engine
{
    Sampler::Sampler()
        : m_impl{ nullptr }
    {};

    Sampler::Sampler(const Device& device, const SamplerDescription& desc, GraphicsApi api)
        : m_impl{}
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::make_unique<SamplerImplDX12>(device, desc);
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::make_unique<SamplerImplVulkan>(device, desc);
    }

    bool Sampler::valid() const
    {
        if (m_impl)
            return true;
        return false;
    };
}
