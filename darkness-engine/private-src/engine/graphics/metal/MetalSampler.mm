#include "engine/graphics/metal/MetalSampler.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/metal/MetalDevice.h"
#include "engine/graphics/DescriptorHandle.h"
#include "engine/graphics/metal/MetalDescriptorHandle.h"
#include "engine/graphics/metal/MetalConversions.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        SamplerImpl::SamplerImpl(
            const Device& device, 
            const DescriptorHandle& handle, 
            const SamplerDescription& desc)
            : m_device{ device }
        {
            /*D3D12_SAMPLER_DESC samplerDesc;
            samplerDesc.Filter = dxFilter(desc.desc.filter);
            samplerDesc.AddressU = dxTextureAddressMode(desc.desc.addressU);
            samplerDesc.AddressV = dxTextureAddressMode(desc.desc.addressV);
            samplerDesc.AddressW = dxTextureAddressMode(desc.desc.addressW);
            samplerDesc.MipLODBias = desc.desc.mipLODBias;
            samplerDesc.MaxAnisotropy = desc.desc.maxAnisotrophy;
            samplerDesc.ComparisonFunc = dxComparisonFunc(desc.desc.comparisonFunc);
            samplerDesc.BorderColor[0] = desc.desc.borderColor[0];
            samplerDesc.BorderColor[1] = desc.desc.borderColor[1];
            samplerDesc.BorderColor[2] = desc.desc.borderColor[2];
            samplerDesc.BorderColor[3] = desc.desc.borderColor[3];
            samplerDesc.MinLOD = desc.desc.minLOD;
            samplerDesc.MaxLOD = desc.desc.maxLOD;*/

            //DeviceImplGet::impl(device).device()->CreateSampler(&samplerDesc, DescriptorHandleImplGet::impl(handle).cpuHandle());
        }
        
        SamplerImpl::SamplerImpl(
            const Device& device,
            const SamplerDescription& /*desc*/)
            : m_device{ device }
        {
        }
        
        SamplerImpl::~SamplerImpl()
        {
        }
    }
}
