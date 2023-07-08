#pragma once

#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/CommonNoDep.h"

namespace engine
{
    enum class Format;
    enum class CommandListType;
    enum class PrimitiveTopologyType;
    enum class InputClassification;
    enum class FillMode;
    enum class CullMode;
    enum class ImageTiling;
    enum class ResourceDimension;
    enum class ResourceState;
    enum class Blend;
    enum class BlendOperation;
    enum class ComparisonFunction;
    enum class StencilOp;
    enum class Filter;
    enum class TextureAddressMode;
    struct DepthStencilOpDescription;
    struct SubResource;
    struct SamplerDescription;
    class Texture;

    namespace implementation
    {
        enum class DescriptorType;

        VkShaderStageFlags vulkanShaderVisibility(ShaderVisibility flags);
        VkQueueFlags vulkanCommandListType(CommandListType type);
        VkFormat vulkanFormat(Format format);
        VkPrimitiveTopology vulkanPrimitiveTopologyType(PrimitiveTopologyType type);
        VkVertexInputRate vulkanInputClassification(InputClassification cls);
        VkPolygonMode vulkanFillMode(FillMode mode);
        VkCullModeFlags vulkanCullMode(CullMode mode);
        VkBufferUsageFlags vulkanBufferUsageFlags(BufferUsageFlags flags);
        VkMemoryPropertyFlags vulkanMemoryFlags(BufferMemoryFlags flags);
        VkDescriptorType vulkanDescriptorType(DescriptorType type);
        VkImageTiling vulkanImageTiling(ImageTiling tiling);
        //VkImageLayout vulkanImageLayout(ImageLayout layout);
        VkImageAspectFlags vulkanImageAspectFlags(ImageAspectFlags flags);
        VkImageType vulkanDimension(ResourceDimension dimension);
        VkImageAspectFlags vulkanFormatAspects(Format format);
        VkImageSubresourceRange vulkanSubResource(const Texture& texture, SubResource subResources);
        VkImageViewType vulkanViewType(ResourceDimension dimension);
        VkImageLayout vulkanResourceStates(ResourceState state);
        VkBlendFactor vulkanBlendFactor(Blend blend);
        VkBlendOp vulkanBlendOp(BlendOperation blendOp);
        VkCompareOp vulkanComparisonFunction(ComparisonFunction comp);
        VkStencilOp vulkanStencilOp(StencilOp op);
        VkStencilOpState vulkanStencilOpState(const DepthStencilOpDescription& op);
        VkFilter vulkanFilter(Filter filter);
        VkSamplerAddressMode vulkanAddressMode(TextureAddressMode mode);
        VkSamplerCreateInfo vulkanSamplerState(const SamplerDescription& desc);
		VkSampleCountFlagBits vulkanSamples(size_t samples);

        ShaderVisibility fromVulkanShaderVisibility(VkShaderStageFlags flags);
        CommandListType fromVulkanCommandListType(VkQueueFlags type);
        Format fromVulkanFormat(VkFormat format);
        PrimitiveTopologyType fromVulkanPrimitiveTopologyType(VkPrimitiveTopology type);
        InputClassification fromVulkanInputClassification(VkVertexInputRate cls);
        FillMode fromVulkanFillMode(VkPolygonMode mode);
        CullMode fromVulkanCullMode(VkCullModeFlags mode);
        BufferUsageFlags fromVulkanBufferUsageFlags(VkBufferUsageFlags flags);
        BufferMemoryFlags fromVulkanMemoryFlags(VkMemoryPropertyFlags flags);
        DescriptorType fromVulkanDescriptorType(VkDescriptorType type);
        ImageTiling fromVulkanImageTiling(VkImageTiling tiling);
        //ImageLayout fromVulkanImageLayout(VkImageLayout layout);
        ImageAspectFlags fromVulkanImageAspectFlags(VkImageAspectFlags flags);
        ResourceDimension fromVulkanDimension(VkImageType dimension);
        SubResource fromVulkanSubResource(VkImageSubresourceRange subResources);
        ResourceDimension fromVulkanViewType(VkImageViewType dimension);
        ResourceState fromVulkanResourceStates(VkImageLayout state);
        Blend fromVulkanBlendFactor(VkBlendFactor blend);
        BlendOperation fromVulkanBlendOp(VkBlendOp blendOp);
        ComparisonFunction fromVulkanComparisonFunction(VkCompareOp comp);
        StencilOp fromVulkanStencilOp(VkStencilOp op);
        DepthStencilOpDescription fromVulkanStencilOpState(const VkStencilOpState& op);
        SamplerDescription fromVulkanSamplerState(const VkSamplerCreateInfo& desc);
    }
}
