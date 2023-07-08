#include "engine/graphics/vulkan/VulkanConversions.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanPipeline.h"
#include "engine/graphics/Common.h"
#include "engine/graphics/Format.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Pipeline.h"
#include "engine/graphics/SamplerDescription.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        VkQueueFlags vulkanCommandListType(CommandListType type)
        {
            switch (type)
            {
                case CommandListType::Direct: return VK_QUEUE_GRAPHICS_BIT;
                case CommandListType::Compute: return VK_QUEUE_COMPUTE_BIT;
                case CommandListType::Copy: return VK_QUEUE_TRANSFER_BIT;
            }
            return VK_QUEUE_GRAPHICS_BIT;
        }

        VkFormat vulkanFormat(Format format)
        {
            switch (format)
            {
                case Format::UNKNOWN: return VK_FORMAT_UNDEFINED;
                case Format::R32G32B32A32_TYPELESS: return VK_FORMAT_R32G32B32A32_UINT;
                case Format::R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
                case Format::R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
                case Format::R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
                case Format::R32G32B32_TYPELESS: return VK_FORMAT_R32G32B32_UINT;
                case Format::R32G32B32_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;// VK_FORMAT_R32G32B32_SFLOAT;
                case Format::R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
                case Format::R32G32B32_SINT: return VK_FORMAT_R32G32B32_SINT;
                case Format::R16G16B16A16_TYPELESS: return VK_FORMAT_R16G16B16A16_UNORM;
                case Format::R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
                case Format::R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
                case Format::R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
                case Format::R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
                case Format::R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
                case Format::R32G32_TYPELESS: return VK_FORMAT_R32G32_UINT;
                case Format::R32G32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;
                case Format::R32G32_UINT: return VK_FORMAT_R32G32_UINT;
                case Format::R32G32_SINT: return VK_FORMAT_R32G32_SINT;
                case Format::R32G8X24_TYPELESS: return VK_FORMAT_D32_SFLOAT_S8_UINT;
                case Format::D32_FLOAT_S8X24_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
                case Format::R32_FLOAT_X8X24_TYPELESS: return VK_FORMAT_D32_SFLOAT_S8_UINT;
                case Format::X32_TYPELESS_G8X24_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
                case Format::R10G10B10A2_TYPELESS: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
                case Format::R10G10B10A2_UNORM: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
                case Format::R10G10B10A2_UINT: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
                case Format::R11G11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
                case Format::R8G8B8A8_TYPELESS: return VK_FORMAT_R8G8B8A8_UNORM;
                case Format::R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
                case Format::R8G8B8A8_UNORM_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
                case Format::R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
                case Format::R8G8B8A8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
                case Format::R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
                case Format::R16G16_TYPELESS: return VK_FORMAT_R16G16_SFLOAT;
                case Format::R16G16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;
                case Format::R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
                case Format::R16G16_UINT: return VK_FORMAT_R16G16_UINT;
                case Format::R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
                case Format::R16G16_SINT: return VK_FORMAT_R16G16_SINT;
                case Format::R32_TYPELESS: return VK_FORMAT_D32_SFLOAT;
                case Format::D32_FLOAT: return VK_FORMAT_D32_SFLOAT;
                case Format::R32_FLOAT: return VK_FORMAT_R32_SFLOAT;
                case Format::R32_UINT: return VK_FORMAT_R32_UINT;
                case Format::R32_SINT: return VK_FORMAT_R32_SINT;
                case Format::R24G8_TYPELESS: return VK_FORMAT_D24_UNORM_S8_UINT;
                case Format::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
                case Format::R24_UNORM_X8_TYPELESS: return VK_FORMAT_D24_UNORM_S8_UINT;
                case Format::X24_TYPELESS_G8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
                case Format::R8G8_TYPELESS: return VK_FORMAT_R8G8_UNORM;
                case Format::R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
                case Format::R8G8_UINT: return VK_FORMAT_R8G8_UINT;
                case Format::R8G8_SNORM: return VK_FORMAT_R8G8_SNORM;
                case Format::R8G8_SINT: return VK_FORMAT_R8G8_SINT;
                case Format::R16_TYPELESS: return VK_FORMAT_R16_UNORM;
                case Format::R16_FLOAT: return VK_FORMAT_R16_SFLOAT;
                case Format::D16_UNORM: return VK_FORMAT_R16_UNORM;
                case Format::R16_UNORM: return VK_FORMAT_R16_UNORM;
                case Format::R16_UINT: return VK_FORMAT_R16_UINT;
                case Format::R16_SNORM: return VK_FORMAT_R16_SNORM;
                case Format::R16_SINT: return VK_FORMAT_R16_SINT;
                case Format::R8_TYPELESS: return VK_FORMAT_R8_UNORM;
                case Format::R8_UNORM: return VK_FORMAT_R8_UNORM;
                case Format::R8_UINT: return VK_FORMAT_R8_UINT;
                case Format::R8_SNORM: return VK_FORMAT_R8_SNORM;
                case Format::R8_SINT: return VK_FORMAT_R8_SINT;
                case Format::A8_UNORM: return VK_FORMAT_R8_UNORM;
                case Format::R1_UNORM: return VK_FORMAT_R8_UNORM;
                case Format::R9G9B9E5_SHAREDEXP: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
                case Format::R8G8_B8G8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
                case Format::G8R8_G8B8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
                case Format::BC1_TYPELESS: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
                case Format::BC1_UNORM: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
                case Format::BC1_UNORM_SRGB: return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
                case Format::BC2_TYPELESS: return VK_FORMAT_BC2_UNORM_BLOCK;
                case Format::BC2_UNORM: return VK_FORMAT_BC2_UNORM_BLOCK;
                case Format::BC2_UNORM_SRGB: return VK_FORMAT_BC2_SRGB_BLOCK;
                case Format::BC3_TYPELESS: return VK_FORMAT_BC3_UNORM_BLOCK;
                case Format::BC3_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
                case Format::BC3_UNORM_SRGB: return VK_FORMAT_BC3_SRGB_BLOCK;
                case Format::BC4_TYPELESS: return VK_FORMAT_BC4_UNORM_BLOCK;
                case Format::BC4_UNORM: return VK_FORMAT_BC4_UNORM_BLOCK;
                case Format::BC4_SNORM: return VK_FORMAT_BC4_SNORM_BLOCK;
                case Format::BC5_TYPELESS: return VK_FORMAT_BC5_UNORM_BLOCK;
                case Format::BC5_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
                case Format::BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
                case Format::B5G6R5_UNORM: return VK_FORMAT_R5G6B5_UNORM_PACK16;
                case Format::B5G5R5A1_UNORM: return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
                case Format::B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
                case Format::B8G8R8X8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
                case Format::R10G10B10_XR_BIAS_A2_UNORM: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
                case Format::B8G8R8A8_TYPELESS: return VK_FORMAT_B8G8R8A8_SRGB;
                case Format::B8G8R8A8_UNORM_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
                case Format::B8G8R8X8_TYPELESS: return VK_FORMAT_B8G8R8A8_SRGB;
                case Format::B8G8R8X8_UNORM_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
                case Format::BC6H_TYPELESS: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
                case Format::BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
                case Format::BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
                case Format::BC7_TYPELESS: return VK_FORMAT_BC7_UNORM_BLOCK;
                case Format::BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
                case Format::BC7_UNORM_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
                case Format::AYUV: return VK_FORMAT_UNDEFINED;
                case Format::Y410: return VK_FORMAT_UNDEFINED;
                case Format::Y416: return VK_FORMAT_UNDEFINED;
                case Format::NV12: return VK_FORMAT_UNDEFINED;
                case Format::P010: return VK_FORMAT_UNDEFINED;
                case Format::P016: return VK_FORMAT_UNDEFINED;
                case Format::OPAQUE_420: return VK_FORMAT_UNDEFINED;
                case Format::YUY2: return VK_FORMAT_UNDEFINED;
                case Format::Y210: return VK_FORMAT_UNDEFINED;
                case Format::Y216: return VK_FORMAT_UNDEFINED;
                case Format::NV11: return VK_FORMAT_UNDEFINED;
                case Format::AI44: return VK_FORMAT_UNDEFINED;
                case Format::IA44: return VK_FORMAT_UNDEFINED;
                case Format::P8: return VK_FORMAT_UNDEFINED;
                case Format::A8P8: return VK_FORMAT_UNDEFINED;
                case Format::B4G4R4A4_UNORM: return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
                case Format::P208: return VK_FORMAT_UNDEFINED;
                case Format::V208: return VK_FORMAT_UNDEFINED;
                case Format::V408: return VK_FORMAT_UNDEFINED;
                default: return VK_FORMAT_UNDEFINED;
            }
        }

        VkPrimitiveTopology vulkanPrimitiveTopologyType(PrimitiveTopologyType type)
        {
            switch (type)
            {
                case PrimitiveTopologyType::Undefined: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                case PrimitiveTopologyType::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                case PrimitiveTopologyType::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                case PrimitiveTopologyType::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				case PrimitiveTopologyType::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
                case PrimitiveTopologyType::PatchList1: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            }
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        }

        VkVertexInputRate vulkanInputClassification(InputClassification cls)
        {
            switch (cls)
            {
                case InputClassification::PerVertexData: return VK_VERTEX_INPUT_RATE_VERTEX;
                case InputClassification::PerInstanceData: return VK_VERTEX_INPUT_RATE_INSTANCE;
            }
            return VK_VERTEX_INPUT_RATE_VERTEX;
        }

        VkPolygonMode vulkanFillMode(FillMode mode)
        {
            switch (mode)
            {
                case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;
                case FillMode::Solid: return VK_POLYGON_MODE_FILL;
                case FillMode::Point: return VK_POLYGON_MODE_POINT;
            }
            return VK_POLYGON_MODE_LINE;
        }

        VkCullModeFlags vulkanCullMode(CullMode mode)
        {
            switch (mode)
            {
                case CullMode::None: return VK_CULL_MODE_NONE;
                case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
                case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
            }
            return VK_CULL_MODE_NONE;
        }

        VkBufferUsageFlags vulkanBufferUsageFlags(BufferUsageFlags flags)
        {
            bool src = (flags & static_cast<uint32_t>(BufferUsageFlagBits::Src)) > 0;
            bool dst = (flags & static_cast<uint32_t>(BufferUsageFlagBits::Dst)) > 0;
            bool uniformTexel = (flags & static_cast<uint32_t>(BufferUsageFlagBits::UniformTexel)) > 0;
            bool storageTexel = (flags & static_cast<uint32_t>(BufferUsageFlagBits::StorageTexel)) > 0;
            bool uniform = (flags & static_cast<uint32_t>(BufferUsageFlagBits::Uniform)) > 0;
            bool storage = (flags & static_cast<uint32_t>(BufferUsageFlagBits::Storage)) > 0;
            bool index = (flags & static_cast<uint32_t>(BufferUsageFlagBits::Index)) > 0;
            bool vertex = (flags & static_cast<uint32_t>(BufferUsageFlagBits::Vertex)) > 0;
            bool indirect = (flags & static_cast<uint32_t>(BufferUsageFlagBits::Indirect)) > 0;
            VkBufferUsageFlags returnFlags{};
            returnFlags |= src ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : 0;
            returnFlags |= dst ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : 0;
            returnFlags |= uniformTexel ? VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT : 0;
            returnFlags |= storageTexel ? VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT : 0;
            returnFlags |= uniform ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0;
            returnFlags |= storage ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : 0;
            returnFlags |= index ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;
            returnFlags |= vertex ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
            returnFlags |= indirect ? VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT : 0;
            return returnFlags;
        }

        VkMemoryPropertyFlags vulkanMemoryFlags(BufferMemoryFlags flags)
        {
            bool deviceLocal = (flags & static_cast<uint32_t>(BufferMemoryFlagBits::DeviceLocal)) > 0;
            bool hostVisible = (flags & static_cast<uint32_t>(BufferMemoryFlagBits::HostVisible)) > 0;
            bool hostCoherent = (flags & static_cast<uint32_t>(BufferMemoryFlagBits::HostCoherent)) > 0;
            bool hostCached = (flags & static_cast<uint32_t>(BufferMemoryFlagBits::HostCached)) > 0;
            bool lazy = (flags & static_cast<uint32_t>(BufferMemoryFlagBits::LazilyAllocated)) > 0;
            VkMemoryPropertyFlags returnFlags{};
            returnFlags |= deviceLocal ? VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : 0;
            returnFlags |= hostVisible ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : 0;
            returnFlags |= hostCoherent ? VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : 0;
            returnFlags |= hostCached ? VK_MEMORY_PROPERTY_HOST_CACHED_BIT : 0;
            returnFlags |= lazy ? VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : 0;
            return returnFlags;
        }

        VkDescriptorType vulkanDescriptorType(DescriptorType type)
        {
            switch (type)
            {
                case DescriptorType::Sampler: return VK_DESCRIPTOR_TYPE_SAMPLER;
                case DescriptorType::CombinedImageSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                case DescriptorType::SampledImage: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                case DescriptorType::StorageImage: return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
                case DescriptorType::UniformTexelBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                case DescriptorType::StorageTexelBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                case DescriptorType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                case DescriptorType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                case DescriptorType::UniformBufferDynamic: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                case DescriptorType::StorageBufferDynamic: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                case DescriptorType::InputAttachment: return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            }
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }

        VkImageTiling vulkanImageTiling(ImageTiling tiling)
        {
            switch (tiling)
            {
                case ImageTiling::Optimal: return VK_IMAGE_TILING_OPTIMAL;
                case ImageTiling::Linear: return VK_IMAGE_TILING_LINEAR;
            }
            return VK_IMAGE_TILING_OPTIMAL;
        }

        /*VkImageLayout vulkanImageLayout(ImageLayout layout)
        {
            switch (layout)
            {
                case ImageLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
                case ImageLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
                case ImageLayout::ColorAttachmentOptimal: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                case ImageLayout::DepthStencilAttachmentOptimal: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                case ImageLayout::DepthStencilReadOnlyOptimal: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                case ImageLayout::ShaderReadOnlyOptimal: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                case ImageLayout::TransferSrcOptimal: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                case ImageLayout::TransferDstOptimal: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                case ImageLayout::Preinitialized: return VK_IMAGE_LAYOUT_PREINITIALIZED;
                case ImageLayout::PresentSrc: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
            return VK_IMAGE_LAYOUT_UNDEFINED;
        }*/

        VkShaderStageFlags vulkanShaderVisibility(ShaderVisibility flags)
        {
            if (flags == 0x7FFFFFFF)
                return VK_SHADER_STAGE_ALL;
            if (flags == 0x0000001F)
                return VK_SHADER_STAGE_ALL_GRAPHICS;

            bool vertex = (flags & static_cast<uint32_t>(ShaderVisibilityBits::Vertex)) > 0;
            bool tesselationControl = (flags & static_cast<uint32_t>(ShaderVisibilityBits::Hull)) > 0;
            bool tesselationEval = (flags & static_cast<uint32_t>(ShaderVisibilityBits::Domain)) > 0;
            bool geometry = (flags & static_cast<uint32_t>(ShaderVisibilityBits::Geometry)) > 0;
            bool fragment = (flags & static_cast<uint32_t>(ShaderVisibilityBits::Pixel)) > 0;
            bool compute = (flags & static_cast<uint32_t>(ShaderVisibilityBits::Compute)) > 0;
            VkShaderStageFlags returnFlags{};
            returnFlags |= vertex ? VK_SHADER_STAGE_VERTEX_BIT : 0;
            returnFlags |= tesselationControl ? VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT : 0;
            returnFlags |= tesselationEval ? VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT : 0;
            returnFlags |= geometry ? VK_SHADER_STAGE_GEOMETRY_BIT : 0;
            returnFlags |= fragment ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
            returnFlags |= compute ? VK_SHADER_STAGE_COMPUTE_BIT : 0;
            return returnFlags;
        }

        VkImageAspectFlags vulkanImageAspectFlags(ImageAspectFlags flags)
        {
            bool color = (flags & static_cast<uint32_t>(ImageAspectFlagBits::Color)) > 0;
            bool depth = (flags & static_cast<uint32_t>(ImageAspectFlagBits::Depth)) > 0;
            bool stencil = (flags & static_cast<uint32_t>(ImageAspectFlagBits::Stencil)) > 0;
            bool metadata = (flags & static_cast<uint32_t>(ImageAspectFlagBits::Metadata)) > 0;
            VkImageAspectFlags returnFlags{};
            returnFlags |= color ? VK_IMAGE_ASPECT_COLOR_BIT : 0;
            returnFlags |= depth ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
            returnFlags |= stencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
            returnFlags |= metadata ? VK_IMAGE_ASPECT_METADATA_BIT : 0;
            return returnFlags;
        }

        VkImageType vulkanDimension(ResourceDimension dimension)
        {
            switch (dimension)
            {
                case ResourceDimension::Unknown: return VkImageType::VK_IMAGE_TYPE_1D;
                case ResourceDimension::Texture1D: return VkImageType::VK_IMAGE_TYPE_1D;
                case ResourceDimension::Texture2D: return VkImageType::VK_IMAGE_TYPE_2D;
                case ResourceDimension::Texture3D: return VkImageType::VK_IMAGE_TYPE_3D;
                case ResourceDimension::Texture1DArray: return VkImageType::VK_IMAGE_TYPE_1D;
                case ResourceDimension::Texture2DArray: return VkImageType::VK_IMAGE_TYPE_2D;
                case ResourceDimension::TextureCube: return VkImageType::VK_IMAGE_TYPE_2D;
                case ResourceDimension::TextureCubeArray: { ASSERT(false, "Not implemented"); break; }
            }
            return VkImageType::VK_IMAGE_TYPE_1D;
        }

        VkImageAspectFlags vulkanFormatAspects(Format format)
        {
            switch (vulkanFormat(format))
            {
            case VK_FORMAT_UNDEFINED: return 0;

            case VK_FORMAT_S8_UINT: return VK_IMAGE_ASPECT_STENCIL_BIT;

            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT: return VK_IMAGE_ASPECT_DEPTH_BIT;

            default: return VK_IMAGE_ASPECT_COLOR_BIT;
            }
        }

        VkImageSubresourceRange vulkanSubResource(const Texture& texture, SubResource subResources)
        {
            return{
                vulkanFormatAspects(texture.format()), // VK_IMAGE_ASPECT_METADATA_BIT
                static_cast<uint32_t>(subResources.firstMipLevel),
                static_cast<uint32_t>(subResources.mipCount == AllMipLevels ? texture.mipLevels() : subResources.mipCount),
                static_cast<uint32_t>(subResources.firstArraySlice),
                static_cast<uint32_t>(subResources.arraySliceCount == AllArraySlices ? texture.arraySlices() : subResources.arraySliceCount)
            };
        }

        VkImageViewType vulkanViewType(ResourceDimension dimension)
        {
            switch (dimension)
            {
                case ResourceDimension::Texture1D: return VK_IMAGE_VIEW_TYPE_1D;
                case ResourceDimension::Texture2D: return VK_IMAGE_VIEW_TYPE_2D;
                case ResourceDimension::Texture3D: return VK_IMAGE_VIEW_TYPE_3D;
                case ResourceDimension::Texture1DArray: return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
                case ResourceDimension::Texture2DArray: return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
                case ResourceDimension::TextureCube: return VK_IMAGE_VIEW_TYPE_CUBE;
                case ResourceDimension::TextureCubeArray: return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
            }
            return VK_IMAGE_VIEW_TYPE_1D;
        }

        VkImageLayout vulkanResourceStates(ResourceState state)
        {
            switch (state)
            {
            case ResourceState::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
            case ResourceState::Common: return VK_IMAGE_LAYOUT_GENERAL;
            case ResourceState::VertexAndConstantBuffer: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ResourceState::IndexBuffer: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ResourceState::RenderTarget: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ResourceState::UnorderedAccess: return VK_IMAGE_LAYOUT_GENERAL;
            case ResourceState::DepthWrite: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            case ResourceState::DepthRead: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            case ResourceState::NonPixelShaderResource: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ResourceState::PixelShaderResource: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ResourceState::StreamOut: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ResourceState::IndirectArgument: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ResourceState::CopyDest: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case ResourceState::CopySource: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ResourceState::ResolveDest: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ResourceState::ResolveSource: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ResourceState::GenericRead: return VK_IMAGE_LAYOUT_GENERAL;
            case ResourceState::Present: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            case ResourceState::Predication: return VK_IMAGE_LAYOUT_GENERAL;
            default: return VK_IMAGE_LAYOUT_GENERAL;
            }
        }

        VkBlendFactor vulkanBlendFactor(Blend blend)
        {
            switch (blend)
            {
            case Blend::Zero: return VK_BLEND_FACTOR_ZERO;
            case Blend::One: return VK_BLEND_FACTOR_ONE;
            case Blend::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
            case Blend::InvSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case Blend::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
            case Blend::InvSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case Blend::DestAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
            case Blend::InvDestAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case Blend::DestColor: return VK_BLEND_FACTOR_DST_COLOR;
            case Blend::InvDestColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case Blend::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
            case Blend::BlendFactor: return VK_BLEND_FACTOR_CONSTANT_COLOR;
            case Blend::InvBlendFactor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            case Blend::Src1Color: return VK_BLEND_FACTOR_SRC1_COLOR;
            case Blend::InvSrc1Color: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
            case Blend::Src1Alpha: return VK_BLEND_FACTOR_SRC1_ALPHA;
            case Blend::InvSrc1Alpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            default: return VK_BLEND_FACTOR_ZERO;
            }
        }

        VkBlendOp vulkanBlendOp(BlendOperation blendOp)
        {
            switch (blendOp)
            {
                case BlendOperation::Add: return VK_BLEND_OP_ADD;
                case BlendOperation::Subtract: return VK_BLEND_OP_SUBTRACT;
                case BlendOperation::RevSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
                case BlendOperation::Min: return VK_BLEND_OP_MIN;
                case BlendOperation::Max: return VK_BLEND_OP_MAX;
                default: return VK_BLEND_OP_ADD;
            }
        }

        VkCompareOp vulkanComparisonFunction(ComparisonFunction comp)
        {
            switch (comp)
            {
                case ComparisonFunction::Never: return VK_COMPARE_OP_NEVER;
                case ComparisonFunction::Less: return VK_COMPARE_OP_LESS;
                case ComparisonFunction::Equal: return VK_COMPARE_OP_EQUAL;
                case ComparisonFunction::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
                case ComparisonFunction::Greater: return VK_COMPARE_OP_GREATER;
                case ComparisonFunction::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
                case ComparisonFunction::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
                case ComparisonFunction::Always: return VK_COMPARE_OP_ALWAYS;
                default: return VK_COMPARE_OP_NEVER;
            }
        }

        VkStencilOp vulkanStencilOp(StencilOp op)
        {
            switch (op)
            {
                case StencilOp::Keep: return VK_STENCIL_OP_KEEP;
                case StencilOp::Zero: return VK_STENCIL_OP_ZERO;
                case StencilOp::Replace: return VK_STENCIL_OP_REPLACE;
                case StencilOp::IncrSat: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
                case StencilOp::DecrSat: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
                case StencilOp::Invert: return VK_STENCIL_OP_INVERT;
                case StencilOp::Incr: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
                case StencilOp::Decr: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
                default: return VK_STENCIL_OP_KEEP;
            }
        }

        VkStencilOpState vulkanStencilOpState(const DepthStencilOpDescription& op)
        {
            VkStencilOpState res;
            res.failOp = vulkanStencilOp(op.StencilFailOp);
            res.passOp = vulkanStencilOp(op.StencilPassOp);
            res.depthFailOp = vulkanStencilOp(op.StencilDepthFailOp);
            res.compareOp = vulkanComparisonFunction(op.StencilFunc);
            res.compareMask = 0xff;
            res.writeMask = 0xff;
            res.reference = 0;
            return res;
        }

        VkFilter vulkanFilter(Filter filter)
        {
            switch (filter)
            {
            case Filter::Point: return VK_FILTER_NEAREST;
            case Filter::Bilinear: return VK_FILTER_LINEAR;
            case Filter::Trilinear: return VK_FILTER_CUBIC_EXT;
            default: return VK_FILTER_LINEAR;
            }
        }

        VkSamplerAddressMode vulkanAddressMode(TextureAddressMode mode)
        {
            switch (mode)
            {
            case TextureAddressMode::Wrap: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case TextureAddressMode::Mirror: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case TextureAddressMode::Clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case TextureAddressMode::Border: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            case TextureAddressMode::MirrorOnce: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
            default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            }
        }

        VkSamplerCreateInfo vulkanSamplerState(const SamplerDescription& sdesc)
        {
            const auto& desc = sdesc.desc;
            VkSamplerCreateInfo res = {};
            res.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            res.flags = 0;
            res.magFilter = vulkanFilter(desc.filter);
            res.minFilter = vulkanFilter(desc.filter);
            res.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            res.addressModeU = vulkanAddressMode(desc.addressU);
            res.addressModeV = vulkanAddressMode(desc.addressV);
            res.addressModeW = vulkanAddressMode(desc.addressW);
            res.mipLodBias = desc.mipLODBias;
            res.anisotropyEnable = (desc.filter == Filter::Anisotropic) ? VK_TRUE : VK_FALSE;
            res.maxAnisotropy = static_cast<float>(desc.maxAnisotrophy);
            res.compareEnable = desc.comparisonFunc != ComparisonFunction::Never;
            res.compareOp = vulkanComparisonFunction(desc.comparisonFunc);
            res.minLod = desc.minLOD;
            res.maxLod = desc.maxLOD;
            res.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;// desc.borderColor;
            res.unnormalizedCoordinates = VK_FALSE;
            return res;
        }

		VkSampleCountFlagBits vulkanSamples(size_t samples)
		{
			switch (samples)
			{
			case 1: return VK_SAMPLE_COUNT_1_BIT;
			case 2: return VK_SAMPLE_COUNT_2_BIT;
			case 4: return VK_SAMPLE_COUNT_4_BIT;
			case 8: return VK_SAMPLE_COUNT_8_BIT;
			case 16: return VK_SAMPLE_COUNT_16_BIT;
			case 32: return VK_SAMPLE_COUNT_32_BIT;
			case 64: return VK_SAMPLE_COUNT_64_BIT;
			}
			return VK_SAMPLE_COUNT_1_BIT;
		}

        // ---------------------------------------------------------------

        ShaderVisibility fromVulkanShaderVisibility(VkShaderStageFlags flags)
        {
            if (flags == 0x7FFFFFFF)
                return static_cast<ShaderVisibility>(ShaderVisibilityBits::All);
            if (flags == 0x0000001F)
                return static_cast<ShaderVisibility>(ShaderVisibilityBits::AllGraphics);

            bool vertex = (flags & VK_SHADER_STAGE_VERTEX_BIT) > 0;
            bool tesselationControl = (flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) > 0;
            bool tesselationEval = (flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) > 0;
            bool geometry = (flags & VK_SHADER_STAGE_GEOMETRY_BIT) > 0;
            bool fragment = (flags & VK_SHADER_STAGE_FRAGMENT_BIT) > 0;
            bool compute = (flags & VK_SHADER_STAGE_COMPUTE_BIT) > 0;
            ShaderVisibility returnFlags{};
            returnFlags |= vertex ? static_cast<uint32_t>(ShaderVisibilityBits::Vertex) : 0;
            returnFlags |= tesselationControl ? static_cast<uint32_t>(ShaderVisibilityBits::Hull) : 0;
            returnFlags |= tesselationEval ? static_cast<uint32_t>(ShaderVisibilityBits::Domain) : 0;
            returnFlags |= geometry ? static_cast<uint32_t>(ShaderVisibilityBits::Geometry) : 0;
            returnFlags |= fragment ? static_cast<uint32_t>(ShaderVisibilityBits::Pixel) : 0;
            returnFlags |= compute ? static_cast<uint32_t>(ShaderVisibilityBits::Compute) : 0;
            return returnFlags;
        }

        CommandListType fromVulkanCommandListType(VkQueueFlags type)
        {
            switch (type)
            {
                case VK_QUEUE_GRAPHICS_BIT: return CommandListType::Direct;
                case VK_QUEUE_COMPUTE_BIT: return CommandListType::Compute;
                case VK_QUEUE_TRANSFER_BIT: return CommandListType::Copy;
            }
            return CommandListType::Direct;
        }

        Format fromVulkanFormat(VkFormat format)
        {
            switch(format)
            {
                case VK_FORMAT_UNDEFINED: return Format::UNKNOWN;
                case VK_FORMAT_R32G32B32A32_SFLOAT: return Format::R32G32B32A32_FLOAT;
                case VK_FORMAT_R32G32B32A32_UINT: return Format::R32G32B32A32_UINT;
                case VK_FORMAT_R32G32B32A32_SINT: return Format::R32G32B32A32_SINT;
                case VK_FORMAT_R32G32B32_SFLOAT: return Format::R32G32B32_FLOAT;
                case VK_FORMAT_R32G32B32_UINT: return Format::R32G32B32_UINT;
                case VK_FORMAT_R32G32B32_SINT: return Format::R32G32B32_SINT;
                case VK_FORMAT_R16G16B16A16_SFLOAT: return Format::R16G16B16A16_FLOAT;
                case VK_FORMAT_R16G16B16A16_UNORM: return Format::R16G16B16A16_UNORM;
                case VK_FORMAT_R16G16B16A16_UINT: return Format::R16G16B16A16_UINT;
                case VK_FORMAT_R16G16B16A16_SNORM: return Format::R16G16B16A16_SNORM;
                case VK_FORMAT_R16G16B16A16_SINT: return Format::R16G16B16A16_SINT;
                case VK_FORMAT_R32G32_SFLOAT: return Format::R32G32_FLOAT;
                case VK_FORMAT_R32G32_UINT: return Format::R32G32_UINT;
                case VK_FORMAT_R32G32_SINT: return Format::R32G32_SINT;
                case VK_FORMAT_D32_SFLOAT_S8_UINT: return Format::D32_FLOAT_S8X24_UINT;
                case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return Format::R10G10B10A2_UNORM;
                case VK_FORMAT_A2R10G10B10_UINT_PACK32: return Format::R10G10B10A2_UINT;
                case VK_FORMAT_B10G11R11_UFLOAT_PACK32: return Format::R11G11B10_FLOAT;
                case VK_FORMAT_R8G8B8A8_UNORM: return Format::R8G8B8A8_UNORM;
                case VK_FORMAT_R8G8B8A8_SRGB: return Format::R8G8B8A8_UNORM_SRGB;
                case VK_FORMAT_R8G8B8A8_UINT: return Format::R8G8B8A8_UINT;
                case VK_FORMAT_R8G8B8A8_SNORM: return Format::R8G8B8A8_SNORM;
                case VK_FORMAT_R8G8B8A8_SINT: return Format::R8G8B8A8_SINT;
                case VK_FORMAT_R16G16_SFLOAT: return Format::R16G16_FLOAT;
                case VK_FORMAT_R16G16_UNORM: return Format::R16G16_UNORM;
                case VK_FORMAT_R16G16_UINT: return Format::R16G16_UINT;
                case VK_FORMAT_R16G16_SNORM: return Format::R16G16_SNORM;
                case VK_FORMAT_R16G16_SINT: return Format::R16G16_SINT;
                case VK_FORMAT_D32_SFLOAT: return Format::D32_FLOAT;
                case VK_FORMAT_R32_SFLOAT: return Format::R32_FLOAT;
                case VK_FORMAT_R32_UINT: return Format::R32_UINT;
                case VK_FORMAT_R32_SINT: return Format::R32_SINT;
                case VK_FORMAT_D24_UNORM_S8_UINT: return Format::D24_UNORM_S8_UINT;
                case VK_FORMAT_R8G8_UNORM: return Format::R8G8_UNORM;
                case VK_FORMAT_R8G8_UINT: return Format::R8G8_UINT;
                case VK_FORMAT_R8G8_SNORM: return Format::R8G8_SNORM;
                case VK_FORMAT_R8G8_SINT: return Format::R8G8_SINT;
                case VK_FORMAT_R16_SFLOAT: return Format::R16_FLOAT;
                case VK_FORMAT_R16_UNORM: return Format::R16_UNORM;
                case VK_FORMAT_R16_UINT: return Format::R16_UINT;
                case VK_FORMAT_R16_SNORM: return Format::R16_SNORM;
                case VK_FORMAT_R16_SINT: return Format::R16_SINT;
                case VK_FORMAT_R8_UNORM: return Format::R8_UNORM;
                case VK_FORMAT_R8_UINT: return Format::R8_UINT;
                case VK_FORMAT_R8_SNORM: return Format::R8_SNORM;
                case VK_FORMAT_R8_SINT: return Format::R8_SINT;
                case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: return Format::R9G9B9E5_SHAREDEXP;
                case VK_FORMAT_BC1_RGBA_UNORM_BLOCK: return Format::BC1_UNORM;
                case VK_FORMAT_BC1_RGBA_SRGB_BLOCK: return Format::BC1_UNORM_SRGB;
                case VK_FORMAT_BC2_UNORM_BLOCK: return Format::BC2_UNORM;
                case VK_FORMAT_BC2_SRGB_BLOCK: return Format::BC2_UNORM_SRGB;
                case VK_FORMAT_BC3_UNORM_BLOCK: return Format::BC3_UNORM;
                case VK_FORMAT_BC3_SRGB_BLOCK: return Format::BC3_UNORM_SRGB;
                case VK_FORMAT_BC4_UNORM_BLOCK: return Format::BC4_UNORM;
                case VK_FORMAT_BC4_SNORM_BLOCK: return Format::BC4_SNORM;
                case VK_FORMAT_BC5_UNORM_BLOCK: return Format::BC5_UNORM;
                case VK_FORMAT_BC5_SNORM_BLOCK: return Format::BC5_SNORM;
                case VK_FORMAT_R5G6B5_UNORM_PACK16: return Format::B5G6R5_UNORM;
                case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return Format::B5G5R5A1_UNORM;
                case VK_FORMAT_B8G8R8A8_UNORM: return Format::B8G8R8A8_UNORM;
                case VK_FORMAT_B8G8R8A8_SRGB: return Format::B8G8R8A8_UNORM_SRGB;
                case VK_FORMAT_BC6H_UFLOAT_BLOCK: return Format::BC6H_UF16;
                case VK_FORMAT_BC6H_SFLOAT_BLOCK: return Format::BC6H_SF16;
                case VK_FORMAT_BC7_UNORM_BLOCK: return Format::BC7_UNORM;
                case VK_FORMAT_BC7_SRGB_BLOCK: return Format::BC7_UNORM_SRGB;
                case VK_FORMAT_B4G4R4A4_UNORM_PACK16: return Format::B4G4R4A4_UNORM;

                // these are the formats that we do not currently handle
                case VK_FORMAT_R4G4_UNORM_PACK8: return Format::UNKNOWN;
                case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return Format::UNKNOWN;
                case VK_FORMAT_B5G6R5_UNORM_PACK16: return Format::UNKNOWN;
                case VK_FORMAT_B5G5R5A1_UNORM_PACK16: return Format::UNKNOWN;
                case VK_FORMAT_A1R5G5B5_UNORM_PACK16: return Format::UNKNOWN;
                case VK_FORMAT_R8_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_R8_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_R8_SRGB: return Format::UNKNOWN;
                case VK_FORMAT_R8G8_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_R8G8_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_R8G8_SRGB: return Format::UNKNOWN;
                case VK_FORMAT_R8G8B8_UNORM: return Format::UNKNOWN;
                case VK_FORMAT_R8G8B8_SNORM: return Format::UNKNOWN;
                case VK_FORMAT_R8G8B8_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_R8G8B8_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_R8G8B8_UINT: return Format::UNKNOWN;
                case VK_FORMAT_R8G8B8_SINT: return Format::UNKNOWN;
                case VK_FORMAT_R8G8B8_SRGB: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8_UNORM: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8_SNORM: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8_UINT: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8_SINT: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8_SRGB: return Format::UNKNOWN;
                case VK_FORMAT_R8G8B8A8_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_R8G8B8A8_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8A8_SNORM: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8A8_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8A8_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8A8_UINT: return Format::UNKNOWN;
                case VK_FORMAT_B8G8R8A8_SINT: return Format::UNKNOWN;
                case VK_FORMAT_A8B8G8R8_UNORM_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A8B8G8R8_SNORM_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A8B8G8R8_USCALED_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A8B8G8R8_UINT_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A8B8G8R8_SINT_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A8B8G8R8_SRGB_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2R10G10B10_SNORM_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2R10G10B10_USCALED_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2R10G10B10_SINT_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2B10G10R10_SNORM_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2B10G10R10_USCALED_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2B10G10R10_UINT_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_A2B10G10R10_SINT_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_R16_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_R16_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_R16G16_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_R16G16_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_R16G16B16_UNORM: return Format::UNKNOWN;
                case VK_FORMAT_R16G16B16_SNORM: return Format::UNKNOWN;
                case VK_FORMAT_R16G16B16_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_R16G16B16_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_R16G16B16_UINT: return Format::UNKNOWN;
                case VK_FORMAT_R16G16B16_SINT: return Format::UNKNOWN;
                case VK_FORMAT_R16G16B16_SFLOAT: return Format::UNKNOWN;
                case VK_FORMAT_R16G16B16A16_USCALED: return Format::UNKNOWN;
                case VK_FORMAT_R16G16B16A16_SSCALED: return Format::UNKNOWN;
                case VK_FORMAT_R64_UINT: return Format::UNKNOWN;
                case VK_FORMAT_R64_SINT: return Format::UNKNOWN;
                case VK_FORMAT_R64_SFLOAT: return Format::UNKNOWN;
                case VK_FORMAT_R64G64_UINT: return Format::UNKNOWN;
                case VK_FORMAT_R64G64_SINT: return Format::UNKNOWN;
                case VK_FORMAT_R64G64_SFLOAT: return Format::UNKNOWN;
                case VK_FORMAT_R64G64B64_UINT: return Format::UNKNOWN;
                case VK_FORMAT_R64G64B64_SINT: return Format::UNKNOWN;
                case VK_FORMAT_R64G64B64_SFLOAT: return Format::UNKNOWN;
                case VK_FORMAT_R64G64B64A64_UINT: return Format::UNKNOWN;
                case VK_FORMAT_R64G64B64A64_SINT: return Format::UNKNOWN;
                case VK_FORMAT_R64G64B64A64_SFLOAT: return Format::UNKNOWN;
                case VK_FORMAT_D16_UNORM: return Format::UNKNOWN;
                case VK_FORMAT_X8_D24_UNORM_PACK32: return Format::UNKNOWN;
                case VK_FORMAT_S8_UINT: return Format::UNKNOWN;
                case VK_FORMAT_D16_UNORM_S8_UINT: return Format::UNKNOWN;
                case VK_FORMAT_BC1_RGB_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_BC1_RGB_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_EAC_R11_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_EAC_R11_SNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_EAC_R11G11_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_EAC_R11G11_SNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_4x4_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_4x4_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_5x4_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_5x4_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_5x5_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_5x5_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_6x5_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_6x5_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_6x6_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_6x6_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_8x5_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_8x5_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_8x6_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_8x6_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_8x8_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_8x8_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_10x5_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_10x5_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_10x6_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_10x6_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_10x8_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_10x8_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_10x10_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_10x10_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_12x10_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_12x10_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_12x12_UNORM_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_ASTC_12x12_SRGB_BLOCK: return Format::UNKNOWN;
                case VK_FORMAT_MAX_ENUM: return Format::UNKNOWN;
                default: return Format::UNKNOWN;
            }
        }

        PrimitiveTopologyType fromVulkanPrimitiveTopologyType(VkPrimitiveTopology type)
        {
            switch (type)
            {
                case VK_PRIMITIVE_TOPOLOGY_POINT_LIST: return PrimitiveTopologyType::PointList;
                case VK_PRIMITIVE_TOPOLOGY_LINE_LIST: return PrimitiveTopologyType::LineList;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: return PrimitiveTopologyType::TriangleList;
                case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST: return PrimitiveTopologyType::PatchList1;

                // we don't know what to do with these
                case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: return PrimitiveTopologyType::LineStrip;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: return PrimitiveTopologyType::TriangleStrip;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN: return PrimitiveTopologyType::TriangleList;
                case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY: return PrimitiveTopologyType::LineList;
                case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY: return PrimitiveTopologyType::LineList;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY: return PrimitiveTopologyType::TriangleList;
                case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY: return PrimitiveTopologyType::TriangleList;
                case VK_PRIMITIVE_TOPOLOGY_MAX_ENUM: return PrimitiveTopologyType::PointList;
            }
            return PrimitiveTopologyType::PointList;
        }

        InputClassification fromVulkanInputClassification(VkVertexInputRate cls)
        {
            switch (cls)
            {
                case VK_VERTEX_INPUT_RATE_VERTEX: return InputClassification::PerVertexData;
                case VK_VERTEX_INPUT_RATE_INSTANCE: return InputClassification::PerInstanceData;
                case VK_VERTEX_INPUT_RATE_MAX_ENUM: return InputClassification::PerVertexData;
            }
            return InputClassification::PerVertexData;
        }

        FillMode fromVulkanFillMode(VkPolygonMode mode)
        {
            switch (mode)
            {
                case VK_POLYGON_MODE_LINE: return FillMode::Wireframe;
                case VK_POLYGON_MODE_FILL: return FillMode::Solid;
                case VK_POLYGON_MODE_POINT: return FillMode::Point;

                // we don't know what to do with these
                case VK_POLYGON_MODE_MAX_ENUM: return FillMode::Point;
            }
            return FillMode::Solid;
        }

        CullMode fromVulkanCullMode(VkCullModeFlags mode)
        {
            switch (mode)
            {
                case VK_CULL_MODE_NONE: return CullMode::None;
                case VK_CULL_MODE_FRONT_BIT: return CullMode::Front;
                case VK_CULL_MODE_BACK_BIT: return CullMode::Back;
            }
            return CullMode::None;
        }

        BufferUsageFlags fromVulkanBufferUsageFlags(VkBufferUsageFlags flags)
        {
            bool src = (flags & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) > 0;
            bool dst = (flags & VK_BUFFER_USAGE_TRANSFER_DST_BIT) > 0;
            bool uniformTexel = (flags & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) > 0;
            bool storageTexel = (flags & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) > 0;
            bool uniform = (flags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) > 0;
            bool storage = (flags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) > 0;
            bool index = (flags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) > 0;
            bool vertex = (flags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) > 0;
            bool indirect = (flags & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) > 0;
            BufferUsageFlags returnFlags{};
            returnFlags |= src ? static_cast<uint32_t>(BufferUsageFlagBits::Src) : 0;
            returnFlags |= dst ? static_cast<uint32_t>(BufferUsageFlagBits::Dst) : 0;
            returnFlags |= uniformTexel ? static_cast<uint32_t>(BufferUsageFlagBits::UniformTexel) : 0;
            returnFlags |= storageTexel ? static_cast<uint32_t>(BufferUsageFlagBits::StorageTexel) : 0;
            returnFlags |= uniform ? static_cast<uint32_t>(BufferUsageFlagBits::Uniform) : 0;
            returnFlags |= storage ? static_cast<uint32_t>(BufferUsageFlagBits::Storage) : 0;
            returnFlags |= index ? static_cast<uint32_t>(BufferUsageFlagBits::Index) : 0;
            returnFlags |= vertex ? static_cast<uint32_t>(BufferUsageFlagBits::Vertex) : 0;
            returnFlags |= indirect ? static_cast<uint32_t>(BufferUsageFlagBits::Indirect) : 0;
            return returnFlags;
        }

        BufferMemoryFlags fromVulkanMemoryFlags(VkMemoryPropertyFlags flags)
        {
            bool deviceLocal = (flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) > 0;
            bool hostVisible = (flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) > 0;
            bool hostCoherent = (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) > 0;
            bool hostCached = (flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) > 0;
            bool lazy = (flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) > 0;
            BufferMemoryFlags returnFlags{};
            returnFlags |= deviceLocal ? static_cast<uint32_t>(BufferMemoryFlagBits::DeviceLocal) : 0;
            returnFlags |= hostVisible ? static_cast<uint32_t>(BufferMemoryFlagBits::HostVisible) : 0;
            returnFlags |= hostCoherent ? static_cast<uint32_t>(BufferMemoryFlagBits::HostCoherent) : 0;
            returnFlags |= hostCached ? static_cast<uint32_t>(BufferMemoryFlagBits::HostCached) : 0;
            returnFlags |= lazy ? static_cast<uint32_t>(BufferMemoryFlagBits::LazilyAllocated) : 0;
            return returnFlags;
        }

        DescriptorType fromVulkanDescriptorType(VkDescriptorType type)
        {
            switch (type)
            {
                case VK_DESCRIPTOR_TYPE_SAMPLER: return DescriptorType::Sampler;
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return DescriptorType::CombinedImageSampler;
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return DescriptorType::SampledImage;
                case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: return DescriptorType::StorageImage;
                case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: return DescriptorType::UniformTexelBuffer;
                case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: return DescriptorType::StorageTexelBuffer;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return DescriptorType::UniformBuffer;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: return DescriptorType::StorageBuffer;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: return DescriptorType::UniformBufferDynamic;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return DescriptorType::StorageBufferDynamic;
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return DescriptorType::InputAttachment;
            }
            return DescriptorType::UniformBuffer;
        }

        ImageTiling fromVulkanImageTiling(VkImageTiling tiling)
        {
            switch (tiling)
            {
                case VK_IMAGE_TILING_OPTIMAL: return ImageTiling::Optimal;
                case VK_IMAGE_TILING_LINEAR: return ImageTiling::Linear;
                case VK_IMAGE_TILING_MAX_ENUM: return ImageTiling::Optimal;
            }
            return ImageTiling::Optimal;
        }

        /*ImageLayout fromVulkanImageLayout(VkImageLayout layout)
        {
            switch (layout)
            {
                case VK_IMAGE_LAYOUT_UNDEFINED: return ImageLayout::Undefined;
                case VK_IMAGE_LAYOUT_GENERAL: return ImageLayout::General;
                case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return ImageLayout::ColorAttachmentOptimal;
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return ImageLayout::DepthStencilAttachmentOptimal;
                case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return ImageLayout::DepthStencilReadOnlyOptimal;
                case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return ImageLayout::ShaderReadOnlyOptimal;
                case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return ImageLayout::TransferSrcOptimal;
                case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return ImageLayout::TransferDstOptimal;
                case VK_IMAGE_LAYOUT_PREINITIALIZED: return ImageLayout::Preinitialized;
                case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return ImageLayout::PresentSrc;
                case VK_IMAGE_LAYOUT_RANGE_SIZE: return ImageLayout::Undefined;
                case VK_IMAGE_LAYOUT_MAX_ENUM: return ImageLayout::Undefined;
            }
            return ImageLayout::Undefined;
        }*/

        ResourceState fromVulkanResourceStates(VkImageLayout state)
        {
            switch (state)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED: return ResourceState::Undefined;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return ResourceState::VertexAndConstantBuffer;
            /*case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return ResourceState::IndexBuffer;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return ResourceState::PixelShaderResource;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: return ResourceState::IndirectArgument;*/

            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return ResourceState::RenderTarget;
            /*case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return ResourceState::ResolveDest;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: return ResourceState::ResolveSource;*/

            case VK_IMAGE_LAYOUT_GENERAL: return ResourceState::UnorderedAccess;
            /*case VK_IMAGE_LAYOUT_GENERAL: return ResourceState::Common;
            case VK_IMAGE_LAYOUT_GENERAL: return ResourceState::NonPixelShaderResource;
            case VK_IMAGE_LAYOUT_GENERAL: return ResourceState::GenericRead;
            case VK_IMAGE_LAYOUT_GENERAL: return ResourceState::Predication;*/

            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return ResourceState::DepthWrite;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: return ResourceState::DepthRead;

            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return ResourceState::StreamOut;
            //case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: return ResourceState::CopySource;

            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: return ResourceState::CopyDest;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: return ResourceState::Present;
            default: return ResourceState::Common;
            }
        }

        ImageAspectFlags fromVulkanImageAspectFlags(VkImageAspectFlags flags)
        {
            bool color = (flags & VK_IMAGE_ASPECT_COLOR_BIT) > 0;
            bool depth = (flags & VK_IMAGE_ASPECT_DEPTH_BIT) > 0;
            bool stencil = (flags & VK_IMAGE_ASPECT_STENCIL_BIT) > 0;
            bool metadata = (flags & VK_IMAGE_ASPECT_METADATA_BIT) > 0;
            ImageAspectFlags returnFlags{};
            returnFlags |= color ? static_cast<uint32_t>(ImageAspectFlagBits::Color) : 0;
            returnFlags |= depth ? static_cast<uint32_t>(ImageAspectFlagBits::Depth) : 0;
            returnFlags |= stencil ? static_cast<uint32_t>(ImageAspectFlagBits::Stencil) : 0;
            returnFlags |= metadata ? static_cast<uint32_t>(ImageAspectFlagBits::Metadata) : 0;
            return returnFlags;
        }

        ResourceDimension fromVulkanDimension(VkImageType dimension)
        {
            switch (dimension)
            {
                case VkImageType::VK_IMAGE_TYPE_1D: return ResourceDimension::Texture1D;
                case VkImageType::VK_IMAGE_TYPE_2D: return ResourceDimension::Texture2D;
                case VkImageType::VK_IMAGE_TYPE_3D: return ResourceDimension::Texture3D;
            }
            return ResourceDimension::Texture1D;
        }

        SubResource fromVulkanSubResource(VkImageSubresourceRange subResources)
        {
            SubResource res;
            res.firstMipLevel = subResources.baseMipLevel;
            res.mipCount = static_cast<int32_t>(subResources.levelCount);
            res.firstArraySlice = subResources.baseArrayLayer;
            res.arraySliceCount = static_cast<int32_t>(subResources.layerCount);
            return res;
        }

        ResourceDimension fromVulkanViewType(VkImageViewType dimension)
        {
            switch (dimension)
            {
                case VK_IMAGE_VIEW_TYPE_1D: return ResourceDimension::Texture1D;
                case VK_IMAGE_VIEW_TYPE_2D: return ResourceDimension::Texture2D;
                case VK_IMAGE_VIEW_TYPE_3D: return ResourceDimension::Texture3D;
                case VK_IMAGE_VIEW_TYPE_1D_ARRAY: return ResourceDimension::Texture1DArray;
                case VK_IMAGE_VIEW_TYPE_2D_ARRAY: return ResourceDimension::Texture2DArray;
                case VK_IMAGE_VIEW_TYPE_CUBE: return ResourceDimension::TextureCube;
                case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY: return ResourceDimension::TextureCubeArray;
            }
            return ResourceDimension::Texture1D;
        }

        Blend fromVulkanBlendFactor(VkBlendFactor blend)
        {
            switch (blend)
            {
            case VK_BLEND_FACTOR_ZERO: return Blend::Zero;
            case VK_BLEND_FACTOR_ONE: return Blend::One;
            case VK_BLEND_FACTOR_SRC_COLOR: return Blend::SrcColor;
            case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR: return Blend::InvSrcColor;
            case VK_BLEND_FACTOR_SRC_ALPHA: return Blend::SrcAlpha;
            case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA: return Blend::InvSrcAlpha;
            case VK_BLEND_FACTOR_DST_ALPHA: return Blend::DestAlpha;
            case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA: return Blend::InvDestAlpha;
            case VK_BLEND_FACTOR_DST_COLOR: return Blend::DestColor;
            case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR: return Blend::InvDestColor;
            case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE: return Blend::SrcAlphaSaturate;
            case VK_BLEND_FACTOR_CONSTANT_COLOR: return Blend::BlendFactor;
            case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR: return Blend::InvBlendFactor;
            case VK_BLEND_FACTOR_SRC1_COLOR: return Blend::Src1Color;
            case VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR: return Blend::InvSrc1Color;
            case VK_BLEND_FACTOR_SRC1_ALPHA: return Blend::Src1Alpha;
            case VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA: return Blend::InvSrc1Alpha;
            default: return Blend::Zero;
            }
        }

        BlendOperation fromVulkanBlendOp(VkBlendOp blendOp)
        {
            switch (blendOp)
            {
            case VK_BLEND_OP_ADD: return BlendOperation::Add;
            case VK_BLEND_OP_SUBTRACT: return BlendOperation::Subtract;
            case VK_BLEND_OP_REVERSE_SUBTRACT: return BlendOperation::RevSubtract;
            case VK_BLEND_OP_MIN: return BlendOperation::Min;
            case VK_BLEND_OP_MAX: return BlendOperation::Max;
            default: return BlendOperation::Add;
            }
        }

        ComparisonFunction fromVulkanComparisonFunction(VkCompareOp comp)
        {
            switch (comp)
            {
                case VK_COMPARE_OP_NEVER: return ComparisonFunction::Never;
                case VK_COMPARE_OP_LESS: return ComparisonFunction::Less;
                case VK_COMPARE_OP_EQUAL: return ComparisonFunction::Equal;
                case VK_COMPARE_OP_LESS_OR_EQUAL: return ComparisonFunction::LessEqual;
                case VK_COMPARE_OP_GREATER: return ComparisonFunction::Greater;
                case VK_COMPARE_OP_NOT_EQUAL: return ComparisonFunction::NotEqual;
                case VK_COMPARE_OP_GREATER_OR_EQUAL: return ComparisonFunction::GreaterEqual;
                case VK_COMPARE_OP_ALWAYS: return ComparisonFunction::Always;
                default: return ComparisonFunction::Never;
            }
        }

        StencilOp fromVulkanStencilOp(VkStencilOp op)
        {
            switch (op)
            {
            case VK_STENCIL_OP_KEEP: return StencilOp::Keep;
            case VK_STENCIL_OP_ZERO: return StencilOp::Zero;
            case VK_STENCIL_OP_REPLACE: return StencilOp::Replace;
            case VK_STENCIL_OP_INCREMENT_AND_CLAMP: return StencilOp::IncrSat;
            case VK_STENCIL_OP_DECREMENT_AND_CLAMP: return StencilOp::DecrSat;
            case VK_STENCIL_OP_INVERT: return StencilOp::Invert;
            case VK_STENCIL_OP_INCREMENT_AND_WRAP: return StencilOp::Incr;
            case VK_STENCIL_OP_DECREMENT_AND_WRAP: return StencilOp::Decr;
            default: return StencilOp::Keep;
            }
        }

        DepthStencilOpDescription fromVulkanStencilOpState(const VkStencilOpState& op)
        {
            DepthStencilOpDescription res;
            res.StencilFailOp = fromVulkanStencilOp(op.failOp);
            res.StencilPassOp = fromVulkanStencilOp(op.passOp);
            res.StencilDepthFailOp = fromVulkanStencilOp(op.depthFailOp);
            res.StencilFunc = fromVulkanComparisonFunction(op.compareOp);
            return res;
        }
    }
}
