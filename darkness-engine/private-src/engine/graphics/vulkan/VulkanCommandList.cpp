#include "engine/graphics/vulkan/VulkanCommandList.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanSwapChain.h"
#include "engine/graphics/vulkan/VulkanDevice.h"
#include "engine/graphics/vulkan/VulkanCommandAllocator.h"
#include "engine/graphics/vulkan/VulkanPipeline.h"
#include "engine/graphics/vulkan/VulkanDescriptorHandle.h"
#include "engine/graphics/vulkan/VulkanResources.h"
#include "engine/graphics/vulkan/VulkanConversions.h"
#include "engine/graphics/vulkan/VulkanSemaphore.h"
#include "engine/graphics/vulkan/VulkanInstance.h"


#include "engine/graphics/Device.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/SwapChain.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Viewport.h"
#include "engine/graphics/Rect.h"
#include "engine/graphics/CommandList.h"

#include "engine/primitives/Color.h"
#include "engine/graphics/vulkan/VulkanQueue.h"
#include "engine/graphics/Queue.h"

#include "tools/Debug.h"
#include <array>

namespace engine
{
    namespace implementation
    {
        engine::string vulkanAccessFlagsToString(ResourceState state)
        {
            switch (state)
            {
            case ResourceState::Common: return "VK_ACCESS_SHADER_READ_BIT";
            case ResourceState::VertexAndConstantBuffer: return "VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT";
            case ResourceState::IndexBuffer: return "VK_ACCESS_INDEX_READ_BIT";
            case ResourceState::RenderTarget: return "VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT";
            case ResourceState::UnorderedAccess: return "VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT";
            case ResourceState::DepthWrite: return "VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT";
            case ResourceState::DepthRead: return "VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT";
            case ResourceState::NonPixelShaderResource: return "VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT";
            case ResourceState::PixelShaderResource: return "VK_ACCESS_SHADER_READ_BIT";
            case ResourceState::StreamOut: return "VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT";
            case ResourceState::IndirectArgument: return "VK_ACCESS_INDEX_READ_BIT";
            case ResourceState::CopyDest: return "VK_ACCESS_TRANSFER_WRITE_BIT";
            case ResourceState::CopySource: return "VK_ACCESS_TRANSFER_READ_BIT";
            case ResourceState::ResolveDest: return "VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT";
            case ResourceState::ResolveSource: return "VK_ACCESS_COLOR_ATTACHMENT_READ_BIT";
            case ResourceState::GenericRead: return "VK_ACCESS_COLOR_ATTACHMENT_READ_BIT";
            case ResourceState::Present: return "VK_ACCESS_COLOR_ATTACHMENT_READ_BIT";
            case ResourceState::Predication: return "VK_ACCESS_COLOR_ATTACHMENT_READ_BIT";
            }
            return "VK_ACCESS_SHADER_READ_BIT";
        }

        engine::string imageLayoutToString(VkImageLayout layout)
        {
            switch (layout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED: return "VK_IMAGE_LAYOUT_UNDEFINED";
            case VK_IMAGE_LAYOUT_GENERAL: return "VK_IMAGE_LAYOUT_GENERAL";
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return "VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL";
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL";
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return "VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL";
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return "VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL";
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return "VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL";
            case VK_IMAGE_LAYOUT_PREINITIALIZED: return "VK_IMAGE_LAYOUT_PREINITIALIZED";
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: return "VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return "VK_IMAGE_LAYOUT_PRESENT_SRC_KHR";
            case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR: return "VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR";
            case VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV: return "VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV";
            case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT: return "VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT";
            default: return "Unknown layout";
            }
        }

        VkAccessFlags vulkanAccessFlags(ResourceState state)
        {
            switch (state)
            {
            case ResourceState::Common: return VK_ACCESS_SHADER_READ_BIT;
            case ResourceState::VertexAndConstantBuffer: return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT;
            case ResourceState::IndexBuffer: return VK_ACCESS_INDEX_READ_BIT;
            case ResourceState::RenderTarget: return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            case ResourceState::UnorderedAccess: return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            case ResourceState::DepthWrite: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            case ResourceState::DepthRead: return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
            case ResourceState::NonPixelShaderResource: return VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT;
            case ResourceState::PixelShaderResource: return VK_ACCESS_SHADER_READ_BIT;
            case ResourceState::StreamOut: return VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT;
            case ResourceState::IndirectArgument: return VK_ACCESS_INDEX_READ_BIT;
            case ResourceState::CopyDest: return VK_ACCESS_TRANSFER_WRITE_BIT;
            case ResourceState::CopySource: return VK_ACCESS_TRANSFER_READ_BIT;
            case ResourceState::ResolveDest: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            case ResourceState::ResolveSource: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case ResourceState::GenericRead: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case ResourceState::Present: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case ResourceState::Predication: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            }
            return VK_ACCESS_SHADER_READ_BIT;
        }

        CommandListImplVulkan::CommandListImplVulkan(
            Device* device, 
            CommandListType type,
            const char* name)
            : m_device{ device }
            , m_type{ type }
            , m_clearColor{ 0.0f, 0.0f, 0.0f, 1.0f }
            , m_allocator{ static_pointer_cast<CommandAllocatorImplVulkan>(static_cast<DeviceImplVulkan*>(device->native())->createCommandAllocator(type, name)) }
            , m_lastAppliedImageBarrierIndex{ 0 }
            , m_lastAppliedBufferBarrierIndex{ 0 }
            , m_frameBufferTargets{}
            , m_frameBufferTargetsDSV{}
            , m_frameBuffers{}
            , m_currentFrameBufferRange{}
            , m_renderPassBeginInfo{}
            , m_open{ false }
			, m_computePipeline{ false }
			, m_activePipelineImpl{ nullptr }
			, m_activeConfiguration{ nullptr }
            , m_debugPassName{ nullptr }
            , m_nextUseInputLaytout{ false }
        {
            m_renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = m_allocator->native();
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            auto result = vkAllocateCommandBuffers(static_cast<DeviceImplVulkan*>(device->native())->device(), &allocInfo, &m_commandList);
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(m_commandList);
            debInfo.pObjectName = name;
            debInfo.objectType = VK_OBJECT_TYPE_COMMAND_BUFFER;
            result = SetDebugUtilsObjectNameEXT(
                static_cast<DeviceImplVulkan*>(device->native())->device(), &debInfo);
            ASSERT(result == VK_SUCCESS);

            begin();
        }

        CommandListAbs& CommandListImplVulkan::abs()
        {
            return m_abs;
        }

        CommandListType CommandListImplVulkan::type() const
        {
            return m_type;
        }

        void CommandListImplVulkan::reset(implementation::PipelineImplIf* /*pipelineState*/)
        {
            auto result = vkResetCommandBuffer(m_commandList, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
            ASSERT(result == VK_SUCCESS);
        }

        void CommandListImplVulkan::clear()
        {
            m_nextUseInputLaytout = false;
            m_lastAppliedImageBarrierIndex = 0;
            m_lastAppliedBufferBarrierIndex = 0;
            m_imageBarriers.clear();
            m_bufferBarriers.clear();
            m_abs.m_debugBuffers.clear();
            m_abs.m_lastSetRTVFormats.clear();

            vkResetCommandBuffer(m_commandList, 0 /*VkCommandBufferResetFlags*/);

            m_frameBuffers.clear();
            m_frameBufferTargets.clear();
            m_frameBufferTargetsDSV.clear();

            begin();
        }

        bool CommandListImplVulkan::isOpen() const
        {
            return m_open;
        }

        void CommandListImplVulkan::setRenderTargets(engine::vector<TextureRTV> targets)
        {
            applyBarriers();
            m_frameBufferTargets = targets;
            m_frameBufferTargetsDSV.clear();
        }

        void CommandListImplVulkan::setRenderTargets(engine::vector<TextureRTV> targets, TextureDSV dsv)
        {
            applyBarriers();
            m_frameBufferTargets = targets;
            m_frameBufferTargetsDSV.clear();
            m_frameBufferTargetsDSV.emplace_back(dsv);
        }

        void CommandListImplVulkan::setRenderTargets(TextureDSV target)
        {
            applyBarriers();
            m_frameBufferTargetsDSV.clear();
            m_frameBufferTargetsDSV.emplace_back(target);
        }

        void CommandListImplVulkan::copyBuffer(
            Buffer srcBuffer, 
            Buffer dstBuffer, 
            uint64_t elements, 
            size_t srcStartElement,
            size_t dstStartElement)
        {
            applyBarriers();

            auto dstElementSize = dstBuffer.description().elementSize;
            if (dstElementSize == -1)
                dstElementSize = static_cast<int32_t>(formatBytes(dstBuffer.description().format));

            auto srcElementSize = srcBuffer.description().elementSize;
            if (srcElementSize == -1)
                srcElementSize = static_cast<int32_t>(formatBytes(srcBuffer.description().format));

            VkBufferCopy copyRegion = {};
            copyRegion.size = static_cast<VkDeviceSize>(elements * srcElementSize);
            copyRegion.srcOffset = srcStartElement * srcElementSize;
            copyRegion.dstOffset = dstStartElement * dstElementSize;

            vkCmdCopyBuffer(
                m_commandList,
                static_cast<BufferImplVulkan*>(srcBuffer.m_impl)->native(),
                static_cast<BufferImplVulkan*>(dstBuffer.m_impl)->native(),
                1,
                &copyRegion);
        }

        void CommandListImplVulkan::copyBufferBytes(Buffer srcBuffer, Buffer dstBuffer, uint64_t bytes, size_t srcStartByte, size_t dstStartByte)
        {
            applyBarriers();

            VkBufferCopy copyRegion = {};
            copyRegion.size = static_cast<VkDeviceSize>(bytes);
            copyRegion.srcOffset = srcStartByte;
            copyRegion.dstOffset = dstStartByte;

            vkCmdCopyBuffer(
                m_commandList,
                static_cast<BufferImplVulkan*>(srcBuffer.m_impl)->native(),
                static_cast<BufferImplVulkan*>(dstBuffer.m_impl)->native(),
                1,
                &copyRegion);
        }

        void CommandListImplVulkan::clearBuffer(BufferUAV buffer, uint32_t value, size_t startElement, size_t numElements)
        {
            applyBarriers();

            auto elementSize = buffer.desc().elementSize;
            if (elementSize == -1)
                elementSize = static_cast<int32_t>(formatBytes(buffer.desc().format));

            if (elementSize < 4)
            {
                vkCmdFillBuffer(
                    m_commandList,
                    static_cast<BufferImplVulkan*>(buffer.buffer().m_impl)->native(),
                    startElement * elementSize,
                    VK_WHOLE_SIZE,
                    value);
            }
            else
                vkCmdFillBuffer(
                    m_commandList,
                    static_cast<BufferImplVulkan*>(buffer.buffer().m_impl)->native(),
                    startElement * elementSize,
                    numElements * elementSize,
                    value);
        }

        void CommandListImplVulkan::copyTexture(const TextureSRV src, TextureUAV dst)
        {
            applyBarriers();
            VkImageSubresourceLayers subResource = {};
            subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subResource.baseArrayLayer = 0;
            subResource.mipLevel = 0;
            subResource.layerCount = 1;

            VkImageCopy region = {};
            region.srcSubresource = subResource;
            region.dstSubresource = subResource;
            region.srcOffset = { 0, 0, 0 };
            region.dstOffset = { 0, 0, 0 };
            region.extent.width = static_cast<uint32_t>(src.width());
            region.extent.height = static_cast<uint32_t>(src.height());
            region.extent.depth = 1;

            vkCmdCopyImage(
                m_commandList,
                static_cast<TextureImplVulkan*>(src.texture().m_impl)->native(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                static_cast<TextureImplVulkan*>(dst.texture().m_impl)->native(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &region
            );
        }

        void CommandListImplVulkan::copyTexture(const TextureSRV src, TextureSRV dst)
        {
            applyBarriers();
            VkImageSubresourceLayers subResource = {};
            subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subResource.baseArrayLayer = 0;
            subResource.mipLevel = 0;
            subResource.layerCount = 1;

            VkImageCopy region = {};
            region.srcSubresource = subResource;
            region.dstSubresource = subResource;
            region.srcOffset = { 0, 0, 0 };
            region.dstOffset = { 0, 0, 0 };
            region.extent.width = static_cast<uint32_t>(src.width());
            region.extent.height = static_cast<uint32_t>(src.height());
            region.extent.depth = 1;

            vkCmdCopyImage(
                m_commandList,
                static_cast<TextureImplVulkan*>(src.texture().m_impl)->native(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                static_cast<TextureImplVulkan*>(dst.texture().m_impl)->native(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &region
            );
        }

        void CommandListImplVulkan::copyTexture(const TextureSRV src, TextureDSV dst)
        {
            applyBarriers();
            VkImageSubresourceLayers subResource = {};
            subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subResource.baseArrayLayer = 0;
            subResource.mipLevel = 0;
            subResource.layerCount = 1;

            VkImageCopy region = {};
            region.srcSubresource = subResource;
            region.dstSubresource = subResource;
            region.srcOffset = { 0, 0, 0 };
            region.dstOffset = { 0, 0, 0 };
            region.extent.width = static_cast<uint32_t>(src.width());
            region.extent.height = static_cast<uint32_t>(src.height());
            region.extent.depth = 1;

            vkCmdCopyImage(
                m_commandList,
                static_cast<TextureImplVulkan*>(src.texture().m_impl)->native(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                static_cast<TextureImplVulkan*>(dst.texture().m_impl)->native(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &region
            );
        }

        void CommandListImplVulkan::copyTexture(const TextureSRV /*src*/, BufferUAV /*dst*/)
        {
            ASSERT(false, "CommandListImplVulkan::copyTexture buffer UAV not implemented");
            applyBarriers();
        }

        void CommandListImplVulkan::copyTexture(const TextureSRV src, BufferSRV dst)
        {
            applyBarriers();

            VkBufferImageCopy region = {};
            region.bufferImageHeight = static_cast<uint32_t>(src.height());
            region.bufferOffset = 0;
            region.bufferRowLength = static_cast<uint32_t>(src.width());
            region.imageExtent = { static_cast<uint32_t>(src.width()), static_cast<uint32_t>(src.height()), 1u };
            region.imageOffset = { 0, 0, 0 };
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            vkCmdCopyImageToBuffer(m_commandList,
                static_cast<TextureImplVulkan*>(src.texture().m_impl)->native(),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                static_cast<BufferImplVulkan*>(dst.buffer().m_impl)->native(),
                1,
                &region);
        }

        void CommandListImplVulkan::copyTexture(
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
            size_t		/*dstMipCount*/)
        {
            applyBarriers();

            VkImageSubresourceLayers subResource = {};
            subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subResource.baseArrayLayer = static_cast<uint32_t>(dstSlice);
            subResource.mipLevel = static_cast<uint32_t>(dstMip);
            subResource.layerCount = 1;

            VkBufferImageCopy region = {};
            region.bufferOffset = static_cast<VkDeviceSize>(srcOffset);
            region.bufferRowLength = static_cast<uint32_t>(srcRowPitch);
            region.bufferImageHeight = static_cast<uint32_t>(srcHeight);
            region.imageSubresource = subResource;
            region.imageOffset = { static_cast<int32_t>(dstX), static_cast<int32_t>(dstY), 0 };
            region.imageExtent = {  static_cast<uint32_t>(srcWidth), static_cast<uint32_t>(srcHeight), 1u };

            vkCmdCopyBufferToImage(
                m_commandList,
                static_cast<BufferImplVulkan*>(srcBuffer.m_impl)->native(),
                static_cast<TextureImplVulkan*>(dst.m_impl)->native(),
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, 
                &region
            );
        }

        void CommandListImplVulkan::clearTextureUAV(TextureUAV texture, const Color4f& color)
        {
            applyBarriers();
            VkClearColorValue col = { color.red(), color.green(), color.blue(), color.alpha() };
            VkImageSubresourceRange range;
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.levelCount = texture.subResource().mipCount;
            range.layerCount = texture.subResource().arraySliceCount;
            range.baseMipLevel = texture.subResource().firstMipLevel;
            range.baseArrayLayer = texture.subResource().firstArraySlice;

            vkCmdClearColorImage(
                m_commandList,
                static_cast<TextureImplVulkan*>(texture.texture().m_impl)->native(),
                VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                &col,
                1,
                &range);
        }

        void CommandListImplVulkan::clearTextureDSV(TextureDSV texture, float depth, uint8_t stencil)
        {
			transition(texture, ResourceState::CopyDest);
            applyBarriers();
            VkClearDepthStencilValue value;
            value.depth = depth;
            value.stencil = static_cast<uint32_t>(stencil);
            
            auto subResource = texture.subResource();
            auto slices = subResource.arraySliceCount == AllArraySlices ? texture.texture().description().arraySlices : subResource.arraySliceCount;
            auto mips = subResource.mipCount == AllMipLevels ? texture.texture().description().mipLevels : subResource.mipCount;

            VkImageSubresourceRange subResourceRange[1];
            subResourceRange[0].baseArrayLayer = subResource.firstArraySlice;
            subResourceRange[0].baseMipLevel = subResource.firstMipLevel;
            subResourceRange[0].layerCount = static_cast<uint32_t>(slices);
            subResourceRange[0].levelCount = static_cast<uint32_t>(mips);
            subResourceRange[0].aspectMask = vulkanFormatAspects(texture.format());

            vkCmdClearDepthStencilImage(
                m_commandList, 
                static_cast<TextureImplVulkan*>(texture.texture().m_impl)->native(),
                VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                &value,
                1, subResourceRange);
        }

        void CommandListImplVulkan::clearTextureRTV(TextureRTV texture, const Color4f& color)
        {
			transition(texture, ResourceState::CopyDest);
            applyBarriers();
            VkClearColorValue col = { color.red(), color.green(), color.blue(), color.alpha() };
            VkImageSubresourceRange range;
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.levelCount = texture.subResource().mipCount;
            range.layerCount = texture.subResource().arraySliceCount;
            range.baseMipLevel = texture.subResource().firstMipLevel;
            range.baseArrayLayer = texture.subResource().firstArraySlice;

            vkCmdClearColorImage(
                m_commandList,
                static_cast<TextureImplVulkan*>(texture.texture().m_impl)->native(),
                VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                &col,
                1,
                &range);
        }

        void CommandListImplVulkan::clearTexture(Texture texture, const Color4f& color, const SubResource& subResource)
        {
            applyBarriers();
            VkClearColorValue clearColor;
            float r = color.red();
            float g = color.green();
            float b = color.blue();
            float a = color.alpha();

            clearColor.float32[0] = r;
            clearColor.float32[1] = g;
            clearColor.float32[2] = b;
            clearColor.float32[3] = a;

            clearColor.int32[0] = *reinterpret_cast<int32_t*>(&r);
            clearColor.int32[1] = *reinterpret_cast<int32_t*>(&g);
            clearColor.int32[2] = *reinterpret_cast<int32_t*>(&b);
            clearColor.int32[3] = *reinterpret_cast<int32_t*>(&a);

            clearColor.uint32[0] = *reinterpret_cast<uint32_t*>(&r);
            clearColor.uint32[1] = *reinterpret_cast<uint32_t*>(&g);
            clearColor.uint32[2] = *reinterpret_cast<uint32_t*>(&b);
            clearColor.uint32[3] = *reinterpret_cast<uint32_t*>(&a);

            auto slices = subResource.arraySliceCount == AllArraySlices ? texture.description().arraySlices : subResource.arraySliceCount;
            auto mips = subResource.mipCount == AllMipLevels ? texture.description().mipLevels : subResource.mipCount;

            VkImageSubresourceRange subResourceRange[1];
            subResourceRange[0].baseArrayLayer = subResource.firstArraySlice;
            subResourceRange[0].baseMipLevel = subResource.firstMipLevel;
            subResourceRange[0].layerCount = static_cast<uint32_t>(slices);
            subResourceRange[0].levelCount = static_cast<uint32_t>(mips);
            subResourceRange[0].aspectMask = vulkanFormatAspects(texture.format());

            //vkCmdClearDepthStencilImage()
            //vkCmdClearAttachments


            vkCmdClearColorImage(
                m_commandList,
                static_cast<TextureImplVulkan*>(texture.m_impl)->native(),
                VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                &clearColor,
                1, subResourceRange
            );
        }

        void CommandListImplVulkan::draw(size_t vertexCount)
        {
			bindPipeDeferred();
            applyBarriers();
            deferredRenderPassBegin();
            vkCmdDraw(m_commandList, static_cast<uint32_t>(vertexCount), 1u, 0u, 0u);

            endRenderPass();
        }

        void CommandListImplVulkan::drawIndirect(Buffer indirectArguments, uint64_t argumentBufferOffset)
        {
			bindPipeDeferred();
            applyBarriers();
            deferredRenderPassBegin();
            vkCmdDrawIndirect(
                m_commandList,
                static_cast<BufferImplVulkan*>(indirectArguments.m_impl)->native(),
                argumentBufferOffset,
                1, 0);

            endRenderPass();
        }

        void CommandListImplVulkan::drawIndexedInstanced(
            size_t indexCount,
            size_t instanceCount,
            size_t firstIndex,
            int32_t vertexOffset,
            size_t firstInstance)
        {
			bindPipeDeferred();
            applyBarriers();
            deferredRenderPassBegin();
            vkCmdDrawIndexed(
                m_commandList,
                static_cast<uint32_t>(indexCount),
                static_cast<uint32_t>(instanceCount),
                static_cast<uint32_t>(firstIndex),
                vertexOffset,
                static_cast<uint32_t>(firstInstance));

            endRenderPass();
        }

        void CommandListImplVulkan::drawIndexedIndirect(
            Buffer indirectArguments, uint64_t argumentBufferOffset)
        {
			bindPipeDeferred();
            applyBarriers();
            deferredRenderPassBegin();
            vkCmdDrawIndexedIndirect(m_commandList, static_cast<BufferImplVulkan*>(indirectArguments.m_impl)->native(), argumentBufferOffset, 1, 0);

            endRenderPass();
        }

        void CommandListImplVulkan::drawIndexedInstancedIndirect(
            BufferIBV indexBuffer,
            Buffer indirectArguments,
            uint64_t argumentBufferOffset,
            Buffer indirectArgumentsCountBuffer,
            uint64_t countBufferOffset)
        {
            bindPipeDeferred();
            bindIndexBuffer(indexBuffer);
            deferredRenderPassBegin();

            CmdDrawIndexedIndirectCountKHR(
                m_commandList,
                static_cast<BufferImplVulkan*>(indirectArguments.m_impl)->native(),
                argumentBufferOffset,
                static_cast<BufferImplVulkan*>(indirectArgumentsCountBuffer.m_impl)->native(),
                countBufferOffset,
                static_cast<uint32_t>(indirectArguments.description().elements),
                static_cast<uint32_t>(indirectArguments.description().elementSize));

            endRenderPass();
        }

        void CommandListImplVulkan::executeIndexedIndirect(
			BufferIBV indexBuffer, 
			Buffer indirectArguments, 
			uint64_t argumentBufferOffset, 
			Buffer countBuffer, 
			uint64_t countBufferOffsetBytes)
        {
			bindPipeDeferred();
			bindIndexBuffer(indexBuffer);
			deferredRenderPassBegin();

			CmdDrawIndexedIndirectCountKHR(
				m_commandList,
                static_cast<BufferImplVulkan*>(indirectArguments.m_impl)->native(),
				argumentBufferOffset,
                static_cast<BufferImplVulkan*>(countBuffer.m_impl)->native(),
				countBufferOffsetBytes,
				static_cast<uint32_t>(indirectArguments.description().elements),
				static_cast<uint32_t>(indirectArguments.description().elementSize));

            endRenderPass();
        }

        void CommandListImplVulkan::dispatch(
            size_t threadGroupCountX,
            size_t threadGroupCountY,
            size_t threadGroupCountZ)
        {
			bindPipeDeferred();
            applyBarriers();
            deferredRenderPassBegin();
            vkCmdDispatch(m_commandList, 
                static_cast<uint32_t>(threadGroupCountX), 
                static_cast<uint32_t>(threadGroupCountY), 
                static_cast<uint32_t>(threadGroupCountZ));

            endRenderPass();
        }

        void CommandListImplVulkan::dispatchIndirect(
            Buffer indirectArguments,
            uint64_t argumentBufferOffset)
        {
			bindPipeDeferred();
            applyBarriers();
            deferredRenderPassBegin();
            vkCmdDispatchIndirect(m_commandList, static_cast<BufferImplVulkan*>(indirectArguments.m_impl)->native(), argumentBufferOffset);

            endRenderPass();
        }

        void CommandListImplVulkan::executeBundle(
            CommandListImplIf* /*commandList*/)
        {
			bindPipeDeferred();
            applyBarriers();
            deferredRenderPassBegin();
            LOG_WARNING("CommandListImplVulkan::executeBundle not implemented");

            endRenderPass();
        }

        void CommandListImplVulkan::dispatchMesh(size_t /*threadGroupCountX*/, size_t /*threadGroupCountY*/, size_t /*threadGroupCountZ*/)
        {
            bindPipeDeferred();
            applyBarriers();
            deferredRenderPassBegin();
            LOG_WARNING("CommandListImplVulkan::dispatchMesh not implemented");

            endRenderPass();
        }

        void CommandListImplVulkan::transition(Texture resource, ResourceState state, const SubResource& subResource)
        {
            TextureImplVulkan* impl = static_cast<TextureImplVulkan*>(resource.m_impl);

            auto localSubRes = subResource;

            if ((localSubRes.arraySliceCount == AllArraySlices) &&
                (localSubRes.mipCount == AllMipLevels))
            {
                bool forceIndividual = false;
                auto firstState = impl->state(0, 0);
                for (int slice = 0; slice < static_cast<int>(resource.arraySlices()); ++slice)
                {
                    for (int mip = 0; mip < static_cast<int>(resource.mipLevels()); ++mip)
                    {
                        if (firstState != impl->state(slice, mip))
                        {
                            forceIndividual = true;
                            break;
                        }
                    }
                    if (forceIndividual)
                        break;
                }

                if (forceIndividual)
                {
                    localSubRes.firstArraySlice = 0;
                    localSubRes.firstMipLevel = 0;
                    localSubRes.arraySliceCount = static_cast<int32_t>(resource.arraySlices());
                    localSubRes.mipCount = static_cast<int32_t>(resource.mipLevels());
                }
            }

            if ((localSubRes.arraySliceCount == AllArraySlices) &&
                (localSubRes.mipCount == AllMipLevels))
            {
                ResourceState anystate = state;
                for (int slice = 0; slice < static_cast<int>(resource.arraySlices()); ++slice)
                {
                    for (int mip = 0; mip < static_cast<int>(resource.mipLevels()); ++mip)
                    {
                        if (impl->state(slice, mip) != anystate)
                        {
                            anystate = impl->state(slice, mip);
                            break;
                        }
                    }
                }
                if (anystate != state)
                {
                    uint32_t sliceCount = localSubRes.arraySliceCount == AllArraySlices ?
                        static_cast<uint32_t>(resource.arraySlices()) :
                        static_cast<uint32_t>(std::min(
                            localSubRes.arraySliceCount, 
                            static_cast<int32_t>(resource.arraySlices() - static_cast<size_t>(localSubRes.firstArraySlice))));

                    uint32_t mipCount = localSubRes.mipCount == AllMipLevels ?
                        static_cast<uint32_t>(resource.mipLevels()) :
                        static_cast<uint32_t>(std::min(localSubRes.mipCount, static_cast<int32_t>(resource.mipLevels() - static_cast<size_t>(localSubRes.firstMipLevel))));


                    VkImageMemoryBarrier barrier = {};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.oldLayout = vulkanResourceStates(anystate);
                    barrier.newLayout = vulkanResourceStates(state);
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = static_cast<TextureImplVulkan*>(resource.m_impl)->native();
                    barrier.srcAccessMask = vulkanAccessFlags(anystate);
                    barrier.dstAccessMask = vulkanAccessFlags(state);
                    barrier.subresourceRange.aspectMask = vulkanFormatAspects(resource.format());
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = mipCount;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = sliceCount;
                    m_imageBarriers.emplace_back(barrier);
                    /*LOG("TRANSITION. Texture (%p): %s, sub-resource(slices: [0 - %i], mips: [0 - %i]), from: %s, to: %s",
                        barrier.image,
                        resource.description().descriptor.name,
                        sliceCount - 1, mipCount - 1,
                        imageLayoutToString(vulkanResourceStates(anystate)).c_str(),
                        imageLayoutToString(vulkanResourceStates(state)).c_str());*/

                    for (int slice = 0; slice < static_cast<int>(resource.arraySlices()); ++slice)
                    {
                        for (int mip = 0; mip < static_cast<int>(resource.mipLevels()); ++mip)
                        {
                            impl->state(slice, mip, state);
                        }
                    }
                }
            }
            else
            {
                uint32_t sliceCount = localSubRes.arraySliceCount == AllArraySlices ?
                    static_cast<uint32_t>(resource.arraySlices()) :
                    static_cast<uint32_t>(std::min(localSubRes.arraySliceCount, static_cast<int32_t>(resource.arraySlices() - static_cast<size_t>(localSubRes.firstArraySlice))));

                uint32_t mipCount = localSubRes.mipCount == AllMipLevels ?
                    static_cast<uint32_t>(resource.mipLevels()) :
                    static_cast<uint32_t>(std::min(localSubRes.mipCount, static_cast<int32_t>(resource.mipLevels() - static_cast<size_t>(localSubRes.firstMipLevel))));

                for (int slice = static_cast<int>(localSubRes.firstArraySlice); slice < static_cast<int>(localSubRes.firstArraySlice + sliceCount); ++slice)
                {
                    for (int mip = static_cast<int>(localSubRes.firstMipLevel); mip < static_cast<int>(localSubRes.firstMipLevel + mipCount); ++mip)
                    {
                        if (impl->state(slice, mip) != state)
                        {
                            VkImageMemoryBarrier barrier = {};
                            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                            barrier.oldLayout = vulkanResourceStates(impl->state(slice, mip));
                            barrier.newLayout = vulkanResourceStates(state);
                            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                            barrier.image = static_cast<TextureImplVulkan*>(resource.m_impl)->native();
                            barrier.srcAccessMask = vulkanAccessFlags(impl->state(slice, mip));
                            barrier.dstAccessMask = vulkanAccessFlags(state);
                            barrier.subresourceRange.aspectMask = vulkanFormatAspects(resource.format());
                            barrier.subresourceRange.baseMipLevel = mip;
                            barrier.subresourceRange.levelCount = 1;
                            barrier.subresourceRange.baseArrayLayer = slice;
                            barrier.subresourceRange.layerCount = 1;

                            m_imageBarriers.emplace_back(barrier);
                            /*LOG("TRANSITION. Texture (%p): %s, sub-resource(slices: [%i - %i], mips: [%i - %i]), from: %s, to: %s",
                                barrier.image,
                                resource.description().descriptor.name,
                                slice, slice, mip, mip,
                                imageLayoutToString(vulkanResourceStates(impl->state(slice, mip))).c_str(),
                                imageLayoutToString(vulkanResourceStates(state)).c_str());*/

                            impl->state(slice, mip, state);
                        }
                    }
                }
            }

        }

        void CommandListImplVulkan::transition(TextureRTV resource, ResourceState state)
        {
            transition(resource.texture(), state, resource.subResource());
        }

        void CommandListImplVulkan::transition(TextureSRV resource, ResourceState state)
        {
            transition(resource.texture(), state);
        }

        void CommandListImplVulkan::transition(TextureDSV resource, ResourceState state)
        {
            transition(resource.texture(), state);
        }

        void CommandListImplVulkan::transition(Buffer resource, ResourceState state)
        {
            BufferImplVulkan* impl = static_cast<BufferImplVulkan*>(resource.m_impl);
            if (impl->state() == state)
                return;

            VkBufferMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.srcAccessMask = vulkanAccessFlags(impl->state());
            barrier.dstAccessMask = vulkanAccessFlags(state);
            barrier.buffer = static_cast<BufferImplVulkan*>(resource.m_impl)->native();
            barrier.offset = 0;
            barrier.size = VK_WHOLE_SIZE;
            m_bufferBarriers.emplace_back(barrier);

            impl->state(state);
        }

        void CommandListImplVulkan::transition(BufferSRV resource, ResourceState state)
        {
            transition(resource.buffer(), state);
        }

        void CommandListImplVulkan::transition(BufferIBV resource, ResourceState state)
        {
            transition(resource.buffer(), state);
        }

        void CommandListImplVulkan::transition(BufferCBV resource, ResourceState state)
        {
            transition(resource.buffer(), state);
        }

        void CommandListImplVulkan::transition(BufferVBV resource, ResourceState state)
        {
            transition(resource.buffer(), state);
        }

        void CommandListImplVulkan::setPredicate(BufferSRV buffer, uint64_t /*offset*/, PredicationOp /*op*/)
        {
            if (buffer.valid())
            {
                VkConditionalRenderingBeginInfoEXT condition = {};
                condition.sType = VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT;
                condition.buffer = static_cast<BufferImplVulkan*>(buffer.buffer().m_impl)->native();
                condition.offset = 0;

                CmdBeginConditionalRendering(m_commandList, &condition);
            }
            else
            {
                CmdEndConditionalRendering(m_commandList);
            }
        }

        void CommandListImplVulkan::applyBarriers()
        {
            auto imageBarrierCount = m_imageBarriers.size() - m_lastAppliedImageBarrierIndex;
            if (imageBarrierCount)
            {
                //LOG("BARRIER APPLY");
                for (int i = 0; i < imageBarrierCount; ++i)
                {
                    //LOG("Transition for resource: %p. from: %s, to: %s",
                    /*LOG("TRANSITION. Texture (%p): %s, sub-resource(slices: [%i - %i], mips: [%i - %i]), from: %s, to: %s",
                        m_imageBarriers[m_lastAppliedImageBarrierIndex + i].image,
                        "unknown",
                        m_imageBarriers[m_lastAppliedImageBarrierIndex + i].subresourceRange.baseArrayLayer,
                        m_imageBarriers[m_lastAppliedImageBarrierIndex + i].subresourceRange.baseArrayLayer + m_imageBarriers[m_lastAppliedImageBarrierIndex + i].subresourceRange.layerCount - 1,
                        m_imageBarriers[m_lastAppliedImageBarrierIndex + i].subresourceRange.baseMipLevel,
                        m_imageBarriers[m_lastAppliedImageBarrierIndex + i].subresourceRange.baseMipLevel + m_imageBarriers[m_lastAppliedImageBarrierIndex + i].subresourceRange.levelCount - 1,

                        imageLayoutToString(m_imageBarriers[m_lastAppliedImageBarrierIndex + i].oldLayout).c_str(),
                        imageLayoutToString(m_imageBarriers[m_lastAppliedImageBarrierIndex + i].newLayout).c_str()
                        );*/
                }

                engine::vector<VkImageMemoryBarrier> barriers;
                for (size_t i = m_lastAppliedImageBarrierIndex; i < m_lastAppliedImageBarrierIndex + imageBarrierCount; ++i)
                {
                    VkImageMemoryBarrier& toFind = m_imageBarriers[i];
                    auto found = std::find_if(barriers.begin(), barriers.end(), [&toFind](VkImageMemoryBarrier& barrier)
                        {
                            return barrier.image == toFind.image &&
                                barrier.subresourceRange.aspectMask == toFind.subresourceRange.aspectMask &&
                                barrier.subresourceRange.baseMipLevel == toFind.subresourceRange.baseMipLevel &&
                                barrier.subresourceRange.levelCount == toFind.subresourceRange.levelCount &&
                                barrier.subresourceRange.baseArrayLayer == toFind.subresourceRange.baseArrayLayer &&
                                barrier.subresourceRange.layerCount == toFind.subresourceRange.layerCount;
                        });
                    if (found != barriers.end())
                    {
                        found->newLayout = toFind.newLayout;
                    }
                    else
                        barriers.emplace_back(toFind);
                }

                vkCmdPipelineBarrier(
                    m_commandList,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    static_cast<uint32_t>(barriers.size()), &barriers[0]);
                //LOG("BARRIER APPLY END");

                m_lastAppliedImageBarrierIndex = m_imageBarriers.size();
            }

            auto bufferBarrierCount = m_bufferBarriers.size() - m_lastAppliedBufferBarrierIndex;
            if (bufferBarrierCount)
            {
                vkCmdPipelineBarrier(
                    m_commandList,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0, nullptr,
                    static_cast<uint32_t>(bufferBarrierCount), &m_bufferBarriers[m_lastAppliedBufferBarrierIndex],
                    0, nullptr
                );

                m_lastAppliedBufferBarrierIndex = m_bufferBarriers.size();
            }
        }

        void CommandListImplVulkan::bindPipe(
			implementation::PipelineImplIf* pipelineImpl,
            shaders::PipelineConfiguration* configuration)
		{
			m_activePipelineImpl = static_cast<PipelineImplVulkan*>(pipelineImpl);
			m_activeConfiguration = configuration;
            m_activePipelineImpl->m_useInputLaytout = m_nextUseInputLaytout;
		}

		void CommandListImplVulkan::bindPipeDeferred()
        {
			if (!m_activePipelineImpl || !m_activeConfiguration)
				return;

            applyBarriers();

			if (m_activeConfiguration->hasComputeShader())
				m_computePipeline = true;
			else
				m_computePipeline = false;

			m_activePipelineImpl->renderPass(m_activeConfiguration).updateAttachments(
                m_frameBufferTargets,
                m_frameBufferTargetsDSV);
			m_activePipelineImpl->configure(this, m_activeConfiguration);

			m_activePipelineImpl->finalize(*this, m_activeConfiguration);

            m_renderPass = &m_activePipelineImpl->renderPass().renderPass();
            m_pipeline = m_activePipelineImpl->m_currentPipelineCache->m_pipeline.get();
            m_pipelineLayout = m_activePipelineImpl->m_pipelineLayout.get();
            m_descriptorSet = &m_activePipelineImpl->descriptorSet();

			m_activePipelineImpl = nullptr;
			m_activeConfiguration = nullptr;
        }

        void CommandListImplVulkan::deferredRenderPassBegin()
        {
            VkDebugUtilsObjectNameInfoEXT debInfo = {};
            debInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo.objectHandle = reinterpret_cast<uint64_t>(*m_renderPass);
            debInfo.pObjectName = m_debugPassName;
            debInfo.objectType = VK_OBJECT_TYPE_RENDER_PASS;
            auto result = SetDebugUtilsObjectNameEXT(
                static_cast<DeviceImplVulkan*>(m_device->native())->device(), &debInfo);
            ASSERT(result == VK_SUCCESS);

            VkDebugUtilsObjectNameInfoEXT debInfo2 = {};
            debInfo2.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            debInfo2.objectHandle = reinterpret_cast<uint64_t>(*m_pipeline);
            debInfo2.pObjectName = m_debugPassName;
            debInfo2.objectType = VK_OBJECT_TYPE_PIPELINE;
            result = SetDebugUtilsObjectNameEXT(
                static_cast<DeviceImplVulkan*>(m_device->native())->device(), &debInfo2);
            ASSERT(result == VK_SUCCESS);

            beginRenderPass(nullptr, 0);

			if (m_computePipeline)
			{
				vkCmdBindPipeline(
					m_commandList,
					VK_PIPELINE_BIND_POINT_COMPUTE,
					*m_pipeline);
			}
			else
			{
				vkCmdBindPipeline(
					m_commandList,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					*m_pipeline);
			}

            bindDescriptorSets();
        }

        void CommandListImplVulkan::setViewPorts(const engine::vector<Viewport>& viewports)
        {
            /*for (auto&& viewport : viewports)
            {
                m_renderPassBeginInfo.renderArea.offset = { static_cast<int32_t>(viewport.topLeftX), static_cast<int32_t>(viewport.topLeftY) };
                m_renderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(viewport.width), static_cast<uint32_t>(viewport.height) };
            }*/
            m_renderPassBeginInfo.renderArea.offset = { static_cast<int32_t>(viewports[0].topLeftX), static_cast<int32_t>(viewports[0].topLeftY) };
            m_renderPassBeginInfo.renderArea.extent = { static_cast<uint32_t>(viewports[0].width), static_cast<uint32_t>(viewports[0].height) };

            engine::vector<VkViewport> viewPorts;
            for (auto& v : viewports)
            {
                VkViewport viewPort;
                viewPort.width = v.width;
                viewPort.height = -v.height;
                viewPort.maxDepth = v.maxDepth;
                viewPort.minDepth = v.minDepth;
                viewPort.x = v.topLeftX;
                viewPort.y = v.height;
                viewPorts.emplace_back(viewPort);
            }
            vkCmdSetViewport(m_commandList, 0, static_cast<uint32_t>(viewPorts.size()), &viewPorts[0]);
        }

        void CommandListImplVulkan::setScissorRects(const engine::vector<Rectangle>& rects)
        {
            m_scissorRects.clear();
            for (auto& rect : rects)
            {
                VkRect2D r;
                r.extent.width = rect.right > rect.left ? rect.right - rect.left : 0;
                r.extent.height = rect.bottom > rect.top ? rect.bottom - rect.top : 0;
                r.offset.x = rect.left;
                r.offset.y = rect.top;

                if(r.offset.x < 0) r.offset.x = 0;
                if(r.offset.y < 0) r.offset.y = 0;

                m_scissorRects.emplace_back(r);
            }
        }

        void CommandListImplVulkan::bindVertexBuffer(const BufferVBV buffer)
        {
            applyBarriers();
            VkBuffer vertexBuffers[] = { static_cast<BufferImplVulkan*>(buffer.buffer().m_impl)->native() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(m_commandList, 0, 1, vertexBuffers, offsets);
            m_nextUseInputLaytout = true;
        }

        void CommandListImplVulkan::bindIndexBuffer(const BufferIBV buffer)
        {
            applyBarriers();
            vkCmdBindIndexBuffer(
                m_commandList,
                static_cast<BufferImplVulkan*>(buffer.buffer().m_impl)->native(),
                0,
                (buffer.desc().elementSize == 2) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
            m_nextUseInputLaytout = true;
        }

        void CommandListImplVulkan::setStructureCounter(BufferUAV /*buffer*/, uint32_t /*value*/)
        {
            LOG_WARNING("CommandListImplVulkan::setStructureCounter not implemented");
            applyBarriers();
        }

        void CommandListImplVulkan::copyStructureCounter(BufferUAV /*srcBuffer*/, Buffer /*dst*/, uint32_t /*dstByteOffset*/)
        {
            LOG_WARNING("CommandListImplVulkan::copyStructureCounter not implemented");
            applyBarriers();
        }

        void CommandListImplVulkan::begin()
        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0;// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(m_commandList, &beginInfo);
            m_open = true;
        }

        void CommandListImplVulkan::end()
        {
            auto result = vkEndCommandBuffer(m_commandList);
            ASSERT(result == VK_SUCCESS);
            m_open = false;
        }

        void CommandListImplVulkan::beginRenderPass(implementation::PipelineImplIf* /*pipeline*/, int /*frameBufferIndex*/)
        {
			if (m_computePipeline)
				return;

            engine::vector<VkImageView> attachments;
            for (auto& rtv : m_frameBufferTargets)
            {
                attachments.emplace_back(static_cast<TextureRTVImplVulkan*>(rtv.m_impl)->native());
            }
            for (auto& dsv : m_frameBufferTargetsDSV)
            {
                attachments.emplace_back(static_cast<TextureDSVImplVulkan*>(dsv.m_impl)->native());
            }

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = *m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();

            auto frameWidth = 0;
            auto frameHeight = 0;

			if (m_frameBufferTargets.size() > 0)
			{
                frameWidth = std::max(static_cast<uint32_t>(m_frameBufferTargets[0].width()) >> static_cast<uint32_t>(m_frameBufferTargets[0].subResource().firstMipLevel), 1u);
                frameHeight = std::max(static_cast<uint32_t>(m_frameBufferTargets[0].height()) >> static_cast<uint32_t>(m_frameBufferTargets[0].subResource().firstMipLevel), 1u);

				framebufferInfo.width = frameWidth;
				framebufferInfo.height = frameHeight;
			}
			else if (m_frameBufferTargetsDSV.size() > 0)
			{
                frameWidth = std::max(static_cast<uint32_t>(m_frameBufferTargetsDSV[0].width()) >> static_cast<uint32_t>(m_frameBufferTargetsDSV[0].subResource().firstMipLevel), 1u);
                frameHeight = std::max(static_cast<uint32_t>(m_frameBufferTargetsDSV[0].height()) >> static_cast<uint32_t>(m_frameBufferTargetsDSV[0].subResource().firstMipLevel), 1u);
				framebufferInfo.width = frameWidth;
				framebufferInfo.height = frameHeight;
			}
            framebufferInfo.layers = 1;

            auto frameBuffer = vulkanPtr<VkFramebuffer>(static_cast<DeviceImplVulkan*>(m_device->native())->device(), vkDestroyFramebuffer);
            auto result = vkCreateFramebuffer(static_cast<DeviceImplVulkan*>(m_device->native())->device(), &framebufferInfo, nullptr, frameBuffer.get());
            ASSERT(result == VK_SUCCESS);
            m_frameBuffers.emplace_back(std::move(frameBuffer));


            m_renderPassBeginInfo.renderPass = *m_renderPass;
            m_renderPassBeginInfo.framebuffer = *m_frameBuffers.back();// pipeline.m_framebuffers[static_cast<std::size_t>(frameBufferIndex)];

            engine::vector<VkClearValue> clearValues(m_frameBufferTargets.size() + m_frameBufferTargetsDSV.size());
			for(int i = 0; i < m_frameBufferTargets.size(); ++i)
				clearValues[i] = m_clearColor;

			for (size_t i = m_frameBufferTargets.size(); i < m_frameBufferTargets.size() + m_frameBufferTargetsDSV.size(); ++i)
				clearValues[i] = m_clearColor;

            m_renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            m_renderPassBeginInfo.pClearValues = clearValues.data();

            if (m_frameBufferTargets.size() > 0)
            {
                setViewPorts({
                    engine::Viewport{
                    0, 0,
                    static_cast<float>(frameWidth),
                    static_cast<float>(frameHeight),
                    0.0f, 1.0f } });

                if (m_scissorRects.size() > 0)
                {
                    vkCmdSetScissor(m_commandList, 0, static_cast<uint32_t>(m_scissorRects.size()), &m_scissorRects[0]);
                }
                else
                {
                    setScissorRects({ engine::Rectangle{ 0, 0,
                        frameWidth,
                        frameHeight } });
                    vkCmdSetScissor(m_commandList, 0, static_cast<uint32_t>(m_scissorRects.size()), &m_scissorRects[0]);
                }
            }
            else if (m_frameBufferTargetsDSV.size() > 0)
            {
                setViewPorts({
                    engine::Viewport{
                    0, 0,
                    static_cast<float>(frameWidth),
                    static_cast<float>(frameHeight),
                    0.0f, 1.0f } });

                if (m_scissorRects.size() > 0)
                {
                    vkCmdSetScissor(m_commandList, 0, static_cast<uint32_t>(m_scissorRects.size()), &m_scissorRects[0]);
                }
                else
                {
                    setScissorRects({ engine::Rectangle{ 0, 0,
                        frameWidth,
                        frameHeight } });
                    vkCmdSetScissor(m_commandList, 0, static_cast<uint32_t>(m_scissorRects.size()), &m_scissorRects[0]);
                }
            }

            vkCmdBeginRenderPass(m_commandList, &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);


            
        }

        void CommandListImplVulkan::endRenderPass()
        {
			if (m_computePipeline)
				return;

            vkCmdEndRenderPass(m_commandList);

            m_scissorRects.clear();
            //m_frameBufferTargetsDSV.clear();
            //m_frameBufferTargets.clear();

            m_nextUseInputLaytout = false;
        }

        VkCommandBuffer& CommandListImplVulkan::native()
        {
            return m_commandList;
        }

        const VkCommandBuffer& CommandListImplVulkan::native() const
        {
            return m_commandList;
        }

        void CommandListImplVulkan::setNextPassDebugName(const char* name)
        {
            m_debugPassName = name;
        }

        void CommandListImplVulkan::bindDescriptorSets()
        {
            if (m_descriptorSet->size() > 0)
            {
				if (m_computePipeline)
				{
					vkCmdBindDescriptorSets(
						m_commandList,
						VK_PIPELINE_BIND_POINT_COMPUTE,
						*m_pipelineLayout,
						0,
                        static_cast<uint32_t>(m_descriptorSet->size()),
						(*m_descriptorSet).data(),
						0, nullptr);
				}
				else
				{
					vkCmdBindDescriptorSets(
						m_commandList,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						*m_pipelineLayout,
						0,
                        static_cast<uint32_t>(m_descriptorSet->size()),
						(*m_descriptorSet).data(),
						0, nullptr);
				}
            }
        }

        VkAccessFlags getAccessFlagsFromLayout(VkImageLayout layout)
        {
            switch (layout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case VK_IMAGE_LAYOUT_GENERAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case VK_IMAGE_LAYOUT_PREINITIALIZED: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            }
            return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        }

        
    }
}
