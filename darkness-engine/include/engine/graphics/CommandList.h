#pragma once

#include "engine/graphics/CommandListImplIf.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/graphics/Format.h"
#include "engine/primitives/Color.h"
#include "engine/primitives/Vector4.h"
#include "engine/rendering/tools/BufferMath.h"
#include "containers/vector.h"

namespace engine
{
    namespace shaders
    {
        class PipelineConfiguration;
    }
    namespace implementation
    {
        class PipelineImplIf;
        class DeviceImplIf;
    }

    class Device;
    class Queue;
    class Barrier;
    class Buffer;
    class BufferSRV;
    class BufferUAV;
    class BufferIBV;
    class BufferCBV;
    class BufferVBV;
    class Texture;
    class TextureRTV;
    class TextureSRV;
    class TextureUAV;
    class TextureDSV;
    
    class ImageBuffer;
    class Semaphore;
    enum class ResourceBarrierFlags;
    enum class ResourceState;
    //enum class ImageLayout;
    struct SubResource;
    struct Viewport;
    struct Rectangle;
    struct CorePipelines;
    struct QueryResultTicks;

    struct ShaderDebugOutput
    {
        uint32_t itemType;
        uint32_t uvalue;
        float fvalue;
    };

    class CommandList
    {
    public:
        CommandList() = default;
        CommandList(CommandList&&) = default;
        CommandList& operator=(CommandList&&) = default;
        CommandList(const CommandList&) = default;
        CommandList& operator=(const CommandList&) = default;

        Barrier createBarrier(
            ResourceBarrierFlags flags,
            TextureRTV resource,
            ResourceState before,
            ResourceState after,
            unsigned int subResource,
            const Semaphore& waitSemaphore,
            const Semaphore& signalSemaphore
        );

        const char* name() const;
        void name(const char* name);

        CommandListType type() const;

        bool verify() const { return m_impl != nullptr; };

        void transition(Texture resource, ResourceState state);
        void transition(Texture resource, ResourceState state, const SubResource& subResource);
        void transition(TextureRTV resource, ResourceState state);
        void transition(TextureSRV resource, ResourceState state);
        void transition(TextureDSV resource, ResourceState state);

        void transition(Buffer resource, ResourceState state);
        void transition(BufferSRV resource, ResourceState state);
        void transition(BufferIBV resource, ResourceState state);
        void transition(BufferVBV resource, ResourceState state);
        void transition(BufferCBV resource, ResourceState state);

        void setPredicate(BufferSRV buffer, uint64_t offset, PredicationOp op);

        void setRenderTargets(
            engine::vector<TextureRTV> targets);
        void setRenderTargets(
            engine::vector<TextureRTV> targets,
            TextureDSV dsv);

        void clearRenderTargetView(TextureRTV target);
        void clearRenderTargetView(TextureRTV target, const Color4f& color);
        void clearDepthStencilView(TextureDSV target, float depth, uint8_t stencil = 0);

		void clearBuffer(BufferUAV buffer, uint32_t value, size_t startElement, size_t numElements);
		void clearBuffer(BufferUAV buffer, uint32_t value = 0u);

		void clearTexture(TextureUAV texture, float value);
		void clearTexture(TextureUAV texture, Vector4f value);
		void clearTexture(TextureUAV texture, uint32_t value);
		void clearTexture(TextureUAV texture, Vector4<uint32_t> value);

        void copyBuffer(
            Buffer srcBuffer,
            Buffer dstBuffer,
            uint64_t elements,
            size_t srcStartElement = 0,
            size_t dstStartElement = 0);

        void copyBufferBytes(
            Buffer srcBuffer,
            Buffer dstBuffer,
            uint64_t bytes,
            size_t srcStartByte = 0,
            size_t dstStartByte = 0);

        void copyBufferIndirect(
            Buffer srcBuffer,
            Buffer dstBuffer,
            Buffer count,
            Buffer* srcBufferIndex = nullptr,
            Buffer* dstBufferIndex = nullptr,
            Buffer* countIndex = nullptr);

        template<typename T>
        void reset(T& pipeline)
        {
            reset(pipeline.m_impl);
        };

        void clear();

        template<typename T>
        void bindPipe(T& pipeline)
        {
            pipeline.setRenderTargetFormats(lastSetRTVFormats(), lastSetDSVFormat(), lastSetSamples());
            bindPipe(pipeline.m_impl.get(), &pipeline);
        };

        void setViewPorts(const engine::vector<Viewport>& viewports);
        void setScissorRects(const engine::vector<Rectangle>& rects);

        engine::vector<QueryResultTicks> fetchQueryResults(uint64_t freq);

        Semaphore& commandListSemaphore();

        implementation::CommandListImplIf* native() { return m_impl.get(); }
        const implementation::CommandListImplIf* native() const { return m_impl.get(); }
    private:
        void reset(engine::shared_ptr<implementation::PipelineImplIf> pipelineState);
        void bindPipe(
            implementation::PipelineImplIf* pipelineImpl,
            shaders::PipelineConfiguration* configuration);
        void beginRenderPass(engine::shared_ptr<implementation::PipelineImplIf> pipeline, int frameBufferIndex);

        engine::vector<Format>& lastSetRTVFormats();
        Format lastSetDSVFormat();
		unsigned int lastSetSamples();
        unsigned long long submitFenceValue() const;
        void submitFenceValue(unsigned long long value);
        CommandListType commandListType() const;
        engine::vector<BufferUAVOwner>& debugBuffers();
    public:

        void bindVertexBuffer(BufferVBV buffer);

        void begin();
        void end();

        template<typename T>
        void beginRenderPass(T& pipeline, int frameBufferIndex)
        {
            beginRenderPass(pipeline.m_impl, frameBufferIndex);
        };

        void endRenderPass();

        void draw(size_t vertexCount);
        void drawIndexedInstanced(
            BufferIBV buffer,
            size_t indexCount,
            size_t instanceCount,
            size_t firstIndex,
            int32_t vertexOffset,
            size_t firstInstance);

        void drawIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset);

        void drawIndexedIndirect(BufferIBV buffer, Buffer indirectArguments, uint64_t argumentBufferOffset);
        
        void drawIndexedInstancedIndirect(
            BufferIBV indexBuffer, 
            Buffer indirectArguments,
            uint64_t argumentBufferOffset,
            Buffer indirectArgumentsCountBuffer,
            uint64_t countBufferOffset);

        void dispatch(
            size_t threadGroupCountX,
            size_t threadGroupCountY,
            size_t threadGroupCountZ);

        void dispatchIndirect(
            Buffer indirectArgs,
            uint64_t argumentBufferOffset);

        void executeIndexedIndirect(
            BufferIBV indexBuffer,
            Buffer indirectArguments,
            uint64_t argumentBufferOffset,
            Buffer countBuffer,
            uint64_t countBufferOffsetBytes);

        void executeBundle(CommandList& commandList);

        void dispatchMesh(
            size_t threadGroupCountX,
            size_t threadGroupCountY,
            size_t threadGroupCountZ);

        void copyTexture(TextureSRV src, TextureUAV dst);
        void copyTexture(TextureSRV src, TextureSRV dst);
		void copyTexture(TextureSRV src, TextureDSV dst);
        void copyTexture(TextureSRV src, TextureRTV dst);
        void copyTexture(TextureSRV src, BufferUAV dst);
        void copyTexture(TextureSRV src, BufferSRV dst);

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
			size_t		dstMipCount);

        void setStructureCounter(BufferUAV buffer, uint32_t value);
        void copyStructureCounter(BufferUAV srcBuffer, Buffer dst, uint32_t dstByteOffset);

        void copyTexture(
            TextureSRV src,
            TextureUAV dst,
            size_t srcLeft,
            size_t srcTop,
            size_t srcFront,
            size_t dstLeft,
            size_t dstTop,
            size_t dstFront,
            size_t width,
            size_t height,
            size_t depth);

        GraphicsApi api() const;
    private:
        

        void copyTexture(
            TextureSRV src,
            TextureRTV dst,
            size_t srcLeft,
            size_t srcTop,
            size_t srcFront,
            size_t dstLeft,
            size_t dstTop,
            size_t dstFront,
            size_t width,
            size_t height,
            size_t depth);

    private:
        friend class Device;
        friend class implementation::DeviceImplDX12;
        friend class implementation::DeviceImplVulkan;
        CommandList(const Device& device, const char* name, CommandListType type = CommandListType::Direct, GraphicsApi api = GraphicsApi::DX12);
        CommandList(implementation::CommandListImplIf* impl, const Device& device, const char* name, CommandListType type = CommandListType::Direct, GraphicsApi api = GraphicsApi::DX12);

    public:
        engine::shared_ptr<implementation::CommandListImplIf> m_impl;

    private:
        friend class Device;
        
        void savePipeline(shaders::PipelineConfiguration* configuration);

        void transitionCommonSRV(TextureSRV srv, ResourceState state);
        void transitionCommonUAV(TextureUAV uav, ResourceState state);
    private:
        friend class Queue;

        void setDebugBuffers(shaders::PipelineConfiguration* configuration);

		void copyDepthTexture(TextureSRV src, TextureDSV dst);

        void makeSureHasCorePipelines(engine::shared_ptr<CorePipelines> pipelines);
    };
}
