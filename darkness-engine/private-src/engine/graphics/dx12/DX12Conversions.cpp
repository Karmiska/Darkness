#include "engine/graphics/dx12/DX12Conversions.h"

#include "engine/graphics/RootSignature.h"
#include "engine/graphics/RootParameter.h"
#include "engine/graphics/SamplerDescription.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/Format.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/Pipeline.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        D3D12_SHADER_VISIBILITY dxShaderVisibility(ShaderVisibility visibility)
        {
            if (((visibility & static_cast<uint32_t>(ShaderVisibilityBits::All)) > 0) ||
                ((visibility & static_cast<uint32_t>(ShaderVisibilityBits::AllGraphics)) > 0))return D3D12_SHADER_VISIBILITY_ALL;
            if ((visibility & static_cast<uint32_t>(ShaderVisibilityBits::Vertex)) > 0) return D3D12_SHADER_VISIBILITY_VERTEX;
            if ((visibility & static_cast<uint32_t>(ShaderVisibilityBits::Hull)) > 0) return D3D12_SHADER_VISIBILITY_HULL;
            if ((visibility & static_cast<uint32_t>(ShaderVisibilityBits::Domain)) > 0) return D3D12_SHADER_VISIBILITY_DOMAIN;
            if ((visibility & static_cast<uint32_t>(ShaderVisibilityBits::Geometry)) > 0) return D3D12_SHADER_VISIBILITY_GEOMETRY;
            if ((visibility & static_cast<uint32_t>(ShaderVisibilityBits::Pixel)) > 0) return D3D12_SHADER_VISIBILITY_PIXEL;
#ifdef DXR_BUILD
            if ((visibility & static_cast<uint32_t>(ShaderVisibilityBits::Amplification)) > 0) return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
            if ((visibility & static_cast<uint32_t>(ShaderVisibilityBits::Mesh)) > 0) return D3D12_SHADER_VISIBILITY_MESH;
#endif
            return D3D12_SHADER_VISIBILITY_ALL;
        }

        D3D12_DESCRIPTOR_RANGE_TYPE dxRangeType(DescriptorRangeType type)
        {
            switch (type)
            {
                case DescriptorRangeType::SRV: return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                case DescriptorRangeType::UAV: return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
                case DescriptorRangeType::CBV: return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                case DescriptorRangeType::Sampler: return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
            }
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        }

        D3D12_FILTER dxFilter(const Filter& filter)
        {
            switch (filter)
            {
            case Filter::Point: return D3D12_FILTER_MIN_MAG_MIP_POINT;
            case Filter::Bilinear: return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            case Filter::Trilinear: return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            case Filter::Anisotropic: return D3D12_FILTER_ANISOTROPIC;
            case Filter::Comparison: return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
            }
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
        }

        D3D12_TEXTURE_ADDRESS_MODE dxTextureAddressMode(TextureAddressMode mode)
        {
            switch (mode)
            {
            case TextureAddressMode::Wrap: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            case TextureAddressMode::Mirror: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
            case TextureAddressMode::Clamp: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
            case TextureAddressMode::Border: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
            case TextureAddressMode::MirrorOnce: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
            }
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        }

        D3D12_COMPARISON_FUNC dxComparisonFunc(ComparisonFunction comp)
        {
            switch (comp)
            {
            case ComparisonFunction::Never: return D3D12_COMPARISON_FUNC_NEVER;
            case ComparisonFunction::Less: return D3D12_COMPARISON_FUNC_LESS;
            case ComparisonFunction::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
            case ComparisonFunction::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
            case ComparisonFunction::Greater: return D3D12_COMPARISON_FUNC_GREATER;
            case ComparisonFunction::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
            case ComparisonFunction::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
            case ComparisonFunction::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
            }
            return D3D12_COMPARISON_FUNC_NEVER;
        }

        D3D12_STATIC_BORDER_COLOR dxStaticBorderColor(StaticBorderColor color)
        {
            switch (color)
            {
            case StaticBorderColor::TransparentBlack: return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
            case StaticBorderColor::OpaqueBlack: return D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
            case StaticBorderColor::OpaqueWhite: return D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
            }
            return D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        }

        D3D12_STATIC_SAMPLER_DESC dxStaticSamplerDesc(const StaticSamplerDescription& desc)
        {
            D3D12_STATIC_SAMPLER_DESC res;
            res.Filter = dxFilter(desc.desc.filter);
            res.AddressU = dxTextureAddressMode(desc.desc.addressU);
            res.AddressV = dxTextureAddressMode(desc.desc.addressV);
            res.AddressW = dxTextureAddressMode(desc.desc.addressW);
            res.MipLODBias = desc.desc.mipLODBias;
            res.MaxAnisotropy = desc.desc.maxAnisotrophy;
            res.ComparisonFunc = dxComparisonFunc(desc.desc.comparisonFunc);
            res.BorderColor = dxStaticBorderColor(desc.desc.borderColor);
            res.MinLOD = desc.desc.minLOD;
            res.MaxLOD = desc.desc.maxLOD;
            res.ShaderRegister = desc.desc.shaderRegister;
            res.RegisterSpace = desc.desc.registerSpace;
            res.ShaderVisibility = dxShaderVisibility(desc.desc.shaderVisibility);
            return res;
        }

        D3D12_ROOT_SIGNATURE_FLAGS dxSignatureFlags(RootSignatureFlags flags)
        {
            switch (flags)
            {
            case RootSignatureFlags::None: return D3D12_ROOT_SIGNATURE_FLAG_NONE;
            case RootSignatureFlags::AllowInputAssemblerInputLayout: return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
            case RootSignatureFlags::DenyVertexShaderRootAccess: return D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
            case RootSignatureFlags::DenyHullShaderRootAccess: return D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
            case RootSignatureFlags::DenyDomainShaderRootAccess: return D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
            case RootSignatureFlags::DenyGeometryShaderRootAccess: return D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            case RootSignatureFlags::DenyPixelShaderRootAccess: return D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
            case RootSignatureFlags::AllowStreamOutput: return D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
            }
            return D3D12_ROOT_SIGNATURE_FLAG_NONE;
        }

        D3D12_RESOURCE_BARRIER_FLAGS dxFlags(ResourceBarrierFlags flags)
        {
            switch (flags)
            {
            case ResourceBarrierFlags::None: return D3D12_RESOURCE_BARRIER_FLAG_NONE;
            case ResourceBarrierFlags::BeginOnly: return D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
            case ResourceBarrierFlags::EndOnly: return D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;
            default: return D3D12_RESOURCE_BARRIER_FLAG_NONE;
            }
        }

        D3D12_RESOURCE_STATES dxResourceStates(ResourceState state)
        {
            switch (state)
            {
            case ResourceState::Common: return D3D12_RESOURCE_STATE_COMMON;
            case ResourceState::VertexAndConstantBuffer: return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            case ResourceState::IndexBuffer: return D3D12_RESOURCE_STATE_INDEX_BUFFER;
            case ResourceState::RenderTarget: return D3D12_RESOURCE_STATE_RENDER_TARGET;
            case ResourceState::UnorderedAccess: return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            case ResourceState::DepthWrite: return D3D12_RESOURCE_STATE_DEPTH_WRITE;
            case ResourceState::DepthRead: return D3D12_RESOURCE_STATE_DEPTH_READ;
            case ResourceState::NonPixelShaderResource: return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            case ResourceState::PixelShaderResource: return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            case ResourceState::StreamOut: return D3D12_RESOURCE_STATE_STREAM_OUT;
            case ResourceState::IndirectArgument: return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
            case ResourceState::CopyDest: return D3D12_RESOURCE_STATE_COPY_DEST;
            case ResourceState::CopySource: return D3D12_RESOURCE_STATE_COPY_SOURCE;
            case ResourceState::ResolveDest: return D3D12_RESOURCE_STATE_RESOLVE_DEST;
            case ResourceState::ResolveSource: return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
            case ResourceState::GenericRead: return D3D12_RESOURCE_STATE_GENERIC_READ;
            case ResourceState::Present: return D3D12_RESOURCE_STATE_PRESENT;
            case ResourceState::Predication: return D3D12_RESOURCE_STATE_PREDICATION;
            default: return D3D12_RESOURCE_STATE_COMMON;
            }
        }

        DXGI_FORMAT dxFormat(Format format)
        {
            switch (format)
            {
                case Format::UNKNOWN: return DXGI_FORMAT_UNKNOWN;
                case Format::R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
                case Format::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
                case Format::R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
                case Format::R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
                case Format::R32G32B32_TYPELESS: return DXGI_FORMAT_R32G32B32_TYPELESS;
                case Format::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
                case Format::R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
                case Format::R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
                case Format::R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
                case Format::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
                case Format::R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
                case Format::R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
                case Format::R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
                case Format::R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
                case Format::R32G32_TYPELESS: return DXGI_FORMAT_R32G32_TYPELESS;
                case Format::R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
                case Format::R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
                case Format::R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
                case Format::R32G8X24_TYPELESS: return DXGI_FORMAT_R32G8X24_TYPELESS;
                case Format::D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
                case Format::R32_FLOAT_X8X24_TYPELESS: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                case Format::X32_TYPELESS_G8X24_UINT: return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
                case Format::R10G10B10A2_TYPELESS: return DXGI_FORMAT_R10G10B10A2_TYPELESS;
                case Format::R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
                case Format::R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;
                case Format::R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
                case Format::R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
                case Format::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
                case Format::R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
                case Format::R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
                case Format::R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
                case Format::R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
                case Format::R16G16_TYPELESS: return DXGI_FORMAT_R16G16_TYPELESS;
                case Format::R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
                case Format::R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
                case Format::R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
                case Format::R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
                case Format::R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
                case Format::R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
                case Format::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
                case Format::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
                case Format::R32_UINT: return DXGI_FORMAT_R32_UINT;
                case Format::R32_SINT: return DXGI_FORMAT_R32_SINT;
                case Format::R24G8_TYPELESS: return DXGI_FORMAT_R24G8_TYPELESS;
                case Format::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
                case Format::R24_UNORM_X8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                case Format::X24_TYPELESS_G8_UINT: return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
                case Format::R8G8_TYPELESS: return DXGI_FORMAT_R8G8_TYPELESS;
                case Format::R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
                case Format::R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
                case Format::R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
                case Format::R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
                case Format::R16_TYPELESS: return DXGI_FORMAT_R16_TYPELESS;
                case Format::R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
                case Format::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
                case Format::R16_UNORM: return DXGI_FORMAT_R16_UNORM;
                case Format::R16_UINT: return DXGI_FORMAT_R16_UINT;
                case Format::R16_SNORM: return DXGI_FORMAT_R16_SNORM;
                case Format::R16_SINT: return DXGI_FORMAT_R16_SINT;
                case Format::R8_TYPELESS: return DXGI_FORMAT_R8_TYPELESS;
                case Format::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
                case Format::R8_UINT: return DXGI_FORMAT_R8_UINT;
                case Format::R8_SNORM: return DXGI_FORMAT_R8_SNORM;
                case Format::R8_SINT: return DXGI_FORMAT_R8_SINT;
                case Format::A8_UNORM: return DXGI_FORMAT_A8_UNORM;
                case Format::R1_UNORM: return DXGI_FORMAT_R1_UNORM;
                case Format::R9G9B9E5_SHAREDEXP: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
                case Format::R8G8_B8G8_UNORM: return DXGI_FORMAT_R8G8_B8G8_UNORM;
                case Format::G8R8_G8B8_UNORM: return DXGI_FORMAT_G8R8_G8B8_UNORM;
                case Format::BC1_TYPELESS: return DXGI_FORMAT_BC1_TYPELESS;
                case Format::BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
                case Format::BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
                case Format::BC2_TYPELESS: return DXGI_FORMAT_BC2_TYPELESS;
                case Format::BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
                case Format::BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
                case Format::BC3_TYPELESS: return DXGI_FORMAT_BC3_TYPELESS;
                case Format::BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
                case Format::BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
                case Format::BC4_TYPELESS: return DXGI_FORMAT_BC4_TYPELESS;
                case Format::BC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
                case Format::BC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
                case Format::BC5_TYPELESS: return DXGI_FORMAT_BC5_TYPELESS;
                case Format::BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
                case Format::BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
                case Format::B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
                case Format::B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
                case Format::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
                case Format::B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
                case Format::R10G10B10_XR_BIAS_A2_UNORM: return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
                case Format::B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
                case Format::B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
                case Format::B8G8R8X8_TYPELESS: return DXGI_FORMAT_B8G8R8X8_TYPELESS;
                case Format::B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
                case Format::BC6H_TYPELESS: return DXGI_FORMAT_BC6H_TYPELESS;
                case Format::BC6H_UF16: return DXGI_FORMAT_BC6H_UF16;
                case Format::BC6H_SF16: return DXGI_FORMAT_BC6H_SF16;
                case Format::BC7_TYPELESS: return DXGI_FORMAT_BC7_TYPELESS;
                case Format::BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
                case Format::BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
                case Format::AYUV: return DXGI_FORMAT_AYUV;
                case Format::Y410: return DXGI_FORMAT_Y410;
                case Format::Y416: return DXGI_FORMAT_Y416;
                case Format::NV12: return DXGI_FORMAT_NV12;
                case Format::P010: return DXGI_FORMAT_P010;
                case Format::P016: return DXGI_FORMAT_P016;
                case Format::OPAQUE_420: return DXGI_FORMAT_420_OPAQUE;
                case Format::YUY2: return DXGI_FORMAT_YUY2;
                case Format::Y210: return DXGI_FORMAT_Y210;
                case Format::Y216: return DXGI_FORMAT_Y216;
                case Format::NV11: return DXGI_FORMAT_NV11;
                case Format::AI44: return DXGI_FORMAT_AI44;
                case Format::IA44: return DXGI_FORMAT_IA44;
                case Format::P8: return DXGI_FORMAT_P8;
                case Format::A8P8: return DXGI_FORMAT_A8P8;
                case Format::B4G4R4A4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM;
                case Format::P208: return DXGI_FORMAT_P208;
                case Format::V208: return DXGI_FORMAT_V208;
                case Format::V408: return DXGI_FORMAT_V408;
                default: return DXGI_FORMAT_R8G8B8A8_UNORM;
            }
        }

        DXGI_FORMAT dxTypelessFormat(Format format)
        {
            switch (format)
            {
            case Format::UNKNOWN: return DXGI_FORMAT_UNKNOWN;
            case Format::R32G32B32A32_TYPELESS: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case Format::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case Format::R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case Format::R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case Format::R32G32B32_TYPELESS: return DXGI_FORMAT_R32G32B32_TYPELESS;
            case Format::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_TYPELESS;
            case Format::R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_TYPELESS;
            case Format::R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_TYPELESS;
            case Format::R16G16B16A16_TYPELESS: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case Format::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case Format::R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case Format::R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case Format::R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case Format::R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case Format::R32G32_TYPELESS: return DXGI_FORMAT_R32G32_TYPELESS;
            case Format::R32G32_FLOAT: return DXGI_FORMAT_R32G32_TYPELESS;
            case Format::R32G32_UINT: return DXGI_FORMAT_R32G32_TYPELESS;
            case Format::R32G32_SINT: return DXGI_FORMAT_R32G32_TYPELESS;
            case Format::R32G8X24_TYPELESS: return DXGI_FORMAT_R32G8X24_TYPELESS;
            case Format::D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32G8X24_TYPELESS;
            case Format::R32_FLOAT_X8X24_TYPELESS: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            case Format::X32_TYPELESS_G8X24_UINT: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            case Format::R10G10B10A2_TYPELESS: return DXGI_FORMAT_R10G10B10A2_TYPELESS;
            case Format::R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_TYPELESS;
            case Format::R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_TYPELESS;
            case Format::R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
            case Format::R8G8B8A8_TYPELESS: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case Format::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case Format::R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case Format::R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case Format::R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case Format::R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case Format::R16G16_TYPELESS: return DXGI_FORMAT_R16G16_TYPELESS;
            case Format::R16G16_FLOAT: return DXGI_FORMAT_R16G16_TYPELESS;
            case Format::R16G16_UNORM: return DXGI_FORMAT_R16G16_TYPELESS;
            case Format::R16G16_UINT: return DXGI_FORMAT_R16G16_TYPELESS;
            case Format::R16G16_SNORM: return DXGI_FORMAT_R16G16_TYPELESS;
            case Format::R16G16_SINT: return DXGI_FORMAT_R16G16_TYPELESS;
            case Format::R32_TYPELESS: return DXGI_FORMAT_R32_TYPELESS;
            case Format::D32_FLOAT: return DXGI_FORMAT_R32_TYPELESS;
            case Format::R32_FLOAT: return DXGI_FORMAT_R32_TYPELESS;
            case Format::R32_UINT: return DXGI_FORMAT_R32_TYPELESS;
            case Format::R32_SINT: return DXGI_FORMAT_R32_TYPELESS;
            case Format::R24G8_TYPELESS: return DXGI_FORMAT_R24G8_TYPELESS;
            case Format::D24_UNORM_S8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;
            case Format::R24_UNORM_X8_TYPELESS: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case Format::X24_TYPELESS_G8_UINT: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case Format::R8G8_TYPELESS: return DXGI_FORMAT_R8G8_TYPELESS;
            case Format::R8G8_UNORM: return DXGI_FORMAT_R8G8_TYPELESS;
            case Format::R8G8_UINT: return DXGI_FORMAT_R8G8_TYPELESS;
            case Format::R8G8_SNORM: return DXGI_FORMAT_R8G8_TYPELESS;
            case Format::R8G8_SINT: return DXGI_FORMAT_R8G8_TYPELESS;
            case Format::R16_TYPELESS: return DXGI_FORMAT_R16_TYPELESS;
            case Format::R16_FLOAT: return DXGI_FORMAT_R16_TYPELESS;
            case Format::D16_UNORM: return DXGI_FORMAT_R16_TYPELESS;
            case Format::R16_UNORM: return DXGI_FORMAT_R16_TYPELESS;
            case Format::R16_UINT: return DXGI_FORMAT_R16_TYPELESS;
            case Format::R16_SNORM: return DXGI_FORMAT_R16_TYPELESS;
            case Format::R16_SINT: return DXGI_FORMAT_R16_TYPELESS;
            case Format::R8_TYPELESS: return DXGI_FORMAT_R8_TYPELESS;
            case Format::R8_UNORM: return DXGI_FORMAT_R8_TYPELESS;
            case Format::R8_UINT: return DXGI_FORMAT_R8_TYPELESS;
            case Format::R8_SNORM: return DXGI_FORMAT_R8_TYPELESS;
            case Format::R8_SINT: return DXGI_FORMAT_R8_TYPELESS;
            case Format::A8_UNORM: return DXGI_FORMAT_R8_TYPELESS;
            case Format::R1_UNORM: return DXGI_FORMAT_R8_TYPELESS;
            case Format::R9G9B9E5_SHAREDEXP: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            case Format::R8G8_B8G8_UNORM: return DXGI_FORMAT_R8G8_B8G8_UNORM;
            case Format::G8R8_G8B8_UNORM: return DXGI_FORMAT_G8R8_G8B8_UNORM;
            case Format::BC1_TYPELESS: return DXGI_FORMAT_BC1_TYPELESS;
            case Format::BC1_UNORM: return DXGI_FORMAT_BC1_TYPELESS;
            case Format::BC1_UNORM_SRGB: return DXGI_FORMAT_BC1_TYPELESS;
            case Format::BC2_TYPELESS: return DXGI_FORMAT_BC2_TYPELESS;
            case Format::BC2_UNORM: return DXGI_FORMAT_BC2_TYPELESS;
            case Format::BC2_UNORM_SRGB: return DXGI_FORMAT_BC2_TYPELESS;
            case Format::BC3_TYPELESS: return DXGI_FORMAT_BC3_TYPELESS;
            case Format::BC3_UNORM: return DXGI_FORMAT_BC3_TYPELESS;
            case Format::BC3_UNORM_SRGB: return DXGI_FORMAT_BC3_TYPELESS;
            case Format::BC4_TYPELESS: return DXGI_FORMAT_BC4_TYPELESS;
            case Format::BC4_UNORM: return DXGI_FORMAT_BC4_TYPELESS;
            case Format::BC4_SNORM: return DXGI_FORMAT_BC4_TYPELESS;
            case Format::BC5_TYPELESS: return DXGI_FORMAT_BC5_TYPELESS;
            case Format::BC5_UNORM: return DXGI_FORMAT_BC5_TYPELESS;
            case Format::BC5_SNORM: return DXGI_FORMAT_BC5_TYPELESS;
            case Format::B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
            case Format::B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
            case Format::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
            case Format::B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
            case Format::R10G10B10_XR_BIAS_A2_UNORM: return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
            case Format::B8G8R8A8_TYPELESS: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
            case Format::B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_TYPELESS;
            case Format::B8G8R8X8_TYPELESS: return DXGI_FORMAT_B8G8R8X8_TYPELESS;
            case Format::B8G8R8X8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8X8_TYPELESS;
            case Format::BC6H_TYPELESS: return DXGI_FORMAT_BC6H_TYPELESS;
            case Format::BC6H_UF16: return DXGI_FORMAT_BC6H_TYPELESS;
            case Format::BC6H_SF16: return DXGI_FORMAT_BC6H_TYPELESS;
            case Format::BC7_TYPELESS: return DXGI_FORMAT_BC7_TYPELESS;
            case Format::BC7_UNORM: return DXGI_FORMAT_BC7_TYPELESS;
            case Format::BC7_UNORM_SRGB: return DXGI_FORMAT_BC7_TYPELESS;
            case Format::AYUV: return DXGI_FORMAT_AYUV;
            case Format::Y410: return DXGI_FORMAT_Y410;
            case Format::Y416: return DXGI_FORMAT_Y416;
            case Format::NV12: return DXGI_FORMAT_NV12;
            case Format::P010: return DXGI_FORMAT_P010;
            case Format::P016: return DXGI_FORMAT_P016;
            case Format::OPAQUE_420: return DXGI_FORMAT_420_OPAQUE;
            case Format::YUY2: return DXGI_FORMAT_YUY2;
            case Format::Y210: return DXGI_FORMAT_Y210;
            case Format::Y216: return DXGI_FORMAT_Y216;
            case Format::NV11: return DXGI_FORMAT_NV11;
            case Format::AI44: return DXGI_FORMAT_AI44;
            case Format::IA44: return DXGI_FORMAT_IA44;
            case Format::P8: return DXGI_FORMAT_P8;
            case Format::A8P8: return DXGI_FORMAT_A8P8;
            case Format::B4G4R4A4_UNORM: return DXGI_FORMAT_B4G4R4A4_UNORM;
            case Format::P208: return DXGI_FORMAT_P208;
            case Format::V208: return DXGI_FORMAT_V208;
            case Format::V408: return DXGI_FORMAT_V408;
            default: return DXGI_FORMAT_R8G8B8A8_UNORM;
            }
        }

        D3D12_VIEWPORT dxViewport(Viewport viewPort)
        {
            return D3D12_VIEWPORT{
                static_cast<FLOAT>(viewPort.topLeftX),
                static_cast<FLOAT>(viewPort.topLeftY),
                static_cast<FLOAT>(viewPort.width),
                static_cast<FLOAT>(viewPort.height),
                static_cast<FLOAT>(viewPort.minDepth),
                static_cast<FLOAT>(viewPort.maxDepth) };
        }

        D3D12_RECT dxRect(Rectangle rect)
        {
            return D3D12_RECT{
                static_cast<LONG>(rect.left),
                static_cast<LONG>(rect.top),
                static_cast<LONG>(rect.right),
                static_cast<LONG>(rect.bottom)
            };
        }

        D3D12_RESOURCE_DIMENSION dxResourceDimension(ResourceDimension dim)
        {
            switch (dim)
            {
                case ResourceDimension::Unknown: return D3D12_RESOURCE_DIMENSION_UNKNOWN;
                case ResourceDimension::Texture1D: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                case ResourceDimension::Texture1DArray: return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
                case ResourceDimension::Texture2D: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                case ResourceDimension::Texture2DArray: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                case ResourceDimension::Texture3D: return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
                case ResourceDimension::TextureCube: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                case ResourceDimension::TextureCubeArray: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            }
            return D3D12_RESOURCE_DIMENSION_UNKNOWN;
        }

        DXGI_SAMPLE_DESC dxSampleDescription(SampleDescription desc)
        {
            return DXGI_SAMPLE_DESC{ 
                static_cast<UINT>(desc.count),
                static_cast<UINT>(desc.quality) };
        }

        D3D12_TEXTURE_LAYOUT dxTextureLayout(TextureLayout layout)
        {
            switch (layout)
            {
                case TextureLayout::Unknown: return D3D12_TEXTURE_LAYOUT_UNKNOWN;
                case TextureLayout::RowMajor: return D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                case TextureLayout::UndefinedSwizzle64KB: return D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE;
                case TextureLayout::StandardSwizzle64KB: return D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE;
            }
            return D3D12_TEXTURE_LAYOUT_UNKNOWN;
        }

        D3D12_RESOURCE_FLAGS dxResourceFlags(ResourceFlags flags)
        {
            switch (flags)
            {
                case ResourceFlags::None: return D3D12_RESOURCE_FLAG_NONE;
                case ResourceFlags::AllowRenderTarget: return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                case ResourceFlags::AllowDepthStencil: return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                case ResourceFlags::AllowUnorderedAccess: return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
                case ResourceFlags::DenyShaderResource: return D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
                case ResourceFlags::AllowCrossAdapter: return D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
                case ResourceFlags::AllowSimultaneousAccess: return D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
            }
            return D3D12_RESOURCE_FLAG_NONE;
        }

        D3D12_DEPTH_STENCIL_VALUE dxDepthStencilValue(DepthStencilValue val)
        {
            return D3D12_DEPTH_STENCIL_VALUE{ 
                static_cast<FLOAT>(val.depth),
                static_cast<UINT8>(val.stencil) };
        }

        D3D12_BLEND dxBlend(Blend blend)
        {
            switch (blend)
            {
                case Blend::Zero: return D3D12_BLEND_ZERO;
                case Blend::One: return D3D12_BLEND_ONE;
                case Blend::SrcColor: return D3D12_BLEND_SRC_COLOR;
                case Blend::InvSrcColor: return D3D12_BLEND_INV_SRC_COLOR;
                case Blend::SrcAlpha: return D3D12_BLEND_SRC_ALPHA;
                case Blend::InvSrcAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
                case Blend::DestAlpha: return D3D12_BLEND_DEST_ALPHA;
                case Blend::InvDestAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
                case Blend::DestColor: return D3D12_BLEND_DEST_COLOR;
                case Blend::InvDestColor: return D3D12_BLEND_INV_DEST_COLOR;
                case Blend::SrcAlphaSaturate: return D3D12_BLEND_SRC_ALPHA_SAT;
                case Blend::BlendFactor: return D3D12_BLEND_BLEND_FACTOR;
                case Blend::InvBlendFactor: return D3D12_BLEND_INV_BLEND_FACTOR;
                case Blend::Src1Color: return D3D12_BLEND_SRC1_COLOR;
                case Blend::InvSrc1Color: return D3D12_BLEND_INV_SRC1_COLOR;
                case Blend::Src1Alpha: return D3D12_BLEND_SRC1_ALPHA;
                case Blend::InvSrc1Alpha: return D3D12_BLEND_INV_SRC1_ALPHA;
            }
            return D3D12_BLEND_ZERO;
        }

        D3D12_BLEND_OP dxBlendOp(BlendOperation op)
        {
            switch (op)
            {
                case BlendOperation::Add: return D3D12_BLEND_OP_ADD;
                case BlendOperation::Subtract: return D3D12_BLEND_OP_SUBTRACT;
                case BlendOperation::RevSubtract: return D3D12_BLEND_OP_REV_SUBTRACT;
                case BlendOperation::Min: return D3D12_BLEND_OP_MIN;
                case BlendOperation::Max: return D3D12_BLEND_OP_MAX;
            }
            return D3D12_BLEND_OP_ADD;
        }

        D3D12_LOGIC_OP dxLogicOp(LogicOperation op)
        {
            switch (op)
            {
                case LogicOperation::Clear: return D3D12_LOGIC_OP_CLEAR;
                case LogicOperation::Set: return D3D12_LOGIC_OP_SET;
                case LogicOperation::Copy: return D3D12_LOGIC_OP_COPY;
                case LogicOperation::CopyInverted: return D3D12_LOGIC_OP_COPY_INVERTED;
                case LogicOperation::Noop: return D3D12_LOGIC_OP_NOOP;
                case LogicOperation::Invert: return D3D12_LOGIC_OP_INVERT;
                case LogicOperation::And: return D3D12_LOGIC_OP_AND;
                case LogicOperation::Nand: return D3D12_LOGIC_OP_NAND;
                case LogicOperation::Or: return D3D12_LOGIC_OP_OR;
                case LogicOperation::Nor: return D3D12_LOGIC_OP_NOR;
                case LogicOperation::Xor: return D3D12_LOGIC_OP_XOR;
                case LogicOperation::Equiv: return D3D12_LOGIC_OP_EQUIV;
                case LogicOperation::AndReverse: return D3D12_LOGIC_OP_AND_REVERSE;
                case LogicOperation::AndInverted: return D3D12_LOGIC_OP_AND_INVERTED;
                case LogicOperation::OrReverse: return D3D12_LOGIC_OP_OR_REVERSE;
                case LogicOperation::OrInverted: return D3D12_LOGIC_OP_OR_INVERTED;
            }
            return D3D12_LOGIC_OP_CLEAR;
        }

        D3D12_RENDER_TARGET_BLEND_DESC dxRenderTargetBlendDesc(const RenderTargetBlendDescription& desc)
        {
            return D3D12_RENDER_TARGET_BLEND_DESC{
                static_cast<BOOL>(desc.desc.blendEnable),
                static_cast<BOOL>(desc.desc.logicOpEnable),
                dxBlend(desc.desc.srcBlend),
                dxBlend(desc.desc.dstBlend),
                dxBlendOp(desc.desc.blendOp),
                dxBlend(desc.desc.srcBlendAlpha),
                dxBlend(desc.desc.dstBlendAlpha),
                dxBlendOp(desc.desc.blendOpAlpha),
                dxLogicOp(desc.desc.logicOp),
                static_cast<UINT8>(desc.desc.renderTargetWriteMask)
            };
        }

        D3D12_BLEND_DESC dxBlendDesc(const BlendDescription& desc)
        {
            return D3D12_BLEND_DESC{
                static_cast<BOOL>(desc.desc.alphaToCoverageEnable),
                static_cast<BOOL>(desc.desc.independentBlendEnable),
                { 
                    dxRenderTargetBlendDesc(desc.desc.renderTarget[0]),
                    dxRenderTargetBlendDesc(desc.desc.renderTarget[1]),
                    dxRenderTargetBlendDesc(desc.desc.renderTarget[2]),
                    dxRenderTargetBlendDesc(desc.desc.renderTarget[3]),
                    dxRenderTargetBlendDesc(desc.desc.renderTarget[4]),
                    dxRenderTargetBlendDesc(desc.desc.renderTarget[5]),
                    dxRenderTargetBlendDesc(desc.desc.renderTarget[6]),
                    dxRenderTargetBlendDesc(desc.desc.renderTarget[7])
                }
            };
        }

        D3D12_FILL_MODE dxFillMode(FillMode mode)
        {
            switch (mode)
            {
                case FillMode::Wireframe: return D3D12_FILL_MODE_WIREFRAME;
                case FillMode::Solid: return D3D12_FILL_MODE_SOLID;
                case FillMode::Point: return D3D12_FILL_MODE_WIREFRAME;
            }
            return D3D12_FILL_MODE_WIREFRAME;
        }

        D3D12_CULL_MODE dxCullMode(CullMode mode)
        {
            switch (mode)
            {
                case CullMode::None: return D3D12_CULL_MODE_NONE;
                case CullMode::Front: return D3D12_CULL_MODE_FRONT;
                case CullMode::Back: return D3D12_CULL_MODE_BACK;
            }
            return D3D12_CULL_MODE_NONE;
        }

        D3D12_CONSERVATIVE_RASTERIZATION_MODE dxConservativeRasterizationMode(ConservativeRasterizationMode mode)
        {
            switch (mode)
            {
                case ConservativeRasterizationMode::Off: return D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
                case ConservativeRasterizationMode::On: return D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
            }
            return D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        D3D12_RASTERIZER_DESC dxRasterizerDesc(const RasterizerDescription& desc)
        {
            return D3D12_RASTERIZER_DESC{
                dxFillMode(desc.desc.fillMode),
                dxCullMode(desc.desc.cullMode),
                static_cast<BOOL>(desc.desc.frontCounterClockwise),
                static_cast<INT>(desc.desc.depthBias),
                static_cast<FLOAT>(desc.desc.depthBiasClamp),
                static_cast<FLOAT>(desc.desc.slopeScaledDepthBias),
                static_cast<BOOL>(desc.desc.depthClipEnable),
                static_cast<BOOL>(desc.desc.multisampleEnable),
                static_cast<BOOL>(desc.desc.antialiasedLineEnable),
                static_cast<UINT>(desc.desc.forcedSampleCount),
                dxConservativeRasterizationMode(desc.desc.conservativeRaster)
            };
        }

        D3D12_DEPTH_WRITE_MASK dxDepthWriteMask(DepthWriteMask mask)
        {
            switch (mask)
            {
                case DepthWriteMask::Zero: return D3D12_DEPTH_WRITE_MASK_ZERO;
                case DepthWriteMask::All: return D3D12_DEPTH_WRITE_MASK_ALL;
            }
            return D3D12_DEPTH_WRITE_MASK_ZERO;
        }

        D3D12_STENCIL_OP dxStencilOp(StencilOp op)
        {
            switch (op)
            {
                case StencilOp::Keep: return D3D12_STENCIL_OP_KEEP;
                case StencilOp::Zero: return D3D12_STENCIL_OP_ZERO;
                case StencilOp::Replace: return D3D12_STENCIL_OP_REPLACE;
                case StencilOp::IncrSat: return D3D12_STENCIL_OP_INCR_SAT;
                case StencilOp::DecrSat: return D3D12_STENCIL_OP_DECR_SAT;
                case StencilOp::Invert: return D3D12_STENCIL_OP_INVERT;
                case StencilOp::Incr: return D3D12_STENCIL_OP_INCR;
                case StencilOp::Decr: return D3D12_STENCIL_OP_DECR;
            }
            return D3D12_STENCIL_OP_KEEP;
        }

        D3D12_DEPTH_STENCILOP_DESC dxDepthStencilOpDesc(const DepthStencilOpDescription& desc)
        {
            return D3D12_DEPTH_STENCILOP_DESC{
                dxStencilOp(desc.StencilFailOp),
                dxStencilOp(desc.StencilDepthFailOp),
                dxStencilOp(desc.StencilPassOp),
                dxComparisonFunc(desc.StencilFunc)
            };
        }

        D3D12_DEPTH_STENCIL_DESC dxDepthStencilDesc(const DepthStencilDescription& desc)
        {
            return D3D12_DEPTH_STENCIL_DESC{
                static_cast<BOOL>(desc.desc.depthEnable),
                dxDepthWriteMask(desc.desc.depthWriteMask),
                dxComparisonFunc(desc.desc.depthFunc),
                static_cast<BOOL>(desc.desc.stencilEnable),
                static_cast<UINT8>(desc.desc.stencilReadMask),
                static_cast<UINT8>(desc.desc.stencilWriteMask),
                dxDepthStencilOpDesc(desc.desc.frontFace),
                dxDepthStencilOpDesc(desc.desc.backFace)
            };
        }

        D3D_PRIMITIVE_TOPOLOGY dxPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency)
        {
            switch (type)
            {
                case PrimitiveTopologyType::Undefined        : return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
                case PrimitiveTopologyType::PointList        : return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
                case PrimitiveTopologyType::LineList        : return adjacency ? D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ : D3D_PRIMITIVE_TOPOLOGY_LINELIST;
                case PrimitiveTopologyType::LineStrip        : return adjacency ? D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ : D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
                case PrimitiveTopologyType::TriangleList    : return adjacency ? D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ : D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                case PrimitiveTopologyType::TriangleStrip    : return adjacency ? D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ : D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                case PrimitiveTopologyType::PatchList1        : return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList2        : return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList3        : return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList4        : return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList5        : return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList6        : return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList7        : return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList8        : return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList9        : return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList10        : return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList11        : return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList12        : return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList13        : return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList14        : return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList15        : return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
                case PrimitiveTopologyType::PatchList16        : return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
            }
            return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }

        D3D12_PRIMITIVE_TOPOLOGY_TYPE dxPrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY type)
        {
            switch (type)
            {
            case D3D_PRIMITIVE_TOPOLOGY_UNDEFINED                    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
            case D3D_PRIMITIVE_TOPOLOGY_POINTLIST                    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case D3D_PRIMITIVE_TOPOLOGY_LINELIST                    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP                    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST                : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP                : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ                : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ               : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ            : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ           : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            case D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            case D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST    : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
            }
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        }

        D3D12_INPUT_CLASSIFICATION dxInputClassification(InputClassification cls)
        {
            switch (cls)
            {
                case InputClassification::PerVertexData: return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                case InputClassification::PerInstanceData: return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
            }
            return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        }

        D3D12_INPUT_ELEMENT_DESC dxInputElementDesc(const InputElementDescription& desc)
        {
            return D3D12_INPUT_ELEMENT_DESC{
                static_cast<LPCSTR>(desc.desc.semanticName),
                static_cast<UINT>(desc.desc.semanticIndex),
                dxFormat(desc.desc.format),
                static_cast<UINT>(desc.desc.inputSlot),
                static_cast<UINT>(desc.desc.alignedByteOffset),
                dxInputClassification(desc.desc.inputSlotClass),
                static_cast<UINT>(desc.desc.instanceDataStepRate)
            };
        }

        D3D12_INDEX_BUFFER_STRIP_CUT_VALUE dxIndexBufferStripCutValue(IndexBufferStripCutValue value)
        {
            switch (value)
            {
                case IndexBufferStripCutValue::ValueDisabled: return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
                case IndexBufferStripCutValue::Value0xFFFF: return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF;
                case IndexBufferStripCutValue::Value0xFFFFFFFF: return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF;
            }
            return D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        }

        D3D12_COMMAND_LIST_TYPE dxCommandListType(CommandListType type)
        {
            switch (type)
            {
                case CommandListType::Direct: return D3D12_COMMAND_LIST_TYPE_DIRECT;
                case CommandListType::Bundle: return D3D12_COMMAND_LIST_TYPE_BUNDLE;
                case CommandListType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
                case CommandListType::Copy: return D3D12_COMMAND_LIST_TYPE_COPY;
            }
            return D3D12_COMMAND_LIST_TYPE_DIRECT;
        }

        D3D12_PREDICATION_OP dxPredicationOp(PredicationOp op)
        {
            switch (op)
            {
                case PredicationOp::EqualZero: return D3D12_PREDICATION_OP_EQUAL_ZERO;
                case PredicationOp::NotEqualZero: return D3D12_PREDICATION_OP_NOT_EQUAL_ZERO;
            }
            return D3D12_PREDICATION_OP_EQUAL_ZERO;
        }

        ///////////////////////


        ShaderVisibility fromDXShaderVisibility(D3D12_SHADER_VISIBILITY visibility)
        {
            if (visibility == D3D12_SHADER_VISIBILITY_ALL) return static_cast<ShaderVisibility>(ShaderVisibilityBits::All);
            if (visibility == D3D12_SHADER_VISIBILITY_VERTEX) return static_cast<ShaderVisibility>(ShaderVisibilityBits::Vertex);
            if (visibility == D3D12_SHADER_VISIBILITY_HULL) return static_cast<ShaderVisibility>(ShaderVisibilityBits::Hull);
            if (visibility == D3D12_SHADER_VISIBILITY_DOMAIN) return static_cast<ShaderVisibility>(ShaderVisibilityBits::Domain);
            if (visibility == D3D12_SHADER_VISIBILITY_GEOMETRY) return static_cast<ShaderVisibility>(ShaderVisibilityBits::Geometry);
            if (visibility == D3D12_SHADER_VISIBILITY_PIXEL) return static_cast<ShaderVisibility>(ShaderVisibilityBits::Pixel);
#ifdef DXR_BUILD
            if (visibility == D3D12_SHADER_VISIBILITY_AMPLIFICATION) return static_cast<ShaderVisibility>(ShaderVisibilityBits::Amplification);
            if (visibility == D3D12_SHADER_VISIBILITY_MESH) return static_cast<ShaderVisibility>(ShaderVisibilityBits::Mesh);
#endif
            return static_cast<ShaderVisibility>(ShaderVisibilityBits::All);
        }

        DescriptorRangeType fromDXRangeType(D3D12_DESCRIPTOR_RANGE_TYPE type)
        {
            switch (type)
            {
                case D3D12_DESCRIPTOR_RANGE_TYPE_SRV: return DescriptorRangeType::SRV;
                case D3D12_DESCRIPTOR_RANGE_TYPE_UAV: return DescriptorRangeType::UAV;
                case D3D12_DESCRIPTOR_RANGE_TYPE_CBV: return DescriptorRangeType::CBV;
                case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER: return DescriptorRangeType::Sampler;
            }
            return DescriptorRangeType::SRV;
        }

        Filter fromDXFilter(D3D12_FILTER filter)
        {
            switch (filter)
            {
            case D3D12_FILTER_MIN_MAG_MIP_POINT: return Filter::Point;
            case D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT: return Filter::Bilinear;
            case D3D12_FILTER_MIN_MAG_MIP_LINEAR: return Filter::Trilinear;
            case D3D12_FILTER_ANISOTROPIC: return Filter::Anisotropic;
            }
            return Filter::Point;
        }

        TextureAddressMode fromDXTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE mode)
        {
            switch (mode)
            {
                case D3D12_TEXTURE_ADDRESS_MODE_WRAP: return TextureAddressMode::Wrap;
                case D3D12_TEXTURE_ADDRESS_MODE_MIRROR: return TextureAddressMode::Mirror;
                case D3D12_TEXTURE_ADDRESS_MODE_CLAMP: return TextureAddressMode::Clamp;
                case D3D12_TEXTURE_ADDRESS_MODE_BORDER: return TextureAddressMode::Border;
                case D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE: return TextureAddressMode::MirrorOnce;
            }
            return TextureAddressMode::Wrap;
        }

        ComparisonFunction fromDXComparisonFunc(D3D12_COMPARISON_FUNC comp)
        {
            switch (comp)
            {
                case D3D12_COMPARISON_FUNC_NEVER: return ComparisonFunction::Never;
                case D3D12_COMPARISON_FUNC_LESS: return ComparisonFunction::Less;
                case D3D12_COMPARISON_FUNC_EQUAL: return ComparisonFunction::Equal;
                case D3D12_COMPARISON_FUNC_LESS_EQUAL: return ComparisonFunction::LessEqual;
                case D3D12_COMPARISON_FUNC_GREATER: return ComparisonFunction::Greater;
                case D3D12_COMPARISON_FUNC_NOT_EQUAL: return ComparisonFunction::NotEqual;
                case D3D12_COMPARISON_FUNC_GREATER_EQUAL: return ComparisonFunction::GreaterEqual;
                case D3D12_COMPARISON_FUNC_ALWAYS: return ComparisonFunction::Always;
            }
            return ComparisonFunction::Never;
        }

        StaticBorderColor fromDXStaticBorderColor(D3D12_STATIC_BORDER_COLOR color)
        {
            switch (color)
            {
                case D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK: return StaticBorderColor::TransparentBlack;
                case D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK: return StaticBorderColor::OpaqueBlack;
                case D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE: return StaticBorderColor::OpaqueWhite;
            }
            return StaticBorderColor::TransparentBlack;
        }

        StaticSamplerDescription fromDXStaticSamplerDesc(const D3D12_STATIC_SAMPLER_DESC& desc)
        {
            return StaticSamplerDescription{
                fromDXFilter(desc.Filter),
                fromDXTextureAddressMode(desc.AddressU),
                fromDXTextureAddressMode(desc.AddressV),
                fromDXTextureAddressMode(desc.AddressW),
                static_cast<float>(desc.MipLODBias),
                static_cast<unsigned int>(desc.MaxAnisotropy),
                fromDXComparisonFunc(desc.ComparisonFunc),
                fromDXStaticBorderColor(desc.BorderColor),
                static_cast<float>(desc.MinLOD),
                static_cast<float>(desc.MaxLOD),
                static_cast<unsigned int>(desc.ShaderRegister),
                static_cast<unsigned int>(desc.RegisterSpace),
                fromDXShaderVisibility(desc.ShaderVisibility)
            };
        }

        RootSignatureFlags fromDXSignatureFlags(D3D12_ROOT_SIGNATURE_FLAGS flags)
        {
            switch (flags)
            {
                case D3D12_ROOT_SIGNATURE_FLAG_NONE: return RootSignatureFlags::None;
                case D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT: return RootSignatureFlags::AllowInputAssemblerInputLayout;
                case D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS: return RootSignatureFlags::DenyVertexShaderRootAccess;
                case D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS: return RootSignatureFlags::DenyHullShaderRootAccess;
                case D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS: return RootSignatureFlags::DenyDomainShaderRootAccess;
                case D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS: return RootSignatureFlags::DenyGeometryShaderRootAccess;
                case D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS: return RootSignatureFlags::DenyPixelShaderRootAccess;
                case D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT: return RootSignatureFlags::AllowStreamOutput;
            }
            return RootSignatureFlags::None;
        }

        ResourceBarrierFlags fromDXFlags(D3D12_RESOURCE_BARRIER_FLAGS flags)
        {
            switch (flags)
            {
                case D3D12_RESOURCE_BARRIER_FLAG_NONE: return ResourceBarrierFlags::None;
                case D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY: return ResourceBarrierFlags::BeginOnly;
                case D3D12_RESOURCE_BARRIER_FLAG_END_ONLY: return ResourceBarrierFlags::EndOnly;
            }
            return ResourceBarrierFlags::None;
        }

        ResourceState fromDXResourceStates(D3D12_RESOURCE_STATES state)
        {
            switch (state)
            {
                case D3D12_RESOURCE_STATE_COMMON: return ResourceState::Common;
                case D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER: return ResourceState::VertexAndConstantBuffer;
                case D3D12_RESOURCE_STATE_INDEX_BUFFER: return ResourceState::IndexBuffer;
                case D3D12_RESOURCE_STATE_RENDER_TARGET: return ResourceState::RenderTarget;
                case D3D12_RESOURCE_STATE_UNORDERED_ACCESS: return ResourceState::UnorderedAccess;
                case D3D12_RESOURCE_STATE_DEPTH_WRITE: return ResourceState::DepthWrite;
                case D3D12_RESOURCE_STATE_DEPTH_READ: return ResourceState::DepthRead;
                case D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE: return ResourceState::NonPixelShaderResource;
                case D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE: return ResourceState::PixelShaderResource;
                case D3D12_RESOURCE_STATE_STREAM_OUT: return ResourceState::StreamOut;
                case D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT: return ResourceState::IndirectArgument;
                case D3D12_RESOURCE_STATE_COPY_DEST: return ResourceState::CopyDest;
                case D3D12_RESOURCE_STATE_COPY_SOURCE: return ResourceState::CopySource;
                case D3D12_RESOURCE_STATE_RESOLVE_DEST: return ResourceState::ResolveDest;
                case D3D12_RESOURCE_STATE_RESOLVE_SOURCE: return ResourceState::ResolveSource;
                case D3D12_RESOURCE_STATE_GENERIC_READ: return ResourceState::GenericRead;
                /*case D3D12_RESOURCE_STATE_PRESENT: return ResourceState::Common;
                case D3D12_RESOURCE_STATE_PREDICATION: return ResourceState::Common;*/
            }
            return ResourceState::Common;
        }

        DescriptorHeapFlags fromDXDescriptorHeapFlags(D3D12_DESCRIPTOR_HEAP_FLAGS type)
        {
            switch (type)
            {
                case D3D12_DESCRIPTOR_HEAP_FLAG_NONE: return DescriptorHeapFlags::None;
                case D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE: return DescriptorHeapFlags::ShaderVisible;
            }
            return DescriptorHeapFlags::None;
        }

        Format fromDXFormat(DXGI_FORMAT format)
        {
            switch (format)
            {
            case DXGI_FORMAT_UNKNOWN: return Format::UNKNOWN;
            case DXGI_FORMAT_R32G32B32A32_TYPELESS: return Format::R32G32B32A32_TYPELESS;
            case DXGI_FORMAT_R32G32B32A32_FLOAT: return Format::R32G32B32A32_FLOAT;
            case DXGI_FORMAT_R32G32B32A32_UINT: return Format::R32G32B32A32_UINT;
            case DXGI_FORMAT_R32G32B32A32_SINT: return Format::R32G32B32A32_SINT;
            case DXGI_FORMAT_R32G32B32_TYPELESS: return Format::R32G32B32_TYPELESS;
            case DXGI_FORMAT_R32G32B32_FLOAT: return Format::R32G32B32_FLOAT;
            case DXGI_FORMAT_R32G32B32_UINT: return Format::R32G32B32_UINT;
            case DXGI_FORMAT_R32G32B32_SINT: return Format::R32G32B32_SINT;
            case DXGI_FORMAT_R16G16B16A16_TYPELESS: return Format::R16G16B16A16_TYPELESS;
            case DXGI_FORMAT_R16G16B16A16_FLOAT: return Format::R16G16B16A16_FLOAT;
            case DXGI_FORMAT_R16G16B16A16_UNORM: return Format::R16G16B16A16_UNORM;
            case DXGI_FORMAT_R16G16B16A16_UINT: return Format::R16G16B16A16_UINT;
            case DXGI_FORMAT_R16G16B16A16_SNORM: return Format::R16G16B16A16_SNORM;
            case DXGI_FORMAT_R16G16B16A16_SINT: return Format::R16G16B16A16_SINT;
            case DXGI_FORMAT_R32G32_TYPELESS: return Format::R32G32_TYPELESS;
            case DXGI_FORMAT_R32G32_FLOAT: return Format::R32G32_FLOAT;
            case DXGI_FORMAT_R32G32_UINT: return Format::R32G32_UINT;
            case DXGI_FORMAT_R32G32_SINT: return Format::R32G32_SINT;
            case DXGI_FORMAT_R32G8X24_TYPELESS: return Format::R32G8X24_TYPELESS;
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return Format::D32_FLOAT_S8X24_UINT;
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS: return Format::R32_FLOAT_X8X24_TYPELESS;
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT: return Format::X32_TYPELESS_G8X24_UINT;
            case DXGI_FORMAT_R10G10B10A2_TYPELESS: return Format::R10G10B10A2_TYPELESS;
            case DXGI_FORMAT_R10G10B10A2_UNORM: return Format::R10G10B10A2_UNORM;
            case DXGI_FORMAT_R10G10B10A2_UINT: return Format::R10G10B10A2_UINT;
            case DXGI_FORMAT_R11G11B10_FLOAT: return Format::R11G11B10_FLOAT;
            case DXGI_FORMAT_R8G8B8A8_TYPELESS: return Format::R8G8B8A8_TYPELESS;
            case DXGI_FORMAT_R8G8B8A8_UNORM: return Format::R8G8B8A8_UNORM;
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return Format::R8G8B8A8_UNORM_SRGB;
            case DXGI_FORMAT_R8G8B8A8_UINT: return Format::R8G8B8A8_UINT;
            case DXGI_FORMAT_R8G8B8A8_SNORM: return Format::R8G8B8A8_SNORM;
            case DXGI_FORMAT_R8G8B8A8_SINT: return Format::R8G8B8A8_SINT;
            case DXGI_FORMAT_R16G16_TYPELESS: return Format::R16G16_TYPELESS;
            case DXGI_FORMAT_R16G16_FLOAT: return Format::R16G16_FLOAT;
            case DXGI_FORMAT_R16G16_UNORM: return Format::R16G16_UNORM;
            case DXGI_FORMAT_R16G16_UINT: return Format::R16G16_UINT;
            case DXGI_FORMAT_R16G16_SNORM: return Format::R16G16_SNORM;
            case DXGI_FORMAT_R16G16_SINT: return Format::R16G16_SINT;
            case DXGI_FORMAT_R32_TYPELESS: return Format::R32_TYPELESS;
            case DXGI_FORMAT_D32_FLOAT: return Format::D32_FLOAT;
            case DXGI_FORMAT_R32_FLOAT: return Format::R32_FLOAT;
            case DXGI_FORMAT_R32_UINT: return Format::R32_UINT;
            case DXGI_FORMAT_R32_SINT: return Format::R32_SINT;
            case DXGI_FORMAT_R24G8_TYPELESS: return Format::R24G8_TYPELESS;
            case DXGI_FORMAT_D24_UNORM_S8_UINT: return Format::D24_UNORM_S8_UINT;
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS: return Format::R24_UNORM_X8_TYPELESS;
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT: return Format::X24_TYPELESS_G8_UINT;
            case DXGI_FORMAT_R8G8_TYPELESS: return Format::R8G8_TYPELESS;
            case DXGI_FORMAT_R8G8_UNORM: return Format::R8G8_UNORM;
            case DXGI_FORMAT_R8G8_UINT: return Format::R8G8_UINT;
            case DXGI_FORMAT_R8G8_SNORM: return Format::R8G8_SNORM;
            case DXGI_FORMAT_R8G8_SINT: return Format::R8G8_SINT;
            case DXGI_FORMAT_R16_TYPELESS: return Format::R16_TYPELESS;
            case DXGI_FORMAT_R16_FLOAT: return Format::R16_FLOAT;
            case DXGI_FORMAT_D16_UNORM: return Format::D16_UNORM;
            case DXGI_FORMAT_R16_UNORM: return Format::R16_UNORM;
            case DXGI_FORMAT_R16_UINT: return Format::R16_UINT;
            case DXGI_FORMAT_R16_SNORM: return Format::R16_SNORM;
            case DXGI_FORMAT_R16_SINT: return Format::R16_SINT;
            case DXGI_FORMAT_R8_TYPELESS: return Format::R8_TYPELESS;
            case DXGI_FORMAT_R8_UNORM: return Format::R8_UNORM;
            case DXGI_FORMAT_R8_UINT: return Format::R8_UINT;
            case DXGI_FORMAT_R8_SNORM: return Format::R8_SNORM;
            case DXGI_FORMAT_R8_SINT: return Format::R8_SINT;
            case DXGI_FORMAT_A8_UNORM: return Format::A8_UNORM;
            case DXGI_FORMAT_R1_UNORM: return Format::R1_UNORM;
            case DXGI_FORMAT_R9G9B9E5_SHAREDEXP: return Format::R9G9B9E5_SHAREDEXP;
            case DXGI_FORMAT_R8G8_B8G8_UNORM: return Format::R8G8_B8G8_UNORM;
            case DXGI_FORMAT_G8R8_G8B8_UNORM: return Format::G8R8_G8B8_UNORM;
            case DXGI_FORMAT_BC1_TYPELESS: return Format::BC1_TYPELESS;
            case DXGI_FORMAT_BC1_UNORM: return Format::BC1_UNORM;
            case DXGI_FORMAT_BC1_UNORM_SRGB: return Format::BC1_UNORM_SRGB;
            case DXGI_FORMAT_BC2_TYPELESS: return Format::BC2_TYPELESS;
            case DXGI_FORMAT_BC2_UNORM: return Format::BC2_UNORM;
            case DXGI_FORMAT_BC2_UNORM_SRGB: return Format::BC2_UNORM_SRGB;
            case DXGI_FORMAT_BC3_TYPELESS: return Format::BC3_TYPELESS;
            case DXGI_FORMAT_BC3_UNORM: return Format::BC3_UNORM;
            case DXGI_FORMAT_BC3_UNORM_SRGB: return Format::BC3_UNORM_SRGB;
            case DXGI_FORMAT_BC4_TYPELESS: return Format::BC4_TYPELESS;
            case DXGI_FORMAT_BC4_UNORM: return Format::BC4_UNORM;
            case DXGI_FORMAT_BC4_SNORM: return Format::BC4_SNORM;
            case DXGI_FORMAT_BC5_TYPELESS: return Format::BC5_TYPELESS;
            case DXGI_FORMAT_BC5_UNORM: return Format::BC5_UNORM;
            case DXGI_FORMAT_BC5_SNORM: return Format::BC5_SNORM;
            case DXGI_FORMAT_B5G6R5_UNORM: return Format::B5G6R5_UNORM;
            case DXGI_FORMAT_B5G5R5A1_UNORM: return Format::B5G5R5A1_UNORM;
            case DXGI_FORMAT_B8G8R8A8_UNORM: return Format::B8G8R8A8_UNORM;
            case DXGI_FORMAT_B8G8R8X8_UNORM: return Format::B8G8R8X8_UNORM;
            case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return Format::R10G10B10_XR_BIAS_A2_UNORM;
            case DXGI_FORMAT_B8G8R8A8_TYPELESS: return Format::B8G8R8A8_TYPELESS;
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return Format::B8G8R8A8_UNORM_SRGB;
            case DXGI_FORMAT_B8G8R8X8_TYPELESS: return Format::B8G8R8X8_TYPELESS;
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB: return Format::B8G8R8X8_UNORM_SRGB;
            case DXGI_FORMAT_BC6H_TYPELESS: return Format::BC6H_TYPELESS;
            case DXGI_FORMAT_BC6H_UF16: return Format::BC6H_UF16;
            case DXGI_FORMAT_BC6H_SF16: return Format::BC6H_SF16;
            case DXGI_FORMAT_BC7_TYPELESS: return Format::BC7_TYPELESS;
            case DXGI_FORMAT_BC7_UNORM: return Format::BC7_UNORM;
            case DXGI_FORMAT_BC7_UNORM_SRGB: return Format::BC7_UNORM_SRGB;
            case DXGI_FORMAT_AYUV: return Format::AYUV;
            case DXGI_FORMAT_Y410: return Format::Y410;
            case DXGI_FORMAT_Y416: return Format::Y416;
            case DXGI_FORMAT_NV12: return Format::NV12;
            case DXGI_FORMAT_P010: return Format::P010;
            case DXGI_FORMAT_P016: return Format::P016;
            case DXGI_FORMAT_420_OPAQUE: return Format::OPAQUE_420;
            case DXGI_FORMAT_YUY2: return Format::YUY2;
            case DXGI_FORMAT_Y210: return Format::Y210;
            case DXGI_FORMAT_Y216: return Format::Y216;
            case DXGI_FORMAT_NV11: return Format::NV11;
            case DXGI_FORMAT_AI44: return Format::AI44;
            case DXGI_FORMAT_IA44: return Format::IA44;
            case DXGI_FORMAT_P8: return Format::P8;
            case DXGI_FORMAT_A8P8: return Format::A8P8;
            case DXGI_FORMAT_B4G4R4A4_UNORM: return Format::B4G4R4A4_UNORM;
            case DXGI_FORMAT_P208: return Format::P208;
            case DXGI_FORMAT_V208: return Format::V208;
            case DXGI_FORMAT_V408: return Format::V408;
            case DXGI_FORMAT_FORCE_UINT: return Format::R8G8B8A8_UNORM;
            default: return Format::R8G8B8A8_UNORM;
            }
        }

        ResourceDimension fromDXResourceDimension(D3D12_RESOURCE_DIMENSION dim)
        {
            switch (dim)
            {
                case D3D12_RESOURCE_DIMENSION_UNKNOWN: return ResourceDimension::Unknown;
                case D3D12_RESOURCE_DIMENSION_TEXTURE1D: return ResourceDimension::Texture1D;
                case D3D12_RESOURCE_DIMENSION_TEXTURE2D: return ResourceDimension::Texture2D;
                case D3D12_RESOURCE_DIMENSION_TEXTURE3D: return ResourceDimension::Texture3D;
            }
            return ResourceDimension::Unknown;
        }

        SampleDescription fromDXSampleDescription(DXGI_SAMPLE_DESC desc)
        {
            return SampleDescription{
                static_cast<unsigned int>(desc.Count),
                static_cast<unsigned int>(desc.Quality) };
        }

        TextureLayout fromDXTextureLayout(D3D12_TEXTURE_LAYOUT layout)
        {
            switch (layout)
            {
                case D3D12_TEXTURE_LAYOUT_UNKNOWN: return TextureLayout::Unknown;
                case D3D12_TEXTURE_LAYOUT_ROW_MAJOR: return TextureLayout::RowMajor;
                case D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE: return TextureLayout::UndefinedSwizzle64KB;
                case D3D12_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE: return TextureLayout::StandardSwizzle64KB;
            }
            return TextureLayout::Unknown;
        }

        ResourceFlags fromDXResourceFlags(D3D12_RESOURCE_FLAGS flags)
        {
            switch (flags)
            {
                case D3D12_RESOURCE_FLAG_NONE: return ResourceFlags::None;
                case D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET: return ResourceFlags::AllowRenderTarget;
                case D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL: return ResourceFlags::AllowDepthStencil;
                case D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS: return ResourceFlags::AllowUnorderedAccess;
                case D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE: return ResourceFlags::DenyShaderResource;
                case D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER: return ResourceFlags::AllowCrossAdapter;
                case D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS: return ResourceFlags::AllowSimultaneousAccess;
            }
            return ResourceFlags::None;
        }

        DepthStencilValue fromDXDepthStencilValue(D3D12_DEPTH_STENCIL_VALUE val)
        {
            return DepthStencilValue{
                static_cast<float>(val.Depth),
                static_cast<unsigned char>(val.Stencil) };
        }

        Viewport fromDXViewport(D3D12_VIEWPORT viewPort)
        {
            return Viewport{
                static_cast<float>(viewPort.TopLeftX),
                static_cast<float>(viewPort.TopLeftY),
                static_cast<float>(viewPort.Width),
                static_cast<float>(viewPort.Height),
                static_cast<float>(viewPort.MinDepth),
                static_cast<float>(viewPort.MaxDepth) };
        }

        Rectangle fromDXRect(D3D12_RECT rect)
        {
            return Rectangle{
                static_cast<int>(rect.left),
                static_cast<int>(rect.top),
                static_cast<int>(rect.right),
                static_cast<int>(rect.bottom)
            };
        }

        Blend fromDXBlend(D3D12_BLEND blend)
        {
            switch (blend)
            {
                case D3D12_BLEND_ZERO: return Blend::Zero;
                case D3D12_BLEND_ONE: return Blend::One;
                case D3D12_BLEND_SRC_COLOR: return Blend::SrcColor;
                case D3D12_BLEND_INV_SRC_COLOR: return Blend::InvSrcColor;
                case D3D12_BLEND_SRC_ALPHA: return Blend::SrcAlpha;
                case D3D12_BLEND_INV_SRC_ALPHA: return Blend::InvSrcAlpha;
                case D3D12_BLEND_DEST_ALPHA: return Blend::DestAlpha;
                case D3D12_BLEND_INV_DEST_ALPHA: return Blend::InvDestAlpha;
                case D3D12_BLEND_DEST_COLOR: return Blend::DestColor;
                case D3D12_BLEND_INV_DEST_COLOR: return Blend::InvDestColor;
                case D3D12_BLEND_SRC_ALPHA_SAT: return Blend::SrcAlphaSaturate;
                case D3D12_BLEND_BLEND_FACTOR: return Blend::BlendFactor;
                case D3D12_BLEND_INV_BLEND_FACTOR: return Blend::InvBlendFactor;
                case D3D12_BLEND_SRC1_COLOR: return Blend::Src1Color;
                case D3D12_BLEND_INV_SRC1_COLOR: return Blend::InvSrc1Color;
                case D3D12_BLEND_SRC1_ALPHA: return Blend::Src1Alpha;
                case D3D12_BLEND_INV_SRC1_ALPHA: return Blend::InvSrc1Alpha;
            }
            return Blend::Zero;
        }

        BlendOperation fromDXBlendOp(D3D12_BLEND_OP op)
        {
            switch (op)
            {
                case D3D12_BLEND_OP_ADD: return BlendOperation::Add;
                case D3D12_BLEND_OP_SUBTRACT: return BlendOperation::Subtract;
                case D3D12_BLEND_OP_REV_SUBTRACT: return BlendOperation::RevSubtract;
                case D3D12_BLEND_OP_MIN: return BlendOperation::Min;
                case D3D12_BLEND_OP_MAX: return BlendOperation::Max;
            }
            return BlendOperation::Add;
        }

        LogicOperation fromDXLogicOp(D3D12_LOGIC_OP op)
        {
            switch (op)
            {
                case D3D12_LOGIC_OP_CLEAR: return LogicOperation::Clear;
                case D3D12_LOGIC_OP_SET: return LogicOperation::Set;
                case D3D12_LOGIC_OP_COPY: return LogicOperation::Copy;
                case D3D12_LOGIC_OP_COPY_INVERTED: return LogicOperation::CopyInverted;
                case D3D12_LOGIC_OP_NOOP: return LogicOperation::Noop;
                case D3D12_LOGIC_OP_INVERT: return LogicOperation::Invert;
                case D3D12_LOGIC_OP_AND: return LogicOperation::And;
                case D3D12_LOGIC_OP_NAND: return LogicOperation::Nand;
                case D3D12_LOGIC_OP_OR: return LogicOperation::Or;
                case D3D12_LOGIC_OP_NOR: return LogicOperation::Nor;
                case D3D12_LOGIC_OP_XOR: return LogicOperation::Xor;
                case D3D12_LOGIC_OP_EQUIV: return LogicOperation::Equiv;
                case D3D12_LOGIC_OP_AND_REVERSE: return LogicOperation::AndReverse;
                case D3D12_LOGIC_OP_AND_INVERTED: return LogicOperation::AndInverted;
                case D3D12_LOGIC_OP_OR_REVERSE: return LogicOperation::OrReverse;
                case D3D12_LOGIC_OP_OR_INVERTED: return LogicOperation::OrInverted;
            }
            return LogicOperation::Clear;
        }

        RenderTargetBlendDescription fromDXRenderTargetBlendDesc(const D3D12_RENDER_TARGET_BLEND_DESC& desc)
        {
            return RenderTargetBlendDescription{
                static_cast<bool>(desc.BlendEnable == TRUE),
                static_cast<bool>(desc.LogicOpEnable == TRUE),
                fromDXBlend(desc.SrcBlend),
                fromDXBlend(desc.DestBlend),
                fromDXBlendOp(desc.BlendOp),
                fromDXBlend(desc.SrcBlendAlpha),
                fromDXBlend(desc.DestBlendAlpha),
                fromDXBlendOp(desc.BlendOpAlpha),
                fromDXLogicOp(desc.LogicOp),
                static_cast<unsigned char>(desc.RenderTargetWriteMask)
            };
        }

        BlendDescription fromDXBlendDesc(const D3D12_BLEND_DESC& desc)
        {
            return BlendDescription()
                .alphaToCoverageEnable(desc.AlphaToCoverageEnable == TRUE)
                .independentBlendEnable(desc.IndependentBlendEnable == TRUE)
                .renderTarget(0, fromDXRenderTargetBlendDesc(desc.RenderTarget[0]))
                .renderTarget(1, fromDXRenderTargetBlendDesc(desc.RenderTarget[0]))
                .renderTarget(2, fromDXRenderTargetBlendDesc(desc.RenderTarget[0]))
                .renderTarget(3, fromDXRenderTargetBlendDesc(desc.RenderTarget[0]))
                .renderTarget(4, fromDXRenderTargetBlendDesc(desc.RenderTarget[0]))
                .renderTarget(5, fromDXRenderTargetBlendDesc(desc.RenderTarget[0]))
                .renderTarget(6, fromDXRenderTargetBlendDesc(desc.RenderTarget[0]))
                .renderTarget(7, fromDXRenderTargetBlendDesc(desc.RenderTarget[0]));
        }

        FillMode fromDXFillMode(D3D12_FILL_MODE mode)
        {
            switch (mode)
            {
                case D3D12_FILL_MODE_WIREFRAME: return FillMode::Wireframe;
                case D3D12_FILL_MODE_SOLID: return FillMode::Solid;
            }
            return FillMode::Wireframe;
        }

        CullMode fromDXCullMode(D3D12_CULL_MODE mode)
        {
            switch (mode)
            {
                case D3D12_CULL_MODE_NONE: return CullMode::None;
                case D3D12_CULL_MODE_FRONT: return CullMode::Front;
                case D3D12_CULL_MODE_BACK: return CullMode::Back;
            }
            return CullMode::None;
        }

        ConservativeRasterizationMode fromDXConservativeRasterizationMode(D3D12_CONSERVATIVE_RASTERIZATION_MODE mode)
        {
            switch (mode)
            {
                case D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF: return ConservativeRasterizationMode::Off;
                case D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON: return ConservativeRasterizationMode::On;
            }
            return ConservativeRasterizationMode::Off;
        }

        RasterizerDescription fromDXRasterizerDesc(const D3D12_RASTERIZER_DESC& desc)
        {
            return RasterizerDescription()
                .fillMode(fromDXFillMode(desc.FillMode))
                .cullMode(fromDXCullMode(desc.CullMode))
                .frontCounterClockwise(desc.FrontCounterClockwise == TRUE)
                .depthBias(static_cast<int>(desc.DepthBias))
                .depthBiasClamp(static_cast<float>(desc.DepthBiasClamp))
                .slopeScaledDepthBias(static_cast<float>(desc.SlopeScaledDepthBias))
                .depthClipEnable(desc.DepthClipEnable == TRUE)
                .multisampleEnable(desc.MultisampleEnable == TRUE)
                .antialiasedLineEnable(desc.AntialiasedLineEnable == TRUE)
                .forcedSampleCount(static_cast<unsigned int>(desc.ForcedSampleCount))
                .conservativeRaster(fromDXConservativeRasterizationMode(desc.ConservativeRaster));
        }

        DepthWriteMask fromDXDepthWriteMask(D3D12_DEPTH_WRITE_MASK mask)
        {
            switch (mask)
            {
                case D3D12_DEPTH_WRITE_MASK_ZERO: return DepthWriteMask::Zero;
                case D3D12_DEPTH_WRITE_MASK_ALL: return DepthWriteMask::All;
            }
            return DepthWriteMask::Zero;
        }

        StencilOp fromDXStencilOp(D3D12_STENCIL_OP op)
        {
            switch (op)
            {
                case D3D12_STENCIL_OP_KEEP: return StencilOp::Keep;
                case D3D12_STENCIL_OP_ZERO: return StencilOp::Zero;
                case D3D12_STENCIL_OP_REPLACE: return StencilOp::Replace;
                case D3D12_STENCIL_OP_INCR_SAT: return StencilOp::IncrSat;
                case D3D12_STENCIL_OP_DECR_SAT: return StencilOp::DecrSat;
                case D3D12_STENCIL_OP_INVERT: return StencilOp::Invert;
                case D3D12_STENCIL_OP_INCR: return StencilOp::Incr;
                case D3D12_STENCIL_OP_DECR: return StencilOp::Decr;
            }
            return StencilOp::Keep;
        }

        DepthStencilOpDescription fromDXDepthStencilOpDesc(const D3D12_DEPTH_STENCILOP_DESC& desc)
        {
            return DepthStencilOpDescription{
                fromDXStencilOp(desc.StencilFailOp),
                fromDXStencilOp(desc.StencilDepthFailOp),
                fromDXStencilOp(desc.StencilPassOp),
                fromDXComparisonFunc(desc.StencilFunc)
            };
        }

        DepthStencilDescription fromDXDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC& desc)
        {
            return DepthStencilDescription()
                .depthEnable(desc.DepthEnable == TRUE)
                .depthWriteMask(fromDXDepthWriteMask(desc.DepthWriteMask))
                .depthFunc(fromDXComparisonFunc(desc.DepthFunc))
                .stencilEnable(desc.StencilEnable == TRUE)
                .stencilReadMask(static_cast<unsigned char>(desc.StencilReadMask))
                .stencilWriteMask(static_cast<unsigned char>(desc.StencilWriteMask))
                .frontFace(fromDXDepthStencilOpDesc(desc.FrontFace))
                .backFace(fromDXDepthStencilOpDesc(desc.BackFace));
        }

        PrimitiveTopologyType fromDXPrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY type)
        {
            switch (type)
            {
            case D3D_PRIMITIVE_TOPOLOGY_UNDEFINED: return PrimitiveTopologyType::Undefined;
            case D3D_PRIMITIVE_TOPOLOGY_POINTLIST: return PrimitiveTopologyType::PointList;
            case D3D_PRIMITIVE_TOPOLOGY_LINELIST: return PrimitiveTopologyType::LineList;
            case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP: return PrimitiveTopologyType::LineStrip;
            case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST: return PrimitiveTopologyType::TriangleList;
            case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: return PrimitiveTopologyType::TriangleStrip;
            case D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList1;
            case D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList2;
            case D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList3;
            case D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList4;
            case D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList5;
            case D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList6;
            case D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList7;
            case D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList8;
            case D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList9;
            case D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList10;
            case D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList11;
            case D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList12;
            case D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList13;
            case D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList14;
            case D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList15;
            case D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST: return PrimitiveTopologyType::PatchList16;
            }
            return PrimitiveTopologyType::Undefined;
        }

        InputClassification fromDXInputClassification(D3D12_INPUT_CLASSIFICATION cls)
        {
            switch (cls)
            {
                case D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA: return InputClassification::PerVertexData;
                case D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA: return InputClassification::PerInstanceData;
            }
            return InputClassification::PerVertexData;
        }

        InputElementDescription fromDXInputElementDesc(const D3D12_INPUT_ELEMENT_DESC& desc)
        {
            return InputElementDescription{
                static_cast<const char*>(desc.SemanticName),
                static_cast<unsigned int>(desc.SemanticIndex),
                fromDXFormat(desc.Format),
                static_cast<unsigned int>(desc.InputSlot),
                static_cast<unsigned int>(desc.AlignedByteOffset),
                fromDXInputClassification(desc.InputSlotClass),
                static_cast<unsigned int>(desc.InstanceDataStepRate),
                0 // offset
            };
        }

        IndexBufferStripCutValue fromDXIndexBufferStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE value)
        {
            switch (value)
            {
                case D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED: return IndexBufferStripCutValue::ValueDisabled;
                case D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF: return IndexBufferStripCutValue::Value0xFFFF;
                case D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF: return IndexBufferStripCutValue::Value0xFFFFFFFF;
            }
            return IndexBufferStripCutValue::ValueDisabled;
        }

        CommandListType fromDXCommandListType(D3D12_COMMAND_LIST_TYPE type)
        {
            switch (type)
            {
                case D3D12_COMMAND_LIST_TYPE_DIRECT: return CommandListType::Direct;
                case D3D12_COMMAND_LIST_TYPE_BUNDLE: return CommandListType::Bundle;
                case D3D12_COMMAND_LIST_TYPE_COMPUTE: return CommandListType::Compute;
                case D3D12_COMMAND_LIST_TYPE_COPY: return CommandListType::Copy;
            }
            return CommandListType::Direct;
        }

        PredicationOp fromDXPredicationOp(D3D12_PREDICATION_OP op)
        {
            switch (op)
            {
                case D3D12_PREDICATION_OP_EQUAL_ZERO: return PredicationOp::EqualZero;
                case D3D12_PREDICATION_OP_NOT_EQUAL_ZERO: return PredicationOp::NotEqualZero;
            }
            return PredicationOp::EqualZero;
        }
    }
}
