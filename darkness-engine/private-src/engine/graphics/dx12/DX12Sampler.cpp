#include "engine/graphics/dx12/DX12Sampler.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12DescriptorHeap.h"
#include "engine/graphics/dx12/DX12Conversions.h"

#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Device.h"
#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        SamplerImplDX12::SamplerImplDX12(
            const Device& device, 
            const SamplerDescription& desc)
            : m_descriptorHandle{ static_cast<const DeviceImplDX12*>(device.native())->heaps().sampler->getDescriptor() }
        {
            D3D12_SAMPLER_DESC samplerDesc;
            samplerDesc.Filter = dxFilter(desc.desc.filter);
            samplerDesc.AddressU = dxTextureAddressMode(desc.desc.addressU);
            samplerDesc.AddressV = dxTextureAddressMode(desc.desc.addressV);
            samplerDesc.AddressW = dxTextureAddressMode(desc.desc.addressW);
            samplerDesc.MipLODBias = desc.desc.mipLODBias;
            samplerDesc.MaxAnisotropy = 1;// desc.desc.maxAnisotrophy;
            samplerDesc.ComparisonFunc = dxComparisonFunc(desc.desc.comparisonFunc);
            samplerDesc.BorderColor[0] = desc.desc.borderColor[0];
            samplerDesc.BorderColor[1] = desc.desc.borderColor[1];
            samplerDesc.BorderColor[2] = desc.desc.borderColor[2];
            samplerDesc.BorderColor[3] = desc.desc.borderColor[3];
            samplerDesc.MinLOD = desc.desc.minLOD;
            samplerDesc.MaxLOD = desc.desc.maxLOD;

            static_cast<const DeviceImplDX12*>(device.native())->device()->CreateSampler(
                &samplerDesc,
                m_descriptorHandle.cpuHandle());
        }

        D3D12_CPU_DESCRIPTOR_HANDLE& SamplerImplDX12::native()
        {
            return m_descriptorHandle.cpuHandle();
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& SamplerImplDX12::native() const
        {
            return m_descriptorHandle.cpuHandle();
        }
    }
}
