#pragma once

#include "containers/vector.h"
#include "engine/graphics/Semaphore.h"

struct MetalCommandList{};
struct MetalBarrier{};

namespace engine
{
    class Device;
    class Pipeline;
    class DescriptorHandle;
    enum class ResourceState;
    class Barrier;
    class Color4f;
    class Buffer;
    class BufferView;
    class Texture;
    class TextureSRV;
    class TextureUAV;
    class Queue;
    //enum class ImageLayout;
    struct SubResource;
    
    namespace implementation
    {
        class CommandAllocatorImpl;
        class CommandListImpl
        {
        public:
            CommandListImpl(const Device& device);
            
            CommandListImpl(const CommandListImpl&) = delete;
            CommandListImpl(CommandListImpl&&) = delete;
            CommandListImpl& operator=(const CommandListImpl&) = delete;
            CommandListImpl& operator=(CommandListImpl&&) = delete;
            
            void reset(const Pipeline& pipelineState);
            
            void setRenderTargets(engine::vector<DescriptorHandle> targets);
            void clearRenderTargetView(const DescriptorHandle& target, const Color4f& color);
            void copyBuffer(
                            const Buffer& srcBuffer,
                            Buffer& dstBuffer,
                            uint64_t elements,
                            uint32_t srcStartElement = 0,
                            uint32_t dstStartElement = 0);
            void bindPipeline(const Pipeline& pipeline);
            void bindVertexBuffer(const Buffer& buffer);
            void bindIndexBuffer(const Buffer& buffer);
            void bindDescriptorSets(const Pipeline& pipeline, const DescriptorHandle& descriptor);
            
            void clearBuffer(Buffer& buffer, uint32_t value, uint32_t startElement, uint32_t numElements);
            void clearTexture(Texture& texture, const SubResource& subResource, const Color4f& color);
            
            void draw();
            void drawIndexed(
                             uint32_t indexCount,
                             uint32_t instanceCount,
                             uint32_t firstIndex,
                             int32_t vertexOffset,
                             uint32_t firstInstance);
            
            void setResourceBarrier(MetalBarrier barrier) const;
            MetalBarrier& barrier();
            const MetalBarrier& barrier() const;
            
            Semaphore& finishedSemaphore();
            const Semaphore& finishedSemaphore() const;
            
            void begin();
            void end();
            
            void beginRenderPass(const Pipeline& pipeline, int frameBufferIndex);
            void endRenderPass();
            
            //void transitionTexture(const Texture& image, ImageLayout from, ImageLayout to);
            void copyTexture(const Texture& src, Texture& dst);
            
            MetalCommandList& native();
            const MetalCommandList& native() const;
        private:
            const Device& m_device;
            MetalCommandList m_commandList;
            mutable MetalBarrier m_barrier;
            Semaphore m_finishedSemaphore;
            
        };
    }
}

