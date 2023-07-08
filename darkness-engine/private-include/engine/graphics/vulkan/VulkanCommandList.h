#pragma once

#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/CommandListImplIf.h"
#include "engine/graphics/Semaphore.h"
#include "engine/graphics/CommonNoDep.h"
#include "engine/graphics/CommandListAbs.h"
#include "engine/graphics/Common.h"
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

    class Texture;
    class TextureRTV;
    class TextureSRV;
    class TextureUAV;
    class TextureDSV;
    class Queue;
    //enum class ImageLayout;
    struct SubResource;
    struct Viewport;
    struct Rectangle;

    class Device;

    namespace implementation
    {
        class PipelineImplIf;
        class PipelineImplVulkan;

        class CommandAllocatorImplVulkan;
        class DescriptorHandleImplVulkan;
        class CommandListImplVulkan : public CommandListImplIf
        {
        public:
            CommandListAbs& abs() override;

            CommandListImplVulkan(Device* device, CommandListType type, const char* name);
            CommandListImplVulkan(const CommandListImplVulkan&) = delete;
            CommandListImplVulkan(CommandListImplVulkan&&) = delete;
            CommandListImplVulkan& operator=(const CommandListImplVulkan&) = delete;
            CommandListImplVulkan& operator=(CommandListImplVulkan&&) = delete;

            CommandListType type() const override;
            void reset(implementation::PipelineImplIf* pipelineState);
            void clear();
            bool isOpen() const;
            const char* name() const { return m_abs.m_name; }

            void setRenderTargets(engine::vector<TextureRTV> targets);
            void setRenderTargets(engine::vector<TextureRTV> targets, TextureDSV dsv);
            void setRenderTargets(TextureDSV target);

            void copyBuffer(Buffer srcBuffer, Buffer dstBuffer, uint64_t elements, size_t srcStartElement = 0, size_t dstStartElement = 0);
            void copyBufferBytes(Buffer srcBuffer, Buffer dstBuffer, uint64_t bytes, size_t srcStartByte = 0, size_t dstStartByte = 0);
            void clearBuffer(BufferUAV buffer, uint32_t value, size_t startElement, size_t numElements);

            void copyTexture(TextureSRV src, TextureUAV dst);
            void copyTexture(TextureSRV src, TextureSRV dst);
            void copyTexture(TextureSRV src, TextureDSV dst);
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
            void clearTextureUAV(TextureUAV texture, const Color4f& color);
            void clearTextureDSV(TextureDSV texture, float depth, uint8_t stencil);
            void clearTextureRTV(TextureRTV texture, const Color4f& color);

            void draw(size_t vertexCount);
            void drawIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset);
            void drawIndexedInstanced(size_t indexCount, size_t instanceCount, size_t firstIndex, int32_t vertexOffset, size_t firstInstance);
            void drawIndexedIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset);
            void drawIndexedInstancedIndirect(
                BufferIBV indexBuffer,
                Buffer indirectArguments,
                uint64_t argumentBufferOffset,
                Buffer indirectArgumentsCountBuffer,
                uint64_t countBufferOffset);
            void executeIndexedIndirect(BufferIBV indexBuffer, Buffer indirectArguments, uint64_t argumentBufferOffset, Buffer countBuffer, uint64_t countBufferOffsetBytes);

            void dispatch(size_t threadGroupCountX, size_t threadGroupCountY, size_t threadGroupCountZ);
            void dispatchIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset);
            void executeBundle(CommandListImplIf* commandList);
            void dispatchMesh(size_t threadGroupCountX, size_t threadGroupCountY, size_t threadGroupCountZ);

            void transition(Texture resource, ResourceState state, const SubResource& subResource = SubResource());
            void transition(TextureRTV resource, ResourceState state);
            void transition(TextureSRV resource, ResourceState state);
            void transition(TextureDSV resource, ResourceState state);

            void transition(Buffer resource, ResourceState state);
            void transition(BufferSRV resource, ResourceState state);
            void transition(BufferIBV resource, ResourceState state);
            void transition(BufferCBV resource, ResourceState state);
            void transition(BufferVBV resource, ResourceState state);

            void setPredicate(BufferSRV buffer, uint64_t offset, PredicationOp op);

            void applyBarriers();

            void bindPipe(
                implementation::PipelineImplIf* pipelineImpl,
                shaders::PipelineConfiguration* configuration) override;

            void setViewPorts(const engine::vector<Viewport>& viewports);
            void setScissorRects(const engine::vector<Rectangle>& rects);

            void bindVertexBuffer(BufferVBV buffer);
            void bindIndexBuffer(BufferIBV buffer);

            void setStructureCounter(BufferUAV buffer, uint32_t value);
            void copyStructureCounter(BufferUAV srcBuffer, Buffer dst, uint32_t dstByteOffset);

            void begin();
            void end();

            


            // dunno about these
            // -------------------------------
            /*void setResourceBarrier(VkSubmitInfo barrier) const;
            VkSubmitInfo& barrier();
            const VkSubmitInfo& barrier() const;*/
            // -------------------------------
            VkCommandBuffer& native();
            const VkCommandBuffer& native() const;

            uint32_t startQuery(const char* /*query*/) { LOG("Not implemented startQuery"); return 0u; };
            void stopQuery(uint32_t /*queryId*/) { LOG("Not implemented stopQuery"); };
            void resolveQueries() {};

            engine::vector<QueryResultTicks> fetchQueryResults(uint64_t /*freq*/) { LOG("Not implemented fetchQueryResults"); return {}; };

            void setNextPassDebugName(const char* name);
        private:
            void beginRenderPass(implementation::PipelineImplIf* pipeline, int frameBufferIndex) override;
            void endRenderPass();

        private:
            Device* m_device;
            CommandListType m_type;
            VkCommandBuffer m_commandList;
            CommandListAbs m_abs;
            mutable VkSubmitInfo m_barrier;
            engine::vector<VkImageMemoryBarrier> m_imageBarriers;
            engine::vector<VkBufferMemoryBarrier> m_bufferBarriers;
            std::size_t m_lastAppliedImageBarrierIndex;
            std::size_t m_lastAppliedBufferBarrierIndex;

            engine::vector<VkRect2D> m_scissorRects;

            const char* m_debugPassName;

            engine::vector<TextureRTV> m_frameBufferTargets;
            engine::vector<TextureDSV> m_frameBufferTargetsDSV;
            engine::vector<engine::shared_ptr<VkFramebuffer>> m_frameBuffers;
            struct FrameBufferRange
            {
                int start;
                int count;
            };
            FrameBufferRange m_currentFrameBufferRange;

            VkRenderPassBeginInfo m_renderPassBeginInfo;
            VkClearValue m_clearColor;

            engine::shared_ptr<CommandAllocatorImplVulkan> m_allocator;
            bool m_open;

            void bindDescriptorSets();
            void clearTexture(Texture texture, const Color4f& color, const SubResource& subResource);

            void deferredRenderPassBegin();
            VkRenderPass* m_renderPass;
            VkPipeline* m_pipeline;
            VkPipelineLayout* m_pipelineLayout;
            const engine::vector<VkDescriptorSet>* m_descriptorSet;

			bool m_computePipeline;

			implementation::PipelineImplVulkan* m_activePipelineImpl;
			shaders::PipelineConfiguration* m_activeConfiguration;
			void bindPipeDeferred();
            bool m_nextUseInputLaytout;
        };
    }
}

