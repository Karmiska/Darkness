#include "engine/graphics/RootSignature.h"
#include "engine/graphics/Device.h"
#include "tools/Debug.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12RootSignature.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanRootSignature.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalRootSignature.h"
#endif

using namespace tools;
using namespace engine::implementation;

namespace engine
{
    RootSignature::RootSignature(const Device& device, GraphicsApi api, int rootParameterCount, int staticSamplerCount)
        : m_impl{}
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::make_unique<RootSignatureImplDX12>(device, rootParameterCount, staticSamplerCount);
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::make_unique<RootSignatureImplVulkan>(device, rootParameterCount, staticSamplerCount);
    }

    void RootSignature::reset(int rootParameterCount, int staticSamplerCount)
    {
        m_impl->reset(rootParameterCount, staticSamplerCount);
    }

    void RootSignature::initStaticSampler(
        int samplerNum,
        const SamplerDescription& description,
        ShaderVisibility visibility)
    {
        m_impl->initStaticSampler(samplerNum, description, visibility);
    }

    void RootSignature::finalize(RootSignatureFlags flags)
    {
        m_impl->finalize(flags);
    }

    void RootSignature::enableNullDescriptors(bool texture, bool writeable)
    {
        m_impl->enableNullDescriptors(texture, writeable);
    }

    size_t RootSignature::rootParameterCount() const
    {
        return m_impl->rootParameterCount();
    }

    RootParameter& RootSignature::operator[](size_t index)
    {
        return (*m_impl)[index];
    }

    const RootParameter& RootSignature::operator[](size_t index) const
    {
        return (*m_impl)[index];
    }
}
