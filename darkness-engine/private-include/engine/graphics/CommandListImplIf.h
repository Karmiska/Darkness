#pragma once

#include "containers/vector.h"
#include "engine/graphics/CommonNoDep.h"

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
    struct Viewport;
    struct Rectangle;
    struct QueryResultTicks;
    struct CommandListAbs;
    enum class CommandListType;
    enum class PredicationOp;

    namespace implementation
    {
        class PipelineImplIf;

        class CommandListImplIf
        {
        public:
            virtual ~CommandListImplIf() {};

            virtual CommandListAbs& abs() = 0;

            virtual CommandListType type() const = 0;
            virtual void reset(implementation::PipelineImplIf* pipelineState) = 0;
            virtual void clear() = 0;
            virtual bool isOpen() const = 0;

            virtual void setRenderTargets(engine::vector<TextureRTV> targets) = 0;
            virtual void setRenderTargets(engine::vector<TextureRTV> targets, TextureDSV dsv) = 0;
            virtual void setRenderTargets(TextureDSV target) = 0;

            virtual void copyBuffer(Buffer srcBuffer, Buffer dstBuffer, uint64_t elements, size_t srcStartElement, size_t dstStartElement) = 0;
            virtual void copyBufferBytes(Buffer srcBuffer, Buffer dstBuffer, uint64_t bytes, size_t srcStartByte, size_t dstStartByte) = 0;

            virtual void clearBuffer(BufferUAV buffer, uint32_t value, size_t startElement, size_t numElements) = 0;

            virtual void copyTexture(TextureSRV src, TextureUAV dst) = 0;
            virtual void copyTexture(TextureSRV src, TextureSRV dst) = 0;
            virtual void copyTexture(TextureSRV src, TextureDSV dst) = 0;
            virtual void copyTexture(TextureSRV src, BufferUAV dst) = 0;
            virtual void copyTexture(TextureSRV src, BufferSRV dst) = 0;
            virtual void copyTexture(
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
                size_t		dstMipCount) = 0;

            virtual void clearTextureUAV(TextureUAV texture, const Color4f& color) = 0;
            virtual void clearTextureDSV(TextureDSV texture, float depth, uint8_t stencil) = 0;
            virtual void clearTextureRTV(TextureRTV texture, const Color4f& color) = 0;

            virtual void draw(size_t vertexCount) = 0;
            virtual void drawIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset) = 0;
            virtual void drawIndexedInstanced(size_t indexCount, size_t instanceCount, size_t firstIndex, int32_t vertexOffset, size_t firstInstance) = 0;
            virtual void drawIndexedIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset) = 0;
            virtual void drawIndexedInstancedIndirect(
                BufferIBV indexBuffer,
                Buffer indirectArguments,
                uint64_t argumentBufferOffset,
                Buffer indirectArgumentsCountBuffer,
                uint64_t countBufferOffset) = 0;
            virtual void executeIndexedIndirect(BufferIBV indexBuffer, Buffer indirectArguments, uint64_t argumentBufferOffset, Buffer countBuffer, uint64_t countBufferOffsetBytes) = 0;

            virtual void dispatch(size_t threadGroupCountX, size_t threadGroupCountY, size_t threadGroupCountZ) = 0;
            virtual void dispatchIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset) = 0;
            virtual void executeBundle(CommandListImplIf* commandList) = 0;
            virtual void dispatchMesh(size_t threadGroupCountX, size_t threadGroupCountY, size_t threadGroupCountZ) = 0;

            virtual void transition(Texture resource, ResourceState state, const SubResource& subResource = SubResource()) = 0;
            virtual void transition(TextureRTV resource, ResourceState state) = 0;
            virtual void transition(TextureSRV resource, ResourceState state) = 0;
            virtual void transition(TextureDSV resource, ResourceState state) = 0;

            virtual void transition(Buffer resource, ResourceState state) = 0;
            virtual void transition(BufferSRV resource, ResourceState state) = 0;
            virtual void transition(BufferIBV resource, ResourceState state) = 0;
            virtual void transition(BufferCBV resource, ResourceState state) = 0;
            virtual void transition(BufferVBV resource, ResourceState state) = 0;

            virtual void setPredicate(BufferSRV buffer, uint64_t offset, PredicationOp op) = 0;

            virtual void applyBarriers() = 0;

            virtual void bindPipe(
                implementation::PipelineImplIf* pipelineImpl,
                shaders::PipelineConfiguration* configuration) = 0;

            virtual void setViewPorts(const engine::vector<Viewport>& viewports) = 0;
            virtual void setScissorRects(const engine::vector<Rectangle>& rects) = 0;

            virtual void bindVertexBuffer(BufferVBV buffer) = 0;
            virtual void bindIndexBuffer(BufferIBV buffer) = 0;

            virtual void setStructureCounter(BufferUAV buffer, uint32_t value) = 0;
            virtual void copyStructureCounter(BufferUAV srcBuffer, Buffer dst, uint32_t dstByteOffset) = 0;

            virtual void begin() = 0;
            virtual void end() = 0;

            virtual void beginRenderPass(implementation::PipelineImplIf* pipeline, int frameBufferIndex) = 0;
            virtual void endRenderPass() = 0;

            virtual uint32_t startQuery(const char* query) = 0;
            virtual void stopQuery(uint32_t queryId) = 0;
            virtual void resolveQueries() = 0;

            virtual engine::vector<QueryResultTicks> fetchQueryResults(uint64_t freq) = 0;
        };
    }
}