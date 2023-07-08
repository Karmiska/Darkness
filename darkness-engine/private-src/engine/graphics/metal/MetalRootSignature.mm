#include "engine/graphics/metal/MetalRootSignature.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "engine/graphics/metal/MetalRootParameter.h"
#include "engine/graphics/metal/MetalSampler.h"
#include "engine/graphics/metal/MetalDevice.h"
#include "engine/graphics/metal/MetalConversions.h"

#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/RootParameter.h"
#include "engine/graphics/RootSignature.h"
#include "engine/graphics/Device.h"

#include "tools/Debug.h"
#include "tools/ComPtr.h"

namespace engine
{
    namespace implementation
    {
        RootSignatureImpl::RootSignatureImpl(const Device& device, int rootParameterCount, int staticSamplerCount)
        : m_device{ device }
        , m_numInitializedStaticSamplers{ 0 }
        , m_samplers{ nullptr }
        , m_parameterCount{ 0 }
        , m_samplerCount{ 0 }
        {
        }
        
        void RootSignatureImpl::reset(int rootParameterCount, int staticSamplerCount)
        {
        }
        
        void RootSignatureImpl::initStaticSampler(int samplerNum, const SamplerDescription& description, ShaderVisibility visibility)
        {
        }
        
        void RootSignatureImpl::finalize(RootSignatureFlags /*flags*/)
        {
        }
        
        size_t RootSignatureImpl::rootParameterCount() const
        {
            return 0;
        }
        
        RootParameter& RootSignatureImpl::operator[](int index)
        {
            ASSERT(index >= 0 && index < m_parameterCount);
            return m_parameters[static_cast<size_t>(index)];
        }
        
        const RootParameter& RootSignatureImpl::operator[](int index) const
        {
            ASSERT(index >= 0 && index < m_parameterCount);
            return m_parameters[static_cast<size_t>(index)];
        }
    }
}
