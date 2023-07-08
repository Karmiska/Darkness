#pragma once

#include "containers/memory.h"

//struct ID3D12PipelineState;

namespace engine
{
    class Device;
    class TextureDSV;
    class BufferView;
    class RootSignature;
    struct BlendDescription;
    struct RasterizerDescription;
    struct DepthStencilDescription;
    enum class PrimitiveTopologyType;
    enum class Format;
    struct InputElementDescription;
    enum class IndexBufferStripCutValue;
    class ShaderBinary;
    class CommandList;
    
    namespace implementation
    {
        class PipelineImpl
        {
        public:
            PipelineImpl(const Device& device);
            ~PipelineImpl();
            
            PipelineImpl(const PipelineImpl&) = delete;
            PipelineImpl(PipelineImpl&&) = delete;
            PipelineImpl& operator=(const PipelineImpl&) = delete;
            PipelineImpl& operator=(PipelineImpl&&) = delete;
            
            void setRootSignature(const RootSignature& signature);
            void setBlendState(const BlendDescription& desc);
            void setRasterizerState(const RasterizerDescription& desc);
            void setDepthStencilState(const DepthStencilDescription& desc);
            void setSampleMask(unsigned int mask);
            void setPrimitiveTopologyType(PrimitiveTopologyType type);
            void setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0);
            void setRenderTargetFormats(unsigned int numRTVs, const Format* RTVFormats, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0);
            void setInputLayout(unsigned int numElements, const InputElementDescription* inputElementDescs);
            void setPrimitiveRestart(IndexBufferStripCutValue value);
            
            void setVertexShader(const ShaderBinary& shaderBinary);
            void setPixelShader(const ShaderBinary& shaderBinary);
            void setGeometryShader(const ShaderBinary& shaderBinary);
            void setHullShader(const ShaderBinary& shaderBinary);
            void setDomainShader(const ShaderBinary& shaderBinary);
            void setComputeShader(const ShaderBinary& shaderBinary);
            
            void setDepthBufferView(engine::shared_ptr<TextureDSV> view);
            
            void finalize();
            
            void* native() const;
        private:
            const Device& m_device;
            void* m_rootSignature;
            void* m_pipelineState;
            void* m_pipelineStateDesc;
            engine::shared_ptr<const void> m_inputLayouts;
        };
    }
}

