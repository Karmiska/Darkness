#include "engine/graphics/dx12/DX12RootParameter.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Conversions.h"

#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/RootParameter.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        RootParameterImplDX12::RootParameterImplDX12()
            : m_parameter{}
        {
        }

        void RootParameterImplDX12::binding(unsigned int /*index*/)
        {
            LOG("DX12 RootParameter binding function is not implemented");
        }

        unsigned int RootParameterImplDX12::binding() const
        {
            return 0;
        }

        void RootParameterImplDX12::visibility(ShaderVisibility /*visibility*/)
        {
            LOG("DX12 RootParameter visibility function is not implemented");
        }

        ShaderVisibility RootParameterImplDX12::visibility() const
        {
            return static_cast<ShaderVisibility>(ShaderVisibilityBits::Vertex);
        }

        D3D12_ROOT_PARAMETER1& RootParameterImplDX12::native()
        {
            return m_parameter;
        }

        void RootParameterImplDX12::initAsConstants(unsigned int reg, unsigned int num32BitValues, ShaderVisibility visibility)
        {
            m_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            m_parameter.ShaderVisibility = dxShaderVisibility(visibility);
            m_parameter.Constants.Num32BitValues = num32BitValues;
            m_parameter.Constants.ShaderRegister = reg;
            m_parameter.Constants.RegisterSpace = 0;
        }

        void RootParameterImplDX12::initAsCBV(unsigned int reg, ShaderVisibility visibility)
        {
            m_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            m_parameter.ShaderVisibility = dxShaderVisibility(visibility);
            m_parameter.Descriptor.ShaderRegister = reg;
            m_parameter.Descriptor.RegisterSpace = 0;
        }

        void RootParameterImplDX12::initAsSRV(unsigned int reg, ShaderVisibility visibility)
        {
            m_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
            m_parameter.ShaderVisibility = dxShaderVisibility(visibility);
            m_parameter.Descriptor.ShaderRegister = reg;
            m_parameter.Descriptor.RegisterSpace = 0;
        }

        void RootParameterImplDX12::initAsUAV(unsigned int reg, ShaderVisibility visibility)
        {
            m_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            m_parameter.ShaderVisibility = dxShaderVisibility(visibility);
            m_parameter.Descriptor.ShaderRegister = reg;
            m_parameter.Descriptor.RegisterSpace = 0;
        }

        void RootParameterImplDX12::initAsDescriptorRange(DescriptorRangeType type, unsigned int reg, unsigned int count, ShaderVisibility visibility)
        {
            initAsDescriptorTable(1, visibility);
            setTableRange(0, type, reg, count);
        }

        void RootParameterImplDX12::initAsDescriptorTable(unsigned int rangeCount, ShaderVisibility visibility)
        {
            m_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            m_parameter.ShaderVisibility = dxShaderVisibility(visibility);
            m_parameter.DescriptorTable.NumDescriptorRanges = rangeCount;
            m_parameter.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE1[rangeCount];
        }

        void RootParameterImplDX12::setTableRange(
            unsigned int rangeIndex, 
            DescriptorRangeType type, 
            unsigned int reg, 
            unsigned int count, 
            unsigned int space)
        {
            D3D12_DESCRIPTOR_RANGE1* range = const_cast<D3D12_DESCRIPTOR_RANGE1*>(m_parameter.DescriptorTable.pDescriptorRanges + rangeIndex);
            range->RangeType = dxRangeType(type);
            range->NumDescriptors = count;
            range->BaseShaderRegister = reg;
            range->RegisterSpace = space;
            range->Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
            range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
        }
    }
}
