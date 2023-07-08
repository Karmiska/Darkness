#include "engine/graphics/RootParameter.h"
#include "engine/graphics/SamplerDescription.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12RootParameter.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanRootParameter.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalRootParameter.h"
#endif

using namespace engine::implementation;

namespace engine
{
    /*RootParameter::RootParameter()
        : m_impl{}
    {
    }*/

    RootParameter::RootParameter(GraphicsApi api)
        : m_impl{}
    {
        if (api == GraphicsApi::DX12)
            m_impl = engine::make_unique<RootParameterImplDX12>();
        else if (api == GraphicsApi::Vulkan)
            m_impl = engine::make_unique<RootParameterImplVulkan>();
    }

    void RootParameter::binding(unsigned int index)
    {
        m_impl->binding(index);
    }

    unsigned int RootParameter::binding() const
    {
        return m_impl->binding();
    }

    void RootParameter::visibility(ShaderVisibility visibility)
    {
        m_impl->visibility(visibility);
    }

    ShaderVisibility RootParameter::visibility() const
    {
        return m_impl->visibility();
    }

    void RootParameter::initAsConstants(unsigned int reg, unsigned int num32BitValues, ShaderVisibility visibility)
    {
        m_impl->initAsConstants(reg, num32BitValues, visibility);
    }

    void RootParameter::initAsCBV(unsigned int reg, ShaderVisibility visibility)
    {
        m_impl->initAsCBV(reg, visibility);
    }

    void RootParameter::initAsSRV(unsigned int reg, ShaderVisibility visibility)
    {
        m_impl->initAsSRV(reg, visibility);
    }

    void RootParameter::initAsUAV(unsigned int reg, ShaderVisibility visibility)
    {
        m_impl->initAsUAV(reg, visibility);
    }

    void RootParameter::initAsDescriptorRange(DescriptorRangeType type, unsigned int reg, unsigned int count, ShaderVisibility visibility)
    {
        m_impl->initAsDescriptorRange(type, reg, count, visibility);
    }

    void RootParameter::initAsDescriptorTable(unsigned int rangeCount, ShaderVisibility visibility)
    {
        m_impl->initAsDescriptorTable(rangeCount, visibility);
    }

    void RootParameter::setTableRange(unsigned int rangeIndex, DescriptorRangeType type, unsigned int reg, unsigned int count, unsigned int space)
    {
        m_impl->setTableRange(rangeIndex, type, reg, count, space);
    }

}
