#include "engine/graphics/dx12/DX12RootSignature.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12RootParameter.h"
#include "engine/graphics/dx12/DX12Sampler.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12Conversions.h"

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
        RootSignatureImplDX12::RootSignatureImplDX12(const Device& device, int rootParameterCount, int staticSamplerCount)
            : m_device{ device }
            , m_signature{ nullptr }
            , m_numInitializedStaticSamplers{ 0 }
            , m_parameters{}
            , m_samplers{ nullptr }
            , m_parameterCount{ 0 }
            , m_samplerCount{ 0 }
        {
            reset(rootParameterCount, staticSamplerCount);
        }

        RootSignatureImplDX12::~RootSignatureImplDX12()
        {
            if (m_signature)
            {
                m_signature->Release();
                m_signature = nullptr;
            }
        }

        void RootSignatureImplDX12::reset(int rootParameterCount, int staticSamplerCount)
        {
            m_parameterCount = rootParameterCount;
            m_samplerCount = staticSamplerCount;

            m_parameters.clear();
            for (int i = 0; i < rootParameterCount; ++i)
                m_parameters.emplace_back(RootParameter(GraphicsApi::DX12));

            if (staticSamplerCount > 0)
                m_samplers.reset(new StaticSamplerDescription[static_cast<size_t>(staticSamplerCount)]);
            else
                m_samplers = nullptr;
        }

        void RootSignatureImplDX12::initStaticSampler(int samplerNum, const SamplerDescription& description, ShaderVisibility visibility)
        {
            StaticSamplerDescription& staticSamplerDesc = m_samplers[m_numInitializedStaticSamplers++];
            staticSamplerDesc.desc.filter = description.desc.filter;
            staticSamplerDesc.desc.addressU = description.desc.addressU;
            staticSamplerDesc.desc.addressV = description.desc.addressV;
            staticSamplerDesc.desc.addressW = description.desc.addressW;
            staticSamplerDesc.desc.mipLODBias = description.desc.mipLODBias;
            staticSamplerDesc.desc.maxAnisotrophy = description.desc.maxAnisotrophy;
            staticSamplerDesc.desc.comparisonFunc = description.desc.comparisonFunc;
            staticSamplerDesc.desc.borderColor = StaticBorderColor::OpaqueWhite;
            staticSamplerDesc.desc.minLOD = description.desc.minLOD;
            staticSamplerDesc.desc.maxLOD = description.desc.maxLOD;
            staticSamplerDesc.desc.shaderRegister = static_cast<uint32_t>(samplerNum);
            staticSamplerDesc.desc.registerSpace = 0;
            staticSamplerDesc.desc.shaderVisibility = visibility;

            if (staticSamplerDesc.desc.addressU == TextureAddressMode::Border ||
                staticSamplerDesc.desc.addressV == TextureAddressMode::Border ||
                staticSamplerDesc.desc.addressW == TextureAddressMode::Border)
            {
                if (description.desc.borderColor[3] == 1.0f)
                {
                    if (description.desc.borderColor[0] == 1.0f)
                        staticSamplerDesc.desc.borderColor = StaticBorderColor::OpaqueWhite;
                    else
                        staticSamplerDesc.desc.borderColor = StaticBorderColor::OpaqueBlack;
                }
                else
                    staticSamplerDesc.desc.borderColor = StaticBorderColor::TransparentBlack;
            }
        }

		engine::string dx12RootSignatureFlagsToString(D3D12_ROOT_SIGNATURE_FLAGS flags)
		{
			engine::string res = "";
			bool first = true;
			if (flags & D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT) res += "D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT";
			if (flags & D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS) { if (first) { res += "D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS"; first = false; } else { res += " | D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS"; } }
			if (flags & D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS) { if (first) { res += "D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS"; first = false; } else { res += " | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS"; } }
			if (flags & D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS) { if (first) { res += "D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS"; first = false; } else { res += " | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS"; } }
			if (flags & D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS) { if (first) { res += "D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS"; first = false; } else { res += " | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS"; } }
			if (flags & D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS) { if (first) { res += "D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS"; first = false; } else { res += " | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS"; } }
			if (flags & D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT) { if (first) { res += "D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT"; first = false; } else { res += " | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT"; } }

			if (res == "") res = "D3D12_ROOT_SIGNATURE_FLAG_NONE";
			return res;
		}

		engine::string dx12RootParameterTypeToString(D3D12_ROOT_PARAMETER_TYPE type)
		{
			switch (type)
			{
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: return "D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE";
			case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS: return "D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS";
			case D3D12_ROOT_PARAMETER_TYPE_CBV: return "D3D12_ROOT_PARAMETER_TYPE_CBV";
			case D3D12_ROOT_PARAMETER_TYPE_SRV: return "D3D12_ROOT_PARAMETER_TYPE_SRV";
			case D3D12_ROOT_PARAMETER_TYPE_UAV: return "D3D12_ROOT_PARAMETER_TYPE_UAV";
			}
			return "Unknown root parameter type";
		}

		engine::string dx12ShaderVisibilityToString(D3D12_SHADER_VISIBILITY type)
		{
			switch (type)
			{
				case D3D12_SHADER_VISIBILITY_ALL: return "D3D12_SHADER_VISIBILITY_ALL";
				case D3D12_SHADER_VISIBILITY_VERTEX: return "D3D12_SHADER_VISIBILITY_VERTEX";
				case D3D12_SHADER_VISIBILITY_HULL: return "D3D12_SHADER_VISIBILITY_HULL";
				case D3D12_SHADER_VISIBILITY_DOMAIN: return "D3D12_SHADER_VISIBILITY_DOMAIN";
				case D3D12_SHADER_VISIBILITY_GEOMETRY: return "D3D12_SHADER_VISIBILITY_GEOMETRY";
				case D3D12_SHADER_VISIBILITY_PIXEL: return "D3D12_SHADER_VISIBILITY_PIXEL";
#ifdef DXR_BUILD
                case D3D12_SHADER_VISIBILITY_AMPLIFICATION: return "D3D12_SHADER_VISIBILITY_AMPLIFICATION";
                case D3D12_SHADER_VISIBILITY_MESH: return "D3D12_SHADER_VISIBILITY_MESH";
#endif
			}
			return "Unknown shader visibility";
		}

		engine::string dx12RootDescriptorFlagsToString(D3D12_ROOT_DESCRIPTOR_FLAGS type)
		{
			switch (type)
			{
			case D3D12_ROOT_DESCRIPTOR_FLAG_NONE: return "D3D12_ROOT_DESCRIPTOR_FLAG_NONE";
			case D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE: return "D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE";
			case D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE: return "D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE";
			case D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC: return "D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC";
			}
			return "Unknown descriptor flags";
		}
		
		engine::string dx12DescriptorRangeTypeToString(D3D12_DESCRIPTOR_RANGE_TYPE type)
		{
			switch (type)
			{
			case D3D12_DESCRIPTOR_RANGE_TYPE_SRV: return "D3D12_DESCRIPTOR_RANGE_TYPE_SRV";
			case D3D12_DESCRIPTOR_RANGE_TYPE_UAV: return "D3D12_DESCRIPTOR_RANGE_TYPE_UAV";
			case D3D12_DESCRIPTOR_RANGE_TYPE_CBV: return "D3D12_DESCRIPTOR_RANGE_TYPE_CBV";
			case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER: return "D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER";
			}
			return "Unknown descriptor range type";
		}

		engine::string dx12DescriptorRangeFlagsToString(D3D12_DESCRIPTOR_RANGE_FLAGS flags)
		{
			engine::string res = "";
			bool first = true;
			if (flags & D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE) res += "D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE";
			if (flags & D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE) { if (first) { res += "D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE"; first = false; } else { res += " | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE"; } }
			if (flags & D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE) { if (first) { res += "D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE"; first = false; } else { res += " | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE"; } }
			if (flags & D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC) { if (first) { res += "D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC"; first = false; } else { res += " | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC"; } }
#ifndef _DURANGO
			if (flags & D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS) { if (first) { res += "D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS"; first = false; } else { res += " | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS"; } }
#endif

			if (res == "") res = "D3D12_DESCRIPTOR_RANGE_FLAG_NONE";
			return res;
		}

		void debugPrintDescriptorRange(UINT index, D3D12_DESCRIPTOR_RANGE1 range)
		{
			LOG("    Range[%u], RangeType: %s, NumDescriptors: %u, BaseShaderRegister: %u, RegisterSpace: %u, Flags: %s, OffsetInDescriptorsFromTableStart: %u", 
				index, 
				dx12DescriptorRangeTypeToString(range.RangeType).c_str(),
				range.NumDescriptors,
				range.BaseShaderRegister,
				range.RegisterSpace,
				dx12DescriptorRangeFlagsToString(range.Flags).c_str(),
				range.OffsetInDescriptorsFromTableStart);
		}

		void debugPrintRootSignature(engine::vector<D3D12_ROOT_PARAMETER1>& parameters, D3D12_ROOT_SIGNATURE_DESC1& rootDesc)
		{
			LOG("==================================");
			LOG("Root Signature");
			LOG("NumParameters: %u, NumStaticSamplers: %u, Flags: %s", rootDesc.NumParameters, rootDesc.NumStaticSamplers, dx12RootSignatureFlagsToString(rootDesc.Flags).c_str());
			for (int i = 0; i < parameters.size(); ++i)
			{
				if (parameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
				{
					LOG("Paramater[%i]. ParameterType: %s, Visibility: %s, NumDescriptorRanges: %u", i,
						dx12RootParameterTypeToString(parameters[i].ParameterType).c_str(),
						dx12ShaderVisibilityToString(parameters[i].ShaderVisibility).c_str(),
						parameters[i].DescriptorTable.NumDescriptorRanges);
					for (UINT a = 0; a < parameters[i].DescriptorTable.NumDescriptorRanges; ++a)
					{
						debugPrintDescriptorRange(a, parameters[i].DescriptorTable.pDescriptorRanges[a]);
					}
				}
				else if (parameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
				{
					LOG("Paramater[%i]. ParameterType: %s, Visibility: %s, ShaderRegister: %u, RegisterSpace: %u, Num32BitValues: %u", i,
						dx12RootParameterTypeToString(parameters[i].ParameterType).c_str(),
						dx12ShaderVisibilityToString(parameters[i].ShaderVisibility).c_str(),
						parameters[i].Constants.ShaderRegister,
						parameters[i].Constants.RegisterSpace,
						parameters[i].Constants.Num32BitValues);
				}
				else if ((parameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV) ||
						 (parameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV) ||
						 (parameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV))
				{
					LOG("Paramater[%i]. ParameterType: %s, Visibility: %s, ShaderRegister: %u, RegisterSpace: %u, Flags: %s", i,
						dx12RootParameterTypeToString(parameters[i].ParameterType).c_str(),
						dx12ShaderVisibilityToString(parameters[i].ShaderVisibility).c_str(),
						parameters[i].Descriptor.ShaderRegister,
						parameters[i].Descriptor.RegisterSpace,
						dx12RootDescriptorFlagsToString(parameters[i].Descriptor.Flags).c_str());
				}
				
			}
			LOG("==================================");
		}

        void RootSignatureImplDX12::finalize(RootSignatureFlags flags)
        {
            if (m_signature)
                return;

            ASSERT(static_cast<int>(m_numInitializedStaticSamplers) == m_samplerCount);

            engine::vector<D3D12_ROOT_PARAMETER1> parameters;
            if (m_parameterCount > 0)
            {
                parameters.resize(m_parameterCount);
                for (size_t i = 0; i < static_cast<size_t>(m_parameterCount); ++i)
                {
                    parameters[i] = static_cast<RootParameterImplDX12*>(m_parameters[i].native())->native();
                }
            }

            D3D12_STATIC_SAMPLER_DESC* samplers = nullptr;
            if (m_samplerCount > 0)
            {
                samplers = new D3D12_STATIC_SAMPLER_DESC[static_cast<size_t>(m_samplerCount)];
                for (size_t i = 0; i < static_cast<size_t>(m_samplerCount); ++i)
                {
                    samplers[i] = dxStaticSamplerDesc(m_samplers[i]);
                }
            }

            D3D12_ROOT_SIGNATURE_DESC1 rootDesc;
            rootDesc.NumParameters = static_cast<UINT>(m_parameterCount);
            rootDesc.pParameters = (const D3D12_ROOT_PARAMETER1*)parameters.data();
            rootDesc.NumStaticSamplers = static_cast<UINT>(m_samplerCount);
            rootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)samplers;
            rootDesc.Flags = dxSignatureFlags(flags) | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			debugPrintRootSignature(parameters, rootDesc);

            tools::ComPtr<ID3DBlob> pOutBlob, pErrorBlob;

            D3D12_VERSIONED_ROOT_SIGNATURE_DESC vdesc;
            vdesc.Desc_1_1 = rootDesc;
            vdesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;

            auto serializeRes = D3D12SerializeVersionedRootSignature(&vdesc, pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
            ASSERT(SUCCEEDED(serializeRes));

            auto createRootSignatureRes = static_cast<const DeviceImplDX12*>(m_device.native())->device()->CreateRootSignature(1, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(),
                DARKNESS_IID_PPV_ARGS(&m_signature));
            ASSERT(SUCCEEDED(createRootSignatureRes));
        }

        void RootSignatureImplDX12::enableNullDescriptors(bool /*texture*/, bool /*writeable*/)
        {
            ASSERT(false, "RootSignatureImplDX12::enableNullDescriptors not implemented");
        }

        size_t RootSignatureImplDX12::rootParameterCount() const
        {
            return 0;
        }

        RootParameter& RootSignatureImplDX12::operator[](size_t index)
        {
            ASSERT(index >= 0 && index < static_cast<size_t>(m_parameterCount));
            return m_parameters[static_cast<size_t>(index)];
        }

        const RootParameter& RootSignatureImplDX12::operator[](size_t index) const
        {
            ASSERT(index >= 0 && index < static_cast<size_t>(m_parameterCount));
            return m_parameters[static_cast<size_t>(index)];
        }

        ID3D12RootSignature* RootSignatureImplDX12::native()
        {
            return m_signature;
        }

        ID3D12RootSignature* RootSignatureImplDX12::native() const
        {
            return m_signature;
        }
    }
}
