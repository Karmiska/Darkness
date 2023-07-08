#pragma once

#include <functional>
#include <cstdint>
#include "containers/vector.h"

namespace engine
{
    namespace shaders
    {
        class PipelineConfiguration;
    }

    class ShaderBinary;
    struct BlendDescription;
    struct RasterizerDescription;
    struct DepthStencilDescription;
    enum class PrimitiveTopologyType;
    enum class IndexBufferStripCutValue;
    enum class Format;

    namespace implementation
    {
        class CommandListImplIf;

        class PipelineShadersImplIf
        {
        public:
            virtual ~PipelineShadersImplIf() {};

            virtual const ShaderBinary* cs() const = 0;
            virtual const ShaderBinary* vs() const = 0;
            virtual const ShaderBinary* ps() const = 0;
            virtual const ShaderBinary* hs() const = 0;
            virtual const ShaderBinary* gs() const = 0;
            virtual const ShaderBinary* ds() const = 0;
            
            virtual const ShaderBinary* rg() const = 0;
            virtual const ShaderBinary* is() const = 0;
            virtual const ShaderBinary* ms() const = 0;
            virtual const ShaderBinary* ah() const = 0;
            virtual const ShaderBinary* ch() const = 0;
            
            virtual const ShaderBinary* amp() const = 0;
            virtual const ShaderBinary* mesh() const = 0;
        };

        class PipelineRootSignatureImplIf
        {
        public:
            virtual ~PipelineRootSignatureImplIf() {};
            virtual std::size_t rootConstantCount() const = 0;
        };

        class PipelineStateImplIf
        {
        public:
            virtual ~PipelineStateImplIf() {};
            virtual uint64_t hash(uint64_t hash) const = 0;

            virtual void setBlendState(const BlendDescription& desc) = 0;
            virtual void setRasterizerState(const RasterizerDescription& desc) = 0;
            virtual void setDepthStencilState(const DepthStencilDescription& desc) = 0;
            virtual void setSampleMask(unsigned int mask) = 0;
            virtual void setPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency = false) = 0;
            virtual void setPrimitiveRestart(IndexBufferStripCutValue value) = 0;
            virtual void setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0) = 0;
            virtual void setRenderTargetFormats(engine::vector<Format> RTVFormats, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0) = 0;
        };

        class PipelineImplIf
        {
        public:
            virtual ~PipelineImplIf() {};

            virtual void setBlendState(const BlendDescription& desc) = 0;
            virtual void setRasterizerState(const RasterizerDescription& desc) = 0;
            virtual void setDepthStencilState(const DepthStencilDescription& desc) = 0;
            virtual void setSampleMask(unsigned int mask) = 0;
            virtual void setPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency = false) = 0;
            virtual void setPrimitiveRestart(IndexBufferStripCutValue value) = 0;
            virtual void setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0) = 0;
            virtual void setRenderTargetFormats(engine::vector<Format> RTVFormats, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0) = 0;
            virtual void configure(CommandListImplIf* commandList, shaders::PipelineConfiguration* configuration) = 0;
        };
    }
}