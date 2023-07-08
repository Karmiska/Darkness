#pragma once

#include <cstdint>

namespace engine
{
    enum class GraphicsApi
    {
        DX12,
        Vulkan
    };

    enum class ResourceState
    {
        Undefined,
        Common,
        VertexAndConstantBuffer,
        IndexBuffer,
        RenderTarget,
        UnorderedAccess,
        DepthWrite,
        DepthRead,
        NonPixelShaderResource,
        PixelShaderResource,
        StreamOut,
        IndirectArgument,
        CopyDest,
        CopySource,
        ResolveDest,
        ResolveSource,
        GenericRead,
        Present,
        Predication
    };

    enum class ResourceUsage
    {
        GpuRead,
        GpuReadWrite,
        GpuRenderTargetRead,
        GpuRenderTargetReadWrite,
        DepthStencil,
        GpuToCpu,
        Upload,
        AccelerationStructure
    };

    static const int MaxNumberOfDescriptorsPerHeap = 256;
    constexpr int ShadowMapWidth = 1024;
    constexpr int ShadowMapHeight = 1024;

    size_t mipCount(size_t width, size_t height);
    size_t mipCount(size_t width, size_t height, size_t depth);

	enum class ResourceDimension
	{
		Unknown,
		Texture1D,
		Texture2D,
		Texture3D,
		Texture1DArray,
		Texture2DArray,
		TextureCube,
		TextureCubeArray
	};

    enum class PredicationOp
    {
        EqualZero,
        NotEqualZero
    };

    enum class ShaderVisibilityBits
    {
        Vertex = 0x00000001,
        Hull = 0x00000002,
        Domain = 0x00000004,
        Geometry = 0x00000008,
        Pixel = 0x00000010,
        Compute = 0x00000020,
        Amplification = 0x00000040,
        Mesh = 0x00000080,
        AllGraphics = 0x0000001F,
        All = 0x7FFFFFFF
    };
    using ShaderVisibility = uint32_t;

    enum class ComparisonFunction
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class DescriptorHeapFlags
    {
        None,
        ShaderVisible
    };

    enum class CommandListType
    {
        Direct,
        Bundle,
        Compute,
        Copy
    };

    enum class BufferUsageFlagBits
    {
        Src = 0x00000001,
        Dst = 0x00000002,
        UniformTexel = 0x00000004,
        StorageTexel = 0x00000008,
        Uniform = 0x00000010,
        Storage = 0x00000020,
        Index = 0x00000040,
        Vertex = 0x00000080,
        Indirect = 0x00000100
    };
    using BufferUsageFlags = uint32_t;

    enum class BufferMemoryFlagBits
    {
        DeviceLocal = 0x00000001,
        HostVisible = 0x00000002,
        HostCoherent = 0x00000004,
        HostCached = 0x00000008,
        LazilyAllocated = 0x00000010
    };
    using BufferMemoryFlags = uint32_t;

    enum class ImageUsageFlagBits
    {
        Src = 0x00000001,
        Dst = 0x00000002,
        Sampled = 0x00000004,
        Storage = 0x00000008,
        ColorAttachment = 0x00000010,
        DepthStencilAttachment = 0x00000020,
        TransientAttachment = 0x00000040,
        InputAttachment = 0x00000080
    };
    using ImageUsageFlags = uint32_t;

    enum class ImageTiling
    {
        Optimal,
        Linear
    };

    enum class ImageAspectFlagBits
    {
        Color       = 0x00000001,
        Depth       = 0x00000002,
        Stencil     = 0x00000004,
        Metadata    = 0x00000008
    };
    using ImageAspectFlags = uint32_t;

    constexpr const int32_t AllMipLevels = -1;
    constexpr const int32_t AllArraySlices = -1;
    struct SubResource
    {
        uint32_t firstMipLevel = 0;
        int32_t mipCount = AllMipLevels;
        uint32_t firstArraySlice = 0;
        int32_t arraySliceCount = AllArraySlices;
    };

    struct DispatchIndirectArgs
    {
        uint32_t threadGroupX;
        uint32_t threadGroupY;
        uint32_t threadGroupZ;
    };

    struct DrawIndirectArgs
    {
        uint32_t VertexCountPerInstance;
        uint32_t InstanceCount;
        uint32_t StartVertexLocation;
        uint32_t StartInstanceLocation;
    };

    struct DrawIndexIndirectArgs
    {
        uint32_t IndexCountPerInstance;
        uint32_t InstanceCount;
        uint32_t StartIndexLocation;
        int BaseVertexLocation;
        uint32_t StartInstanceLocation;
    };
}
