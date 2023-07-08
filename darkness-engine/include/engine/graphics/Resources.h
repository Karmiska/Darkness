#pragma once

#include "engine/graphics/ResourcesImplIf.h"

#include "engine/graphics/Format.h"
#include "containers/vector.h"
#include "tools/ByteRange.h"
#include "engine/graphics/CommonNoDep.h"
#include "shaders/ShaderPodTypes.h"
#include <functional>
#include <type_traits>

// for memcpy
#include <cstring>

namespace engine
{
    using ResourceKey = std::size_t;

    class Device;

	class BufferOwner;
	class BufferSRVOwner;
	class BufferUAVOwner;
	class BufferIBVOwner;
	class BufferCBVOwner;
	class BufferVBVOwner;
    class BindlessBufferSRVOwner;
    class BindlessBufferUAVOwner;

	class RaytracingAccelerationStructureOwner;

	class TextureOwner;
	class TextureSRVOwner;
	class TextureUAVOwner;
	class TextureDSVOwner;
	class TextureRTVOwner;
    class BindlessTextureSRVOwner;
    class BindlessTextureUAVOwner;

    namespace implementation
    {
        class SwapChainImplDX12;
        class SwapChainImplVulkan;
        class DeviceImplDX12;
        class DeviceImplVulkan;
        class BarrierImplDX12;
        class BarrierImplVulkan;
        class CommandListImplDX12;
        class CommandListImplVulkan;
        class PipelineImplDX12;
        class PipelineImplVulkan;
        class DescriptorHeapImplDX12;
        class DescriptorHeapImplVulkan;

        class BufferSRVImplDX12;
        class BufferSRVImplVulkan;
        class BufferUAVImplDX12;
        class BufferUAVImplVulkan;
        class BufferIBVImplDX12;
        class BufferIBVImplVulkan;
        class BufferVBVImplDX12;
        class BufferVBVImplVulkan;
        class BufferCBVImplDX12;
        class BufferCBVImplVulkan;
        class BindlessBufferSRVImplDX12;
        class BindlessBufferSRVImplVulkan;
        class BindlessBufferUAVImplDX12;
        class BindlessBufferUAVImplVulkan;
        class RaytracingAccelerationStructureImplDX12;
        class RaytracingAccelerationStructureImplVulkan;

        class TextureSRVImplDX12;
        class TextureSRVImplVulkan;
        class TextureUAVImplDX12;
        class TextureUAVImplVulkan;
        class TextureDSVImplDX12;
        class TextureDSVImplVulkan;
        class TextureRTVImplDX12;
        class TextureRTVImplVulkan;
        class BindlessTextureSRVImplDX12;
        class BindlessTextureSRVImplVulkan;
        class BindlessTextureUAVImplDX12;
        class BindlessTextureUAVImplVulkan;

#define ImplementationFriendAccess \
        friend class implementation::SwapChainImplDX12; \
        friend class implementation::SwapChainImplVulkan; \
        friend class implementation::DeviceImplDX12; \
        friend class implementation::DeviceImplVulkan; \
        friend class implementation::BarrierImplDX12; \
        friend class implementation::BarrierImplVulkan; \
        friend class implementation::CommandListImplDX12; \
        friend class implementation::CommandListImplVulkan; \
        friend class implementation::PipelineImplDX12; \
        friend class implementation::PipelineImplVulkan; \
        friend class implementation::DescriptorHeapImplDX12; \
        friend class implementation::DescriptorHeapImplVulkan

#define BufferViewImplementationFriendAccess \
        friend class implementation::BufferSRVImplDX12; \
        friend class implementation::BufferSRVImplVulkan; \
        friend class implementation::BufferUAVImplDX12; \
        friend class implementation::BufferUAVImplVulkan; \
        friend class implementation::BufferIBVImplDX12; \
        friend class implementation::BufferIBVImplVulkan; \
        friend class implementation::BufferVBVImplDX12; \
        friend class implementation::BufferVBVImplVulkan; \
        friend class implementation::BufferCBVImplDX12; \
        friend class implementation::BufferCBVImplVulkan; \
        friend class implementation::RaytracingAccelerationStructureImplDX12; \
        friend class implementation::RaytracingAccelerationStructureImplVulkan

#define TextureViewImplementationFriendAccess \
        friend class implementation::TextureSRVImplDX12; \
        friend class implementation::TextureSRVImplVulkan; \
        friend class implementation::TextureUAVImplDX12; \
        friend class implementation::TextureUAVImplVulkan; \
        friend class implementation::TextureDSVImplDX12; \
        friend class implementation::TextureDSVImplVulkan; \
        friend class implementation::TextureRTVImplDX12; \
        friend class implementation::TextureRTVImplVulkan

    }

    class Buffer
    {
    public:
		Buffer() : m_impl{ nullptr } {};
        Buffer(implementation::BufferImplIf* impl);

        void* map(const Device& device);
        void unmap(const Device& device);

        const BufferDescription::Descriptor& description() const;
        ResourceState state() const;
        void state(ResourceState state) const;

		bool operator==(const Buffer& buffer) const;
		bool operator!=(const Buffer& buffer) const;
		operator bool() const;
    protected:
        ImplementationFriendAccess;
        BufferViewImplementationFriendAccess;
		
		friend class BufferOwner;

        implementation::BufferImplIf* m_impl;
    };

	class RaytracingAccelerationStructure;

    class BufferSRV
    {
    public:
        BufferSRV() : m_impl{ nullptr }, m_uniqueId{ 0u } {};
        
		bool valid() const;
        const BufferDescription::Descriptor& desc() const;

        Buffer buffer() const;

        uint64_t resourceId() const;
		operator bool() const;

		bool operator==(const BufferSRV& buffer) const;
		bool operator!=(const BufferSRV& buffer) const;
    protected:
        ImplementationFriendAccess;
        friend class implementation::BindlessBufferSRVImplDX12;
        friend class implementation::BindlessBufferSRVImplVulkan;
		friend class RaytracingAccelerationStructure;
		friend class BufferSRVOwner;

        BufferSRV(implementation::BufferSRVImplIf* impl);
		implementation::BufferSRVImplIf* m_impl;
        uint64_t m_uniqueId;
    };

    class BufferUAV
    {
    public:
		BufferUAV() : m_impl{ nullptr }, m_uniqueId{ 0u } {};

        bool valid() const;
        const BufferDescription::Descriptor& desc() const;

        Buffer buffer() const;

        uint64_t resourceId() const;
		operator bool() const;

		bool operator==(const BufferUAV& buffer) const;
		bool operator!=(const BufferUAV& buffer) const;
    protected:
        ImplementationFriendAccess;
        friend class implementation::BindlessBufferUAVImplDX12;
        friend class implementation::BindlessBufferUAVImplVulkan;
		friend class BufferUAVOwner;

        BufferUAV(implementation::BufferUAVImplIf* impl);
		implementation::BufferUAVImplIf* m_impl;
        uint64_t m_uniqueId;
    };

    class BufferIBV
    {
    public:
		BufferIBV() : m_impl{ nullptr } {};

        bool valid() const;
        const BufferDescription::Descriptor& desc() const;

        Buffer buffer() const;
		operator bool() const;
		bool operator==(const BufferIBV& buffer) const;
		bool operator!=(const BufferIBV& buffer) const;
    protected:
        ImplementationFriendAccess;
		friend class BufferIBVOwner;

        BufferIBV(implementation::BufferIBVImplIf* impl);
		implementation::BufferIBVImplIf* m_impl;
    };

    class BufferCBV
    {
    public:
		BufferCBV() : m_impl{ nullptr } {};

        bool valid() const;
        const BufferDescription::Descriptor& desc() const;

        Buffer buffer() const;
		operator bool() const;
		bool operator==(const BufferCBV& buffer) const;
		bool operator!=(const BufferCBV& buffer) const;
    protected:
        ImplementationFriendAccess;
		friend class BufferCBVOwner;

        BufferCBV(implementation::BufferCBVImplIf* impl);
		implementation::BufferCBVImplIf* m_impl;
    };

    class BufferVBV
    {
    public:
		BufferVBV() : m_impl{ nullptr } {};

        bool valid() const;
        const BufferDescription::Descriptor& desc() const;

        Buffer buffer() const;
		operator bool() const;
		bool operator==(const BufferVBV& buffer) const;
		bool operator!=(const BufferVBV& buffer) const;
    protected:
        ImplementationFriendAccess;
		friend class BufferVBVOwner;

        BufferVBV(implementation::BufferVBVImplIf* impl);
		implementation::BufferVBVImplIf* m_impl;
    };

    class BindlessBufferSRV
    {
    public:
        BindlessBufferSRV() : m_impl{ nullptr } {};
        BindlessBufferSRV(implementation::BindlessBufferSRVImplIf* impl);

        bool operator==(const BindlessBufferSRV& buffer) const;
        bool operator!=(const BindlessBufferSRV& buffer) const;
        operator bool() const;

        uint32_t push(BufferSRVOwner buffer);
        size_t size() const;
        BufferSRV get(size_t index) const;
        uint64_t resourceId() const;
        bool change() const;
        void change(bool value);
    protected:
        ImplementationFriendAccess;
        friend class BindlessBufferSRVOwner;

        implementation::BindlessBufferSRVImplIf* m_impl;
    };

    class BindlessBufferUAV
    {
    public:
        BindlessBufferUAV() : m_impl{ nullptr } {};
        BindlessBufferUAV(implementation::BindlessBufferUAVImplIf* impl);

        bool operator==(const BindlessBufferUAV& buffer) const;
        bool operator!=(const BindlessBufferUAV& buffer) const;
        operator bool() const;

        uint32_t push(BufferUAVOwner buffer);
        size_t size() const;
        BufferUAV get(size_t index) const;
        uint64_t resourceId() const;
        bool change() const;
        void change(bool value);
    protected:
        ImplementationFriendAccess;
        friend class BindlessBufferUAVOwner;

        implementation::BindlessBufferUAVImplIf* m_impl;
    };

	class RaytracingAccelerationStructure
	{
	public:
		RaytracingAccelerationStructure() : m_impl{ nullptr } {};
		RaytracingAccelerationStructure(implementation::RaytracingAccelerationStructureImplIf* impl);

		uint64_t resourceId() const;

		ResourceState state() const;
		void state(ResourceState state) const;

		bool operator==(const RaytracingAccelerationStructure& buffer) const;
		bool operator!=(const RaytracingAccelerationStructure& buffer) const;
		operator bool() const;
	protected:
        ImplementationFriendAccess;
		friend class RaytracingAccelerationStructureOwner;

		implementation::RaytracingAccelerationStructureImplIf* m_impl;
	};

    class Texture
    {
    public:
        Texture() : m_impl{ nullptr } {};

        void* map(const Device& device);
        void unmap(const Device& device);

        bool valid() const;

        Format format() const;
        size_t width() const;
        size_t height() const;
        size_t depth() const;
        size_t arraySlices() const;
        size_t mipLevels() const;
        size_t samples() const;
        ResourceDimension dimension() const;

        const TextureDescription::Descriptor& description() const;
        ResourceState state(int slice, int mip) const;
        void state(int slice, int mip, ResourceState state) const;

		bool operator==(const Texture& texture) const;
		bool operator!=(const Texture& texture) const;
		operator bool() const;
    protected:
        ImplementationFriendAccess;
        TextureViewImplementationFriendAccess;
		friend class TextureOwner;

        Texture(implementation::TextureImplIf* texture);
		implementation::TextureImplIf* m_impl;
    };

	class TextureSRV
    {
    public:
        TextureSRV() : m_impl{ nullptr }, m_uniqueId{ 0u } {};

        bool valid() const;

        Format format() const;
        size_t width() const;
        size_t height() const;
        size_t depth() const;
		ResourceDimension dimension() const;

        Texture texture() const;

        const SubResource& subResource() const;

        uint64_t resourceId() const;
		operator bool() const;
		bool operator==(const TextureSRV& texture) const;
		bool operator!=(const TextureSRV& texture) const;
    protected:
        ImplementationFriendAccess;
        friend class implementation::BindlessTextureSRVImplDX12;
        friend class implementation::BindlessTextureSRVImplVulkan;
		friend class TextureSRVOwner;

        TextureSRV(implementation::TextureSRVImplIf* view);
        implementation::TextureSRVImplIf* m_impl;
        uint64_t m_uniqueId;
    };

    class TextureUAV
    {
    public:
        TextureUAV() : m_impl{ nullptr }, m_uniqueId{ 0u } {};
        
        bool valid() const;

        Format format() const;
        size_t width() const;
        size_t height() const;
        size_t depth() const;
		ResourceDimension dimension() const;

        Texture texture() const;

        const SubResource& subResource() const;

        uint64_t resourceId() const;
		operator bool() const;
		bool operator==(const TextureUAV& texture) const;
		bool operator!=(const TextureUAV& texture) const;
    protected:
        ImplementationFriendAccess;
        friend class implementation::BindlessTextureUAVImplDX12;
        friend class implementation::BindlessTextureUAVImplVulkan;
		friend class TextureUAVOwner;

        TextureUAV(implementation::TextureUAVImplIf* view);
        implementation::TextureUAVImplIf* m_impl;
        uint64_t m_uniqueId;
    };

    class TextureDSV
    {
    public:
        TextureDSV() : m_impl{ nullptr } {};

		bool valid() const;

        Format format() const;
        size_t width() const;
        size_t height() const;

        Texture texture() const;

        const SubResource& subResource() const;
		operator bool() const;
		bool operator==(const TextureDSV& texture) const;
		bool operator!=(const TextureDSV& texture) const;
    protected:
        ImplementationFriendAccess;
		friend class TextureDSVOwner;

        TextureDSV(implementation::TextureDSVImplIf* view);
        implementation::TextureDSVImplIf* m_impl;
    };

    class TextureRTV
    {
    public:
        TextureRTV() : m_impl{ nullptr } {};

        bool valid() const;

        Format format() const;
        size_t width() const;
        size_t height() const;

        Texture texture() const;

        const SubResource& subResource() const;
		operator bool() const;
		bool operator==(const TextureRTV& texture) const;
		bool operator!=(const TextureRTV& texture) const;
    protected:
        ImplementationFriendAccess;
		friend class TextureRTVOwner;

        TextureRTV(implementation::TextureRTVImplIf* view);
        implementation::TextureRTVImplIf* m_impl;
    };

    class BindlessTextureSRV
    {
    public:
        BindlessTextureSRV() : m_impl{ nullptr } {};

        bool operator==(const BindlessTextureSRV& texture) const;
        bool operator!=(const BindlessTextureSRV& texture) const;
        operator bool() const;

        uint32_t push(TextureSRVOwner texture);
        size_t size() const;
        TextureSRV get(size_t index) const;
        uint64_t resourceId() const;
        bool change() const;
        void change(bool value);
    protected:
        ImplementationFriendAccess;
        friend class BindlessTextureSRVOwner;

        BindlessTextureSRV(implementation::BindlessTextureSRVImplIf* texture);
        implementation::BindlessTextureSRVImplIf* m_impl;
    };

    class BindlessTextureUAV
    {
    public:
        BindlessTextureUAV() : m_impl{ nullptr } {};

        bool operator==(const BindlessTextureUAV& texture) const;
        bool operator!=(const BindlessTextureUAV& texture) const;
        operator bool() const;

        uint32_t push(TextureUAVOwner texture);
        size_t size() const;
        TextureUAV get(size_t index) const;
        uint64_t resourceId() const;
        bool change() const;
        void change(bool value);
    protected:
        ImplementationFriendAccess;
        friend class BindlessTextureUAVOwner;

        BindlessTextureUAV(implementation::BindlessTextureUAVImplIf* texture);
        implementation::BindlessTextureUAVImplIf* m_impl;
    };

	class RootConstant
	{
	public:
		RootConstant() = default;

		RootConstant(const engine::string& name)
			: m_name{ name }
		{}

		const engine::string& name() const
		{
			return m_name;
		}

        tools::ByteRange range;
        engine::shared_ptr<BufferCBVOwner> buffer;
	private:
		engine::string m_name;
	};

	static_assert(std::is_standard_layout<Buffer>::value, "Must be a POD type.");
	static_assert(std::is_standard_layout<BufferSRV>::value, "Must be a POD type.");
	static_assert(std::is_standard_layout<BufferUAV>::value, "Must be a POD type.");
	static_assert(std::is_standard_layout<BufferIBV>::value, "Must be a POD type.");
	static_assert(std::is_standard_layout<BufferCBV>::value, "Must be a POD type.");
	static_assert(std::is_standard_layout<BufferVBV>::value, "Must be a POD type.");
    static_assert(std::is_standard_layout<BindlessBufferSRV>::value, "Must be a POD type.");
    static_assert(std::is_standard_layout<BindlessBufferUAV>::value, "Must be a POD type.");

	static_assert(std::is_standard_layout<RaytracingAccelerationStructure>::value, "Must be a POD type.");

	static_assert(std::is_standard_layout<Texture>::value, "Must be a POD type.");
	static_assert(std::is_standard_layout<TextureSRV>::value, "Must be a POD type.");
	static_assert(std::is_standard_layout<TextureUAV>::value, "Must be a POD type.");
	static_assert(std::is_standard_layout<TextureDSV>::value, "Must be a POD type.");
	static_assert(std::is_standard_layout<TextureRTV>::value, "Must be a POD type.");
    static_assert(std::is_standard_layout<BindlessTextureSRV>::value, "Must be a POD type.");
    static_assert(std::is_standard_layout<BindlessTextureUAV>::value, "Must be a POD type.");

}
