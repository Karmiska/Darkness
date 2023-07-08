#pragma once

#include "engine/graphics/CommandListImplIf.h"
#include "engine/graphics/dx12/DX12AfterMath.h"

#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Common.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/graphics/CommandListAbs.h"
#include "engine/graphics/GpuMarkerStorage.h"
#include "engine/graphics/ResourceOwners.h"
#include "tools/ComPtr.h"

#include "containers/vector.h"
#include "containers/memory.h"

namespace engine
{
    namespace shaders
    {
        class PipelineConfiguration;
    }

    enum class ResourceState;
    class Barrier;
    class Color4f;
    class Buffer;
    class BufferUAV;
    class BufferSRV;
    class BufferVBV;
    class BufferIBV;
    class BufferCBV;
	class Device;

    class Texture;
    class TextureRTV;
    class TextureSRV;
    class TextureUAV;
    class TextureDSV;
    class Semaphore;
    class Queue;
    //enum class ImageLayout;
    struct SubResource;
    struct Viewport;
    struct Rectangle;
    struct QueryResultTicks;

    namespace implementation
    {
        class BufferUAVImpl;
        class DeviceImpl;
        class PipelineImpl;
        class CommandAllocatorImplDX12;
        class CommandListImplDX12 : public CommandListImplIf
        {
        public:
            CommandListAbs& abs() override { return m_abs; };
            
            CommandListImplDX12(Device* device, CommandListType type, const char* name);
            CommandListImplDX12(const CommandListImplDX12&) = delete;
            CommandListImplDX12(CommandListImplDX12&&) = delete;
            CommandListImplDX12& operator=(const CommandListImplDX12&) = delete;
            CommandListImplDX12& operator=(CommandListImplDX12&&) = delete;
            ~CommandListImplDX12();

			CommandListType type() const override;
            void reset(implementation::PipelineImplIf* pipelineState) override;
            void clear() override;
            bool isOpen() const override;
            const char* name() const { return m_abs.m_name; }

            void setRenderTargets(engine::vector<TextureRTV> targets) override;
            void setRenderTargets(engine::vector<TextureRTV> targets, TextureDSV dsv) override;
			void setRenderTargets(TextureDSV target) override;

            void copyBuffer(Buffer srcBuffer, Buffer dstBuffer, uint64_t elements, size_t srcStartElement = 0, size_t dstStartElement = 0) override;
            void copyBufferBytes(Buffer srcBuffer, Buffer dstBuffer, uint64_t bytes, size_t srcStartByte = 0, size_t dstStartByte = 0) override;

            void clearBuffer(BufferUAV buffer, uint32_t value, size_t startElement, size_t numElements) override;

            void copyTexture(TextureSRV src, TextureUAV dst) override;
            void copyTexture(TextureSRV src, TextureSRV dst) override;
            void copyTexture(TextureSRV src, TextureDSV dst) override;
            void copyTexture(TextureSRV src, BufferUAV dst) override;
            void copyTexture(TextureSRV src, BufferSRV dst) override;
			void copyTexture(
				Buffer		srcBuffer,
				size_t		srcOffset,
				size_t		srcWidth,
				size_t		srcHeight,
				size_t		srcRowPitch,
				Texture		dst,
				size_t		dstX,
				size_t		dstY,
				size_t		dstMip,
				size_t		dstSlice,
				size_t		dstMipCount) override;

            void clearTextureUAV(TextureUAV texture, const Color4f& color) override;
            void clearTextureDSV(TextureDSV texture, float depth, uint8_t stencil) override;
            void clearTextureRTV(TextureRTV texture, const Color4f& color) override;

            void draw(size_t vertexCount) override;
            void drawIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset) override;
            void drawIndexedInstanced(size_t indexCount, size_t instanceCount, size_t firstIndex, int32_t vertexOffset, size_t firstInstance) override;
            void drawIndexedIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset) override;
            void drawIndexedInstancedIndirect(
                BufferIBV indexBuffer,
                Buffer indirectArguments,
                uint64_t argumentBufferOffset,
                Buffer indirectArgumentsCountBuffer,
                uint64_t countBufferOffset);
            void executeIndexedIndirect(BufferIBV indexBuffer, Buffer indirectArguments, uint64_t argumentBufferOffset, Buffer countBuffer, uint64_t countBufferOffsetBytes) override;

            void dispatch(size_t threadGroupCountX, size_t threadGroupCountY, size_t threadGroupCountZ) override;
            void dispatchIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset) override;
            void executeBundle(CommandListImplIf* commandList) override;
            void dispatchMesh(size_t threadGroupCountX, size_t threadGroupCountY, size_t threadGroupCountZ) override;

            void transition(Texture resource, ResourceState state, const SubResource& subResource = SubResource()) override;
            void transition(TextureRTV resource, ResourceState state) override;
            void transition(TextureSRV resource, ResourceState state) override;
            void transition(TextureDSV resource, ResourceState state) override;

            void transition(Buffer resource, ResourceState state) override;
            void transition(BufferSRV resource, ResourceState state) override;
            void transition(BufferIBV resource, ResourceState state) override;
            void transition(BufferCBV resource, ResourceState state) override;
            void transition(BufferVBV resource, ResourceState state) override;

            void setPredicate(BufferSRV buffer, uint64_t offset, PredicationOp op) override;

            void applyBarriers() override;

            void bindPipe(
                implementation::PipelineImplIf* pipelineImpl,
                shaders::PipelineConfiguration* configuration) override;

            void setViewPorts(const engine::vector<Viewport>& viewports) override;
            void setScissorRects(const engine::vector<Rectangle>& rects) override;

            void bindVertexBuffer(BufferVBV buffer) override;
            void bindIndexBuffer(BufferIBV buffer) override;

            void setStructureCounter(BufferUAV buffer, uint32_t value) override;
            void copyStructureCounter(BufferUAV srcBuffer, Buffer dst, uint32_t dstByteOffset) override;

            void begin() override;
            void end() override;

            void beginRenderPass(implementation::PipelineImplIf* pipeline, int frameBufferIndex) override;
            void endRenderPass() override;

            ID3D12GraphicsCommandList* native();
            ID3D12GraphicsCommandList* native() const;

#ifdef DXR_BUILD
            ID3D12GraphicsCommandList6* dxrCommandlist() const;
#endif

            uint32_t startQuery(const char* query) override;
            void stopQuery(uint32_t queryId) override;
            void resolveQueries() override;

            engine::vector<QueryResultTicks> fetchQueryResults(uint64_t freq) override;

#ifdef AFTERMATH_ENABLED
            AfterMathHandle& afterMathContextHandle()
            {
                return m_afterMathContext;
            }
#endif
        private:
#ifdef AFTERMATH_ENABLED
            AfterMathHandle m_afterMathContext;
#endif
			Device* m_device;
            CommandListType m_type;
            CommandListAbs m_abs;
            engine::unique_ptr<GpuMarkerContainer> m_gpuMarkers;
            engine::shared_ptr<CommandAllocatorImplDX12> m_allocator;
            tools::ComPtr<ID3D12GraphicsCommandList> m_commandList;
#ifdef DXR_BUILD
			tools::ComPtr<ID3D12GraphicsCommandList6> m_dxrCommandList;
#endif
            engine::shared_ptr<Semaphore> m_temporary;
            engine::vector<D3D12_RESOURCE_BARRIER> m_barriers;
            DescriptorHeapsDX12 m_descriptorHeaps;
            bool m_open;
            ID3D12PipelineState* m_lastPipelineState;
            ID3D12RootSignature* m_rootSignature;

            engine::vector<D3D12_INDIRECT_ARGUMENT_DESC> m_argumentDescs;
            engine::vector<ID3D12CommandSignature*> m_commandSignatures;

            engine::vector<uint64_t> m_lastRootDescriptor;
            D3D_PRIMITIVE_TOPOLOGY m_lastTopology;

            tools::ComPtr<ID3D12QueryHeap> m_queryHeap;
            BufferOwner m_queryBuffer;
			Buffer m_queryBufferHandle;
            engine::vector<std::pair<uint32_t, engine::string>> m_querys;
            bool m_resolved;
            bool m_pipeBound;

            size_t m_lastAppliedBarrierIndex;
            
            void setHeaps();
        };
    }
}

