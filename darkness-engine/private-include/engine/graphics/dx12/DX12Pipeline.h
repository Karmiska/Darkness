#pragma once

#include "engine/graphics/PipelineImplIf.h"
#include "containers/memory.h"
#include "containers/vector.h"
#include "containers/unordered_map.h"
#include "containers/string.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Common.h"
#include "tools/ComPtr.h"
#include "tools/ByteRange.h"
#include <functional>

namespace engine
{
    namespace shaders
    {
        class PipelineConfiguration;
        class Shader;
    }

    namespace implementation
    {
        class DescriptorHeapImpl;
    }

    class Device;
    class Buffer;
    class BufferCBV;
    class TextureDSV;
    class TextureRTV;
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
    class ShaderBinaryImplIf;
    class ShaderBinaryImplDX12;
    class ShaderBinaryImplVulkan;
    class Sampler;
    class TextureSRV;
    class ShaderStorage;
    class RootSignature;
    class PipelineAbs;

    namespace implementation
    {
        class CommandListImplIf;
        constexpr int MaximumBindlessResources = 600;

        struct ShaderContainerDX12
        {
            const shaders::Shader* shader;
            D3D12_SHADER_VISIBILITY visibility;
        };

        class PipelineShadersDX12 : public PipelineShadersImplIf
#if 1
        {
        public:
            PipelineShadersDX12(
                Device& device,
                ShaderStorage& storage,
                std::function<void()> onChange,
                shaders::PipelineConfiguration* configuration);
            ~PipelineShadersDX12();

            const ShaderBinary* cs() const override;
            const ShaderBinary* vs() const override;
            const ShaderBinary* ps() const override;
            const ShaderBinary* hs() const override;
            const ShaderBinary* gs() const override;
            const ShaderBinary* ds() const override;

            const ShaderBinary* rg() const override;
            const ShaderBinary* is() const override;
            const ShaderBinary* ms() const override;
            const ShaderBinary* ah() const override;
            const ShaderBinary* ch() const override;

            const ShaderBinary* amp() const override;
            const ShaderBinary* mesh() const override;
        private:
            Device& m_device;
            ShaderStorage& m_storage;
            std::function<void()> m_onChange;

            engine::shared_ptr<const ShaderBinary> m_cs;
            engine::shared_ptr<const ShaderBinary> m_vs;
            engine::shared_ptr<const ShaderBinary> m_ps;
            engine::shared_ptr<const ShaderBinary> m_hs;
            engine::shared_ptr<const ShaderBinary> m_gs;
            engine::shared_ptr<const ShaderBinary> m_ds;

            engine::shared_ptr<const ShaderBinary> m_rg;
            engine::shared_ptr<const ShaderBinary> m_is;
            engine::shared_ptr<const ShaderBinary> m_ms;
            engine::shared_ptr<const ShaderBinary> m_ah;
            engine::shared_ptr<const ShaderBinary> m_ch;

            engine::shared_ptr<const ShaderBinary> m_amp;
            engine::shared_ptr<const ShaderBinary> m_mesh;

            void loadShaders(shaders::PipelineConfiguration* configuration);
        };
#endif

        struct ShaderBindingCountDX12
        {
            std::size_t resourceBindingCount;
            std::size_t samplerBindingCount;
            std::size_t rootParameterCount;
            std::size_t descriptorTableCount;
        };

        class PipelineRootSignatureDX12 : public PipelineRootSignatureImplIf
#if 1
        {
        public:
            PipelineRootSignatureDX12(
                Device& device,
                engine::vector<ShaderContainerDX12>& shaders);
            ID3D12RootSignature* signature()
            {
                return m_signature.Get();
            };
            const ShaderBindingCountDX12& bindingCounts() const
            {
                return m_bindingCounts;
            };
            std::size_t rootConstantCount() const override
            {
                return m_rootConstantCount;
            }
        private:
            tools::ComPtr<ID3D12RootSignature> m_signature;
            ShaderBindingCountDX12 m_bindingCounts;
            std::size_t m_rootConstantCount;
        };
#endif
        
        class PipelineStateDX12 : public PipelineStateImplIf
#if 1
        {
        public:
            PipelineStateDX12();

            uint64_t hash(uint64_t hash) const;

            void setBlendState(const BlendDescription& desc);
            void setRasterizerState(const RasterizerDescription& desc);
            void setDepthStencilState(const DepthStencilDescription& desc);
            void setSampleMask(unsigned int mask);
            void setPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency = false);
            void setPrimitiveRestart(IndexBufferStripCutValue value);
            void setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0);
            void setRenderTargetFormats(engine::vector<Format> RTVFormats, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0);

            enum class FinalizeFlags
            {
                None,
                ForceReCreate
            };
            void finalize(
                Device& device, 
                const PipelineShadersDX12& shaders, 
                PipelineRootSignatureDX12& rootSignature, 
                engine::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayouts,
                FinalizeFlags flags = FinalizeFlags::None
                );
            ID3D12PipelineState* pipeline()
            {
                return m_pipeline.Get();
            }

            D3D_PRIMITIVE_TOPOLOGY topology;

            D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipelineStateDesc()
            {
                return *m_pipelineStateDesc;
            }
            bool finalized() const { return m_finalized; }
            bool errorState() const { return m_inErrorState; }
        private:
            engine::shared_ptr<D3D12_GRAPHICS_PIPELINE_STATE_DESC> m_pipelineStateDesc;
            engine::shared_ptr<D3D12_COMPUTE_PIPELINE_STATE_DESC> m_computePipelineStateDesc;
            tools::ComPtr<ID3D12PipelineState> m_pipeline;
            bool m_finalized;
            bool m_inErrorState;

            void initialize();
        };
#endif

        class DescriptorTablesDX12
        {
        public:
            DescriptorTablesDX12(
                Device& device,
                size_t shaderResourceBindingCount,
                size_t samplerBindingCount);

            DescriptorHandleDX12& resourceHandles();
            DescriptorHandleDX12& samplerHandles();
        private:
            DescriptorHandleDX12 m_resourceHandles;
            DescriptorHandleDX12 m_samplerHandles;
        };

        class CommandListImplDX12;
        class PipelineImplDX12 : public PipelineImplIf
        {
        public:
            PipelineImplDX12(
                Device& device,
                ShaderStorage& storage);

            ~PipelineImplDX12();
            PipelineImplDX12(const PipelineImplDX12&) = default;
            PipelineImplDX12(PipelineImplDX12&&) = default;
            PipelineImplDX12& operator=(const PipelineImplDX12&) = default;
            PipelineImplDX12& operator=(PipelineImplDX12&&) = default;

            void setBlendState(const BlendDescription& desc) override;
            void setRasterizerState(const RasterizerDescription& desc) override;
            void setDepthStencilState(const DepthStencilDescription& desc) override;
            void setSampleMask(unsigned int mask) override;
            void setPrimitiveTopologyType(PrimitiveTopologyType type, bool adjacency = false) override;
            void setPrimitiveRestart(IndexBufferStripCutValue value) override;
            void setRenderTargetFormat(Format RTVFormat, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0) override;
            void setRenderTargetFormats(engine::vector<Format> RTVFormats, Format DSVFormat, unsigned int msaaCount = 1, unsigned int msaaQuality = 0) override;
            
            struct TableBinding
            {
                D3D12_GPU_DESCRIPTOR_HANDLE handle;
                UINT parameterIndex;
            };
            const engine::vector<TableBinding>& resourceTablePointers()
            {
                return *m_lastTableBindings;
            };
            
            D3D_PRIMITIVE_TOPOLOGY& topology()
            {
                return m_pipelineState.topology;
            };

            void configure(CommandListImplIf* cmdList, shaders::PipelineConfiguration* configuration);
            bool valid() const;
        private:
            struct ConstantUpdate
            {
                BufferCBV buffer;
                tools::ByteRange range;
            };

            struct PipelineCache
            {
                engine::unique_ptr<DescriptorTablesDX12> descriptorTables;
                engine::unordered_map<D3D12_SHADER_VISIBILITY, engine::vector<engine::unique_ptr<BufferCBVOwner>>> constants;
                engine::vector<TableBinding> tableBindings;
                uint64_t lastUsedFrame;

                engine::vector<ConstantUpdate> constantUploads;
            };

        private:
            Device& m_device;
            ShaderStorage& m_storage;
            engine::unique_ptr<PipelineShadersDX12> m_pipelineShaders;
            engine::unique_ptr<PipelineRootSignatureDX12> m_rootSignature;
            PipelineStateDX12 m_pipelineState;
            engine::vector<D3D12_INPUT_ELEMENT_DESC> m_inputLayouts;
            engine::vector<char> m_semanticNames;
            uint32_t m_semanticIndex;
            engine::vector<TableBinding>* m_lastTableBindings;
            engine::unordered_map<uint64_t, PipelineCache> m_hashResourceStorage;
            engine::vector<ShaderContainerDX12> m_shaders;

            void gatherShaders(shaders::PipelineConfiguration* configuration);

            void onShaderChange();
            void createInputLayout();

            // CommandListImpl
            // needs access to ID3D12PipelineState
            // needs access to ID3D12RootSignature
            // needs access to configure
            friend class CommandListImplDX12;
            

            // PipelineAbs
            // needs access to configure
            friend class PipelineAbs;

            enum class WriteFlags
            {
                WriteDescriptors,
                UpdateConstants
            };
            void writeResources(
                CommandListImplDX12& commandList, 
                PipelineCache& cache, 
                WriteFlags flags = WriteFlags::WriteDescriptors);

            
            void eraseUnused();
        };
    }
}

