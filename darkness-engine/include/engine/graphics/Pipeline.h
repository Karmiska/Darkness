#pragma once

#include "Common.h"
#include "engine/graphics/Format.h"
#include "containers/memory.h"
#include "containers/vector.h"

namespace engine
{
    namespace shaders
    {
        class PipelineConfiguration;
    }

    class RootSignature;
    class ShaderBinary;
    class ShaderStorage;
    class Device;
    class CommandList;
    class TextureSRV;
    class TextureDSV;
    class TextureRTV;
    class Sampler;
    class Buffer;
    enum class Format;
    enum class GraphicsApi;

    enum class Blend
    {
        Zero,
        One,
        SrcColor,
        InvSrcColor,
        SrcAlpha,
        InvSrcAlpha,
        DestAlpha,
        InvDestAlpha,
        DestColor,
        InvDestColor,
        SrcAlphaSaturate,
        BlendFactor,
        InvBlendFactor,
        Src1Color,
        InvSrc1Color,
        Src1Alpha,
        InvSrc1Alpha
    };

    enum class BlendOperation
    {
        Add,
        Subtract,
        RevSubtract,
        Min,
        Max
    };

    enum class LogicOperation
    {
        Clear,
        Set,
        Copy,
        CopyInverted,
        Noop,
        Invert,
        And,
        Nand,
        Or,
        Nor,
        Xor,
        Equiv,
        AndReverse,
        AndInverted,
        OrReverse,
        OrInverted
    };

    enum class ColorWriteEnable
    {
        None = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = (((Red | Green) | Blue) | Alpha)
    };

    struct RenderTargetBlendDescription
    {
        struct
        {
            bool blendEnable = false;
            bool logicOpEnable = false;
            Blend srcBlend = Blend::One;
            Blend dstBlend = Blend::One;
            BlendOperation blendOp = BlendOperation::Add;
            Blend srcBlendAlpha = Blend::One;
            Blend dstBlendAlpha = Blend::One;
            BlendOperation blendOpAlpha = BlendOperation::Add;
            LogicOperation logicOp = LogicOperation::Noop;
            unsigned char renderTargetWriteMask = 1 | 2 | 4 | 8;
        } desc;
        RenderTargetBlendDescription& blendEnable(bool blendEnable) { desc.blendEnable = blendEnable; return *this; }
        RenderTargetBlendDescription& logicOpEnable(bool logicOpEnable) { desc.logicOpEnable = logicOpEnable; return *this; }
        RenderTargetBlendDescription& srcBlend(Blend srcBlend) { desc.srcBlend = srcBlend; return *this; }
        RenderTargetBlendDescription& dstBlend(Blend dstBlend) { desc.dstBlend = dstBlend; return *this; }
        RenderTargetBlendDescription& blendOp(BlendOperation blendOp) { desc.blendOp = blendOp; return *this; }
        RenderTargetBlendDescription& srcBlendAlpha(Blend srcBlendAlpha) { desc.srcBlendAlpha = srcBlendAlpha; return *this; }
        RenderTargetBlendDescription& dstBlendAlpha(Blend dstBlendAlpha) { desc.dstBlendAlpha = dstBlendAlpha; return *this; }
        RenderTargetBlendDescription& blendOpAlpha(BlendOperation blendOpAlpha) { desc.blendOpAlpha = blendOpAlpha; return *this; }
        RenderTargetBlendDescription& logicOp(LogicOperation logicOp) { desc.logicOp = logicOp; return *this; }
        RenderTargetBlendDescription& renderTargetWriteMask(unsigned char renderTargetWriteMask) { desc.renderTargetWriteMask = renderTargetWriteMask; return *this; }

    };

    struct BlendDescription
    {
        struct
        {
            bool alphaToCoverageEnable = false;
            bool independentBlendEnable = false;
            RenderTargetBlendDescription renderTarget[8]
            {
                RenderTargetBlendDescription{ false, false, Blend::SrcAlpha, Blend::InvSrcAlpha, BlendOperation::Add, Blend::One, Blend::InvSrcAlpha, BlendOperation::Add, LogicOperation::Clear, 1 | 2 | 4 | 8 },
                RenderTargetBlendDescription{ false, false, Blend::SrcAlpha, Blend::InvSrcAlpha, BlendOperation::Add, Blend::One, Blend::InvSrcAlpha, BlendOperation::Add, LogicOperation::Clear, 1 | 2 | 4 | 8 },
                RenderTargetBlendDescription{ false, false, Blend::SrcAlpha, Blend::InvSrcAlpha, BlendOperation::Add, Blend::One, Blend::InvSrcAlpha, BlendOperation::Add, LogicOperation::Clear, 1 | 2 | 4 | 8 },
                RenderTargetBlendDescription{ false, false, Blend::SrcAlpha, Blend::InvSrcAlpha, BlendOperation::Add, Blend::One, Blend::InvSrcAlpha, BlendOperation::Add, LogicOperation::Clear, 1 | 2 | 4 | 8 },
                RenderTargetBlendDescription{ false, false, Blend::SrcAlpha, Blend::InvSrcAlpha, BlendOperation::Add, Blend::One, Blend::InvSrcAlpha, BlendOperation::Add, LogicOperation::Clear, 1 | 2 | 4 | 8 },
                RenderTargetBlendDescription{ false, false, Blend::SrcAlpha, Blend::InvSrcAlpha, BlendOperation::Add, Blend::One, Blend::InvSrcAlpha, BlendOperation::Add, LogicOperation::Clear, 1 | 2 | 4 | 8 },
                RenderTargetBlendDescription{ false, false, Blend::SrcAlpha, Blend::InvSrcAlpha, BlendOperation::Add, Blend::One, Blend::InvSrcAlpha, BlendOperation::Add, LogicOperation::Clear, 1 | 2 | 4 | 8 },
                RenderTargetBlendDescription{ false, false, Blend::SrcAlpha, Blend::InvSrcAlpha, BlendOperation::Add, Blend::One, Blend::InvSrcAlpha, BlendOperation::Add, LogicOperation::Clear, 1 | 2 | 4 | 8 }
            };
        } desc;
        
        BlendDescription& alphaToCoverageEnable(bool alphaToCoverageEnable) { desc.alphaToCoverageEnable = alphaToCoverageEnable; return *this; }
        BlendDescription& independentBlendEnable(bool independentBlendEnable) { desc.independentBlendEnable = independentBlendEnable; return *this; }
        BlendDescription& renderTarget(int target, const RenderTargetBlendDescription& trgDesc) { desc.renderTarget[target] = trgDesc; return *this; }
    };

    enum class FillMode
    {
        Wireframe,
        Solid,
        Point
    };

    enum class CullMode
    {
        None,
        Front,
        Back
    };

    enum class ConservativeRasterizationMode
    {
        Off,
        On
    };

    struct RasterizerDescription
    {
        struct
        {
            FillMode fillMode = FillMode::Solid;
            CullMode cullMode = CullMode::Back;
            bool frontCounterClockwise = true;
            int depthBias = 0;
            float depthBiasClamp = 0.0f;
            float slopeScaledDepthBias = 0.0f;
            bool depthClipEnable = false;
            bool multisampleEnable = false;
            bool antialiasedLineEnable = false;
            unsigned int forcedSampleCount = 0;
            ConservativeRasterizationMode conservativeRaster = ConservativeRasterizationMode::Off;
        } desc;

        RasterizerDescription& fillMode(FillMode mode) { desc.fillMode = mode; return *this; }
        RasterizerDescription& cullMode(CullMode mode) { desc.cullMode = mode; return *this; }
        RasterizerDescription& frontCounterClockwise(bool frontCounterClockwise) { desc.frontCounterClockwise = frontCounterClockwise; return *this; }
        RasterizerDescription& depthBias(int depthBias) { desc.depthBias = depthBias; return *this; }
        RasterizerDescription& depthBiasClamp(float depthBiasClamp) { desc.depthBiasClamp = depthBiasClamp; return *this; }
        RasterizerDescription& slopeScaledDepthBias(float slopeScaledDepthBias) { desc.slopeScaledDepthBias = slopeScaledDepthBias; return *this; }
        RasterizerDescription& depthClipEnable(bool depthClipEnable) { desc.depthClipEnable = depthClipEnable; return *this; }
        RasterizerDescription& multisampleEnable(bool multisampleEnable) { desc.multisampleEnable = multisampleEnable; return *this; }
        RasterizerDescription& antialiasedLineEnable(bool antialiasedLineEnable) { desc.antialiasedLineEnable = antialiasedLineEnable; return *this; }
        RasterizerDescription& forcedSampleCount(unsigned int forcedSampleCount) { desc.forcedSampleCount = forcedSampleCount; return *this; }
        RasterizerDescription& conservativeRaster(ConservativeRasterizationMode conservativeRaster) { desc.conservativeRaster = conservativeRaster; return *this; }
    };

    enum class DepthWriteMask
    {
        Zero,
        All
    };

    enum class StencilOp
    {
        Keep,
        Zero,
        Replace,
        IncrSat,
        DecrSat,
        Invert,
        Incr,
        Decr
    };

    struct DepthStencilOpDescription
    {
        StencilOp StencilFailOp;
        StencilOp StencilDepthFailOp;
        StencilOp StencilPassOp;
        ComparisonFunction StencilFunc;
    };

    struct DepthStencilDescription
    {
        struct
        {
            bool depthEnable = true;
            DepthWriteMask depthWriteMask = DepthWriteMask::All;
            ComparisonFunction depthFunc = ComparisonFunction::GreaterEqual;
            bool stencilEnable = false;
            unsigned char stencilReadMask = 0xff;
            unsigned char stencilWriteMask = 0xff;
            DepthStencilOpDescription frontFace = { StencilOp::Keep, StencilOp::Incr, StencilOp::Keep, ComparisonFunction::Always };
            DepthStencilOpDescription backFace = { StencilOp::Keep, StencilOp::Incr, StencilOp::Keep, ComparisonFunction::Always };
        } desc;

        DepthStencilDescription& depthEnable(bool depthEnable) { desc.depthEnable = depthEnable; return *this; }
        DepthStencilDescription& depthWriteMask(DepthWriteMask depthWriteMask) { desc.depthWriteMask = depthWriteMask; return *this; }
        DepthStencilDescription& depthFunc(ComparisonFunction depthFunc) { desc.depthFunc = depthFunc; return *this; }
        DepthStencilDescription& stencilEnable(bool stencilEnable) { desc.stencilEnable = stencilEnable; return *this; }
        DepthStencilDescription& stencilReadMask(unsigned char stencilReadMask) { desc.stencilReadMask = stencilReadMask; return *this; }
        DepthStencilDescription& stencilWriteMask(unsigned char stencilWriteMask) { desc.stencilWriteMask = stencilWriteMask; return *this; }
        DepthStencilDescription& frontFace(DepthStencilOpDescription frontFace) { desc.frontFace = frontFace; return *this; }
        DepthStencilDescription& backFace(DepthStencilOpDescription backFace) { desc.backFace = backFace; return *this; }
    };

    enum class PrimitiveTopologyType
    {
        Undefined,
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        PatchList1,
        PatchList2,
        PatchList3,
        PatchList4,
        PatchList5,
        PatchList6,
        PatchList7,
        PatchList8,
        PatchList9,
        PatchList10,
        PatchList11,
        PatchList12,
        PatchList13,
        PatchList14,
        PatchList15,
        PatchList16
    };

    enum class InputClassification
    {
        PerVertexData,
        PerInstanceData
    };

    struct InputElementDescription
    {
        struct {
            const char* semanticName;
            unsigned int semanticIndex;
            Format format;
            unsigned int inputSlot;
            unsigned int alignedByteOffset;
            InputClassification inputSlotClass;
            unsigned int instanceDataStepRate;
            unsigned int offset;
        } desc;
        InputElementDescription& semanticName(const char* semanticName) { desc.semanticName = semanticName; return *this; }
        InputElementDescription& semanticIndex(unsigned int semanticIndex) { desc.semanticIndex = semanticIndex; return *this; }
        InputElementDescription& format(Format format) { desc.format = format; return *this; }
        InputElementDescription& inputSlot(unsigned int inputSlot) { desc.inputSlot = inputSlot; return *this; }
        InputElementDescription& alignedByteOffset(unsigned int alignedByteOffset) { desc.alignedByteOffset = alignedByteOffset; return *this; }
        InputElementDescription& inputSlotClass(InputClassification inputSlotClass) { desc.inputSlotClass = inputSlotClass; return *this; }
        InputElementDescription& instanceDataStepRate(unsigned int instanceDataStepRate) { desc.instanceDataStepRate = instanceDataStepRate; return *this; }
        InputElementDescription& offset(unsigned int offset) { desc.offset = offset; return *this; }
    };

    enum class IndexBufferStripCutValue
    {
        ValueDisabled,
        Value0xFFFF,
        Value0xFFFFFFFF
    };

    namespace implementation
    {
        class PipelineImplIf;
        class CommandListImplIf;
    }

    class PipelineAbs
    {
    public:
        PipelineAbs(
            Device& device,
            ShaderStorage& storage,
            GraphicsApi api);

        void setBlendState(const BlendDescription& desc);
        void setRasterizerState(const RasterizerDescription& desc);
        void setDepthStencilState(const DepthStencilDescription& desc);
        void setSampleMask(unsigned int mask);
        void setPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency = false);
        void setPrimitiveRestart(IndexBufferStripCutValue value);

    protected:
        friend class CommandList;

        void configure(implementation::CommandListImplIf* commandList, shaders::PipelineConfiguration* configuration);

        void setRenderTargetFormat(Format RTVFormat, Format DSVFormat = Format::UNKNOWN, unsigned int msaaCount = 1, unsigned int msaaQuality = 0);

        void setRenderTargetFormats(
            engine::vector<Format> RTVFormats,
            Format DSVFormat = Format::UNKNOWN,
            unsigned int msaaCount = 1,
            unsigned int msaaQuality = 0);

        engine::shared_ptr<implementation::PipelineImplIf> m_impl;
    };

    template <typename T>
    class Pipeline : public T, public PipelineAbs
    {
    public:
        Pipeline(
            Device& device,
            ShaderStorage& storage,
            GraphicsApi api)
            : PipelineAbs(device, storage, api)
        {}
    };
}
