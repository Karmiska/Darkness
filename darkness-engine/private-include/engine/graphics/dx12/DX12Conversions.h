#pragma once

#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/CommonNoDep.h"

namespace engine
{
    enum class Filter;
    enum class TextureAddressMode;
    enum class ComparisonFunction;
    enum class StaticBorderColor;
    enum class DescriptorRangeType;
    enum class RootSignatureFlags;
    enum class Format;
    enum class ResourceBarrierFlags;
    enum class ResourceState;
    enum class Blend;
    enum class BlendOperation;
    enum class LogicOperation;
    enum class FillMode;
    enum class CullMode;
    enum class ConservativeRasterizationMode;
    enum class DepthWriteMask;
    enum class StencilOp;
    enum class PrimitiveTopologyType;
    enum class InputClassification;
    enum class IndexBufferStripCutValue;
    enum class ResourceDimension;
    enum class TextureLayout;
    enum class ResourceFlags;
    enum class ColorWriteEnable;
    enum class CommandListType;
    enum class PredicationOp;
    
    struct StaticSamplerDescription;
    struct ResourceDescription;
    struct Viewport;
    struct Rectangle;
    struct RenderTargetBlendDescription;
    struct BlendDescription;
    struct RasterizerDescription;
    struct DepthStencilOpDescription;
    struct DepthStencilDescription;
    struct InputElementDescription;
    struct SampleDescription;
    struct DepthStencilValue;

    namespace implementation
    {
        D3D12_SHADER_VISIBILITY dxShaderVisibility(ShaderVisibility visibility);
        D3D12_DESCRIPTOR_RANGE_TYPE dxRangeType(DescriptorRangeType type);
        D3D12_FILTER dxFilter(const Filter& filter);
        D3D12_TEXTURE_ADDRESS_MODE dxTextureAddressMode(TextureAddressMode mode);
        D3D12_COMPARISON_FUNC dxComparisonFunc(ComparisonFunction comp);
        D3D12_STATIC_BORDER_COLOR dxStaticBorderColor(StaticBorderColor color);
        D3D12_STATIC_SAMPLER_DESC dxStaticSamplerDesc(const StaticSamplerDescription& desc);
        D3D12_ROOT_SIGNATURE_FLAGS dxSignatureFlags(RootSignatureFlags flags);
        D3D12_RESOURCE_BARRIER_FLAGS dxFlags(ResourceBarrierFlags flags);
        D3D12_RESOURCE_STATES dxResourceStates(ResourceState state);
        DXGI_FORMAT dxFormat(Format format);
        DXGI_FORMAT dxTypelessFormat(Format format);
        D3D12_VIEWPORT dxViewport(Viewport viewPort);
        D3D12_RECT dxRect(Rectangle rect);
        D3D12_RESOURCE_DIMENSION dxResourceDimension(ResourceDimension dim);
        DXGI_SAMPLE_DESC dxSampleDescription(SampleDescription desc);
        D3D12_TEXTURE_LAYOUT dxTextureLayout(TextureLayout layout);
        D3D12_RESOURCE_FLAGS dxResourceFlags(ResourceFlags flags);
        D3D12_DEPTH_STENCIL_VALUE dxDepthStencilValue(DepthStencilValue val);
        D3D12_BLEND dxBlend(Blend blend);
        D3D12_BLEND_OP dxBlendOp(BlendOperation op);
        D3D12_LOGIC_OP dxLogicOp(LogicOperation op);
        D3D12_RENDER_TARGET_BLEND_DESC dxRenderTargetBlendDesc(const RenderTargetBlendDescription& desc);
        D3D12_BLEND_DESC dxBlendDesc(const BlendDescription& desc);
        D3D12_FILL_MODE dxFillMode(FillMode mode);
        D3D12_CULL_MODE dxCullMode(CullMode mode);
        D3D12_CONSERVATIVE_RASTERIZATION_MODE dxConservativeRasterizationMode(ConservativeRasterizationMode mode);
        D3D12_RASTERIZER_DESC dxRasterizerDesc(const RasterizerDescription& desc);
        D3D12_DEPTH_WRITE_MASK dxDepthWriteMask(DepthWriteMask mask);
        D3D12_STENCIL_OP dxStencilOp(StencilOp op);
        D3D12_DEPTH_STENCILOP_DESC dxDepthStencilOpDesc(const DepthStencilOpDescription& desc);
        D3D12_DEPTH_STENCIL_DESC dxDepthStencilDesc(const DepthStencilDescription& desc);
        D3D_PRIMITIVE_TOPOLOGY dxPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency = false);
        D3D12_PRIMITIVE_TOPOLOGY_TYPE dxPrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY type);

        D3D12_INPUT_CLASSIFICATION dxInputClassification(InputClassification cls);
        D3D12_INPUT_ELEMENT_DESC dxInputElementDesc(const InputElementDescription& desc);
        D3D12_INDEX_BUFFER_STRIP_CUT_VALUE dxIndexBufferStripCutValue(IndexBufferStripCutValue value);
        D3D12_COLOR_WRITE_ENABLE dxColorWriteEnable(ColorWriteEnable val);
        D3D12_COMMAND_LIST_TYPE dxCommandListType(CommandListType type);
        D3D12_PREDICATION_OP dxPredicationOp(PredicationOp op);

        ShaderVisibility fromDXShaderVisibility(D3D12_SHADER_VISIBILITY visibility);
        DescriptorRangeType fromDXRangeType(D3D12_DESCRIPTOR_RANGE_TYPE type);
        Filter fromDXFilter(D3D12_FILTER filter);
        TextureAddressMode fromDXTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE mode);
        ComparisonFunction fromDXComparisonFunc(D3D12_COMPARISON_FUNC comp);
        StaticBorderColor fromDXStaticBorderColor(D3D12_STATIC_BORDER_COLOR color);
        StaticSamplerDescription fromDXStaticSamplerDesc(const D3D12_STATIC_SAMPLER_DESC& desc);
        RootSignatureFlags fromDXSignatureFlags(D3D12_ROOT_SIGNATURE_FLAGS flags);
        ResourceBarrierFlags fromDXFlags(D3D12_RESOURCE_BARRIER_FLAGS flags);
        ResourceState fromDXResourceStates(D3D12_RESOURCE_STATES state);
        Format fromDXFormat(DXGI_FORMAT format);
        Viewport fromDXViewport(D3D12_VIEWPORT viewPort);
        Rectangle fromDXRect(D3D12_RECT rect);
        ResourceDimension fromDXResourceDimension(D3D12_RESOURCE_DIMENSION dim);
        SampleDescription fromDXSampleDescription(DXGI_SAMPLE_DESC desc);
        TextureLayout fromDXTextureLayout(D3D12_TEXTURE_LAYOUT layout);
        ResourceFlags fromDXResourceFlags(D3D12_RESOURCE_FLAGS flags);
        DepthStencilValue fromDXDepthStencilValue(D3D12_DEPTH_STENCIL_VALUE val);
        ResourceDescription fromDXResourceDesc(D3D12_RESOURCE_DESC);
        Blend fromDXBlend(D3D12_BLEND blend);
        BlendOperation fromDXBlendOp(D3D12_BLEND_OP op);
        LogicOperation fromDXLogicOp(D3D12_LOGIC_OP op);
        RenderTargetBlendDescription fromDXRenderTargetBlendDesc(const D3D12_RENDER_TARGET_BLEND_DESC& desc);
        BlendDescription fromDXBlendDesc(const D3D12_BLEND_DESC& desc);
        FillMode fromDXFillMode(D3D12_FILL_MODE mode);
        CullMode fromDXCullMode(D3D12_CULL_MODE mode);
        ConservativeRasterizationMode fromDXConservativeRasterizationMode(D3D12_CONSERVATIVE_RASTERIZATION_MODE mode);
        RasterizerDescription fromDXRasterizerDesc(const D3D12_RASTERIZER_DESC& desc);
        DepthWriteMask fromDXDepthWriteMask(D3D12_DEPTH_WRITE_MASK mask);
        StencilOp fromDXStencilOp(D3D12_STENCIL_OP op);
        DepthStencilOpDescription fromDXDepthStencilOpDesc(const D3D12_DEPTH_STENCILOP_DESC& desc);
        DepthStencilDescription fromDXDepthStencilDesc(const D3D12_DEPTH_STENCIL_DESC& desc);
        PrimitiveTopologyType fromDXPrimitiveTopologyType(D3D_PRIMITIVE_TOPOLOGY type);
        InputClassification fromDXInputClassification(D3D12_INPUT_CLASSIFICATION cls);
        InputElementDescription fromDXInputElementDesc(const D3D12_INPUT_ELEMENT_DESC& desc);
        IndexBufferStripCutValue fromDXIndexBufferStripCutValue(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE value);
        ColorWriteEnable fromDXColorWriteEnable(D3D12_COLOR_WRITE_ENABLE val);
        CommandListType fromDXCommandListType(D3D12_COMMAND_LIST_TYPE type);
        PredicationOp fromDXPredicationOp(D3D12_PREDICATION_OP op);
    }
}
