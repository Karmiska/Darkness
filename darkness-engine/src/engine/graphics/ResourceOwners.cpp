#include "engine/graphics/ResourceOwners.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12Resources.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanResources.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalBuffer.h"
#endif

namespace engine
{
	BufferOwner::operator Buffer()
	{
		return Buffer{ m_implementation.get() };
	}

	BufferOwner::operator const Buffer() const
	{
		return Buffer{ m_implementation.get() };
	}

	BufferOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	Buffer BufferOwner::resource() const
	{
		return Buffer{ m_implementation.get() };
	}

	BufferOwner::BufferOwner(
		engine::shared_ptr<implementation::BufferImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::BufferImplIf>)> returnImplementation)
		: m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::BufferImplIf>>(returnImplementation, impl) }
	{}

	BufferOwner::BufferOwner(
		engine::shared_ptr<implementation::BufferImplIf> implementation,
		engine::shared_ptr<Returner<implementation::BufferImplIf>> returner)
		: m_implementation{ implementation }
		, m_returner{ returner }
	{}

	// ---------------------------------------------

	BufferSRVOwner::BufferSRVOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
	{}

	BufferSRVOwner::operator BufferSRV()
	{
		return BufferSRV{ m_implementation.get() };
	}

	BufferSRVOwner::operator const BufferSRV() const
	{
		return BufferSRV{ m_implementation.get() };
	}

	BufferSRV BufferSRVOwner::resource() const
	{
		return BufferSRV{ m_implementation.get() };
	}

	BufferSRVOwner::operator BufferOwner()
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferSRVOwner::operator BufferOwner() const
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferSRVOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	BufferSRVOwner::BufferSRVOwner(
		engine::shared_ptr<implementation::BufferSRVImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::BufferSRVImplIf>)> returnImplementation,
		engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
		engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation)
		: m_bufferImplementation{ bufferimpl }
		, m_bufferReturner{ bufferReturnImplementation }
		, m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::BufferSRVImplIf>>(returnImplementation, impl) }
	{}

	// ---------------------------------------------

	BufferUAVOwner::BufferUAVOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
	{}

	BufferUAVOwner::operator BufferUAV()
	{
		return BufferUAV{ m_implementation.get() };
	}

	BufferUAVOwner::operator const BufferUAV() const
	{
		return BufferUAV{ m_implementation.get() };
	}

	BufferUAV BufferUAVOwner::resource() const
	{
		return BufferUAV{ m_implementation.get() };
	}

	BufferUAVOwner::operator BufferOwner()
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferUAVOwner::operator BufferOwner() const
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferUAVOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	BufferUAVOwner::BufferUAVOwner(
		engine::shared_ptr<implementation::BufferUAVImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::BufferUAVImplIf>)> returnImplementation,
		engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
		engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation)
		: m_bufferImplementation{ bufferimpl }
		, m_bufferReturner{ bufferReturnImplementation }
		, m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::BufferUAVImplIf>>(returnImplementation, impl) }
	{}

	// ---------------------------------------------

	BufferIBVOwner::BufferIBVOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
	{}

	BufferIBVOwner::operator BufferIBV()
	{
		return BufferIBV{ m_implementation.get() };
	}

	BufferIBVOwner::operator const BufferIBV() const
	{
		return BufferIBV{ m_implementation.get() };
	}

	BufferIBV BufferIBVOwner::resource() const
	{
		return BufferIBV{ m_implementation.get() };
	}

	BufferIBVOwner::operator BufferOwner()
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferIBVOwner::operator BufferOwner() const
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferIBVOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	BufferIBVOwner::BufferIBVOwner(
		engine::shared_ptr<implementation::BufferIBVImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::BufferIBVImplIf>)> returnImplementation,
		engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
		engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation)
		: m_bufferImplementation{ bufferimpl }
		, m_bufferReturner{ bufferReturnImplementation }
		, m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::BufferIBVImplIf>>(returnImplementation, impl) }
	{}

	// ---------------------------------------------

	BufferCBVOwner::BufferCBVOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
	{}

	BufferCBVOwner::operator BufferCBV()
	{
		return BufferCBV{ m_implementation.get() };
	}

	BufferCBVOwner::operator const BufferCBV() const
	{
		return BufferCBV{ m_implementation.get() };
	}

	BufferCBV BufferCBVOwner::resource() const
	{
		return BufferCBV{ m_implementation.get() };
	}

	BufferCBVOwner::operator BufferOwner()
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferCBVOwner::operator BufferOwner() const
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferCBVOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	BufferCBVOwner::BufferCBVOwner(
		engine::shared_ptr<implementation::BufferCBVImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::BufferCBVImplIf>)> returnImplementation,
		engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
		engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation)
		: m_bufferImplementation{ bufferimpl }
		, m_bufferReturner{ bufferReturnImplementation }
		, m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::BufferCBVImplIf>>(returnImplementation, impl) }
	{}

	// ---------------------------------------------

	BufferVBVOwner::BufferVBVOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
	{}

	BufferVBVOwner::operator BufferVBV()
	{
		return BufferVBV{ m_implementation.get() };
	}

	BufferVBVOwner::operator const BufferVBV() const
	{
		return BufferVBV{ m_implementation.get() };
	}

	BufferVBV BufferVBVOwner::resource() const
	{
		return BufferVBV{ m_implementation.get() };
	}

	BufferVBVOwner::operator BufferOwner()
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferVBVOwner::operator BufferOwner() const
	{
		return BufferOwner{ m_bufferImplementation, m_bufferReturner };
	}

	BufferVBVOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	BufferVBVOwner::BufferVBVOwner(
		engine::shared_ptr<implementation::BufferVBVImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::BufferVBVImplIf>)> returnImplementation,
		engine::shared_ptr<implementation::BufferImplIf> bufferimpl,
		engine::shared_ptr<Returner<implementation::BufferImplIf>> bufferReturnImplementation)
		: m_bufferImplementation{ bufferimpl }
		, m_bufferReturner{ bufferReturnImplementation }
		, m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::BufferVBVImplIf>>(returnImplementation, impl) }
	{}

	// ---------------------------------------------

    BindlessBufferSRVOwner::BindlessBufferSRVOwner()
        : m_implementation{ nullptr }
        , m_returner{ nullptr }
    {}

    BindlessBufferSRVOwner::operator BindlessBufferSRV()
    {
        return BindlessBufferSRV{ m_implementation.get() };
    }

    BindlessBufferSRVOwner::operator const BindlessBufferSRV() const
    {
        return BindlessBufferSRV{ m_implementation.get() };
    }

    uint32_t BindlessBufferSRVOwner::push(BufferSRVOwner buffer)
    {
        return m_implementation->push(buffer);
    }

    size_t BindlessBufferSRVOwner::size() const
    {
        return m_implementation->size();
    }

    BufferSRV BindlessBufferSRVOwner::get(size_t index)
    {
        return m_implementation->get(index);
    }

    uint64_t BindlessBufferSRVOwner::resourceId() const
    {
        return m_implementation->resourceId();
    }

    BindlessBufferSRVOwner::BindlessBufferSRVOwner(
        engine::shared_ptr<implementation::BindlessBufferSRVImplIf> impl,
        std::function<void(engine::shared_ptr<implementation::BindlessBufferSRVImplIf>)> returnImplementation)
        : m_implementation{ impl }
        , m_returner{ engine::make_shared<Returner<implementation::BindlessBufferSRVImplIf>>(returnImplementation, impl) }
    {}

    BindlessBufferSRVOwner::BindlessBufferSRVOwner(engine::shared_ptr<implementation::BindlessBufferSRVImplIf> implementation,
        engine::shared_ptr<Returner<implementation::BindlessBufferSRVImplIf>> returner)
        : m_implementation{ implementation }
        , m_returner{ returner }
    {}

    // ---------------------------------------------

    BindlessBufferUAVOwner::BindlessBufferUAVOwner()
        : m_implementation{ nullptr }
        , m_returner{ nullptr }
    {}

    BindlessBufferUAVOwner::operator BindlessBufferUAV()
    {
        return BindlessBufferUAV{ m_implementation.get() };
    }

    BindlessBufferUAVOwner::operator const BindlessBufferUAV() const
    {
        return BindlessBufferUAV{ m_implementation.get() };
    }

    uint32_t BindlessBufferUAVOwner::push(BufferUAVOwner buffer)
    {
        return m_implementation->push(buffer);
    }

    size_t BindlessBufferUAVOwner::size() const
    {
        return m_implementation->size();
    }

    BufferUAV BindlessBufferUAVOwner::get(size_t index)
    {
        return m_implementation->get(index);
    }

    uint64_t BindlessBufferUAVOwner::resourceId() const
    {
        return m_implementation->resourceId();
    }

    BindlessBufferUAVOwner::BindlessBufferUAVOwner(
        engine::shared_ptr<implementation::BindlessBufferUAVImplIf> impl,
        std::function<void(engine::shared_ptr<implementation::BindlessBufferUAVImplIf>)> returnImplementation)
        : m_implementation{ impl }
        , m_returner{ engine::make_shared<Returner<implementation::BindlessBufferUAVImplIf>>(returnImplementation, impl) }
    {}

    BindlessBufferUAVOwner::BindlessBufferUAVOwner(engine::shared_ptr<implementation::BindlessBufferUAVImplIf> implementation,
        engine::shared_ptr<Returner<implementation::BindlessBufferUAVImplIf>> returner)
        : m_implementation{ implementation }
        , m_returner{ returner }
    {}

    // ---------------------------------------------

	RaytracingAccelerationStructureOwner::RaytracingAccelerationStructureOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
	{}

	RaytracingAccelerationStructureOwner::operator RaytracingAccelerationStructure()
	{
		return RaytracingAccelerationStructure{ m_implementation.get() };
	}

	RaytracingAccelerationStructureOwner::operator const RaytracingAccelerationStructure() const
	{
		return RaytracingAccelerationStructure{ m_implementation.get() };
	}

	RaytracingAccelerationStructureOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	RaytracingAccelerationStructure RaytracingAccelerationStructureOwner::resource() const
	{
		return RaytracingAccelerationStructure{ m_implementation.get() };
	}

	RaytracingAccelerationStructureOwner::RaytracingAccelerationStructureOwner(
		engine::shared_ptr<implementation::RaytracingAccelerationStructureImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::RaytracingAccelerationStructureImplIf>)> returnImplementation)
		: m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::RaytracingAccelerationStructureImplIf>>(returnImplementation, impl) }
	{}

	RaytracingAccelerationStructureOwner::RaytracingAccelerationStructureOwner(engine::shared_ptr<implementation::RaytracingAccelerationStructureImplIf> implementation,
		engine::shared_ptr<Returner<implementation::RaytracingAccelerationStructureImplIf>> returner)
		: m_implementation{ implementation }
		, m_returner{ returner }
	{}

	// ---------------------------------------------

	TextureOwner::TextureOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
	{}

	TextureOwner::operator Texture()
	{
		return Texture{ m_implementation.get() };
	}

	TextureOwner::operator const Texture() const
	{
		return Texture{ m_implementation.get() };
	}

	TextureOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	Texture TextureOwner::resource() const
	{
		return Texture{ m_implementation.get() };
	}

	TextureOwner::TextureOwner(
		engine::shared_ptr<implementation::TextureImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::TextureImplIf>)> returnImplementation)
		: m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::TextureImplIf>>(returnImplementation, impl) }
	{}

	TextureOwner::TextureOwner(
		engine::shared_ptr<implementation::TextureImplIf> implementation,
		engine::shared_ptr<Returner<implementation::TextureImplIf>> returner)
		: m_implementation{ implementation }
		, m_returner{ returner }
	{}

	// ---------------------------------------------

	TextureSRVOwner::TextureSRVOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
		, m_textureImplementation{ nullptr }
		, m_textureReturner{ nullptr }
	{}

	TextureSRVOwner::operator TextureSRV()
	{
		return TextureSRV{ m_implementation.get() };
	}

	TextureSRVOwner::operator const TextureSRV() const
	{
		return TextureSRV{ m_implementation.get() };
	}

	TextureSRV TextureSRVOwner::resource() const
	{
		return TextureSRV{ m_implementation.get() };
	}

	TextureSRVOwner::operator TextureOwner()
	{
		return TextureOwner{ m_textureImplementation, m_textureReturner };
	}

	TextureSRVOwner::operator TextureOwner() const
	{
		return TextureOwner{ m_textureImplementation, m_textureReturner };
	}

	TextureSRVOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	TextureSRVOwner::TextureSRVOwner(
		engine::shared_ptr<implementation::TextureSRVImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::TextureSRVImplIf>)> returnImplementation,
		engine::shared_ptr<implementation::TextureImplIf> textureImpl,
		engine::shared_ptr<Returner<implementation::TextureImplIf>> textureReturnImplementation)
		: m_textureImplementation{ textureImpl }
		, m_textureReturner{ textureReturnImplementation }
		, m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::TextureSRVImplIf>>(returnImplementation, impl) }
	{}

	// ---------------------------------------------

	TextureUAVOwner::TextureUAVOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
		, m_textureImplementation{ nullptr }
		, m_textureReturner{ nullptr }
	{}

	TextureUAVOwner::operator TextureUAV()
	{
		return TextureUAV{ m_implementation.get() };
	}

	TextureUAVOwner::operator const TextureUAV() const
	{
		return TextureUAV{ m_implementation.get() };
	}

	TextureUAV TextureUAVOwner::resource() const
	{
		return TextureUAV{ m_implementation.get() };
	}

	TextureUAVOwner::operator TextureOwner()
	{
		return TextureOwner{ m_textureImplementation, m_textureReturner };
	}

	TextureUAVOwner::operator TextureOwner() const
	{
		return TextureOwner{ m_textureImplementation, m_textureReturner };
	}

	TextureUAVOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	TextureUAVOwner::TextureUAVOwner(
		engine::shared_ptr<implementation::TextureUAVImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::TextureUAVImplIf>)> returnImplementation,
		engine::shared_ptr<implementation::TextureImplIf> textureImpl,
		engine::shared_ptr<Returner<implementation::TextureImplIf>> textureReturnImplementation)
		: m_textureImplementation{ textureImpl }
		, m_textureReturner{ textureReturnImplementation }
		, m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::TextureUAVImplIf>>(returnImplementation, impl) }
	{}

	// ---------------------------------------------

	TextureDSVOwner::TextureDSVOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
		, m_textureImplementation{ nullptr }
		, m_textureReturner{ nullptr }
	{}

	TextureDSVOwner::operator TextureDSV()
	{
		return TextureDSV{ m_implementation.get() };
	}

	TextureDSVOwner::operator const TextureDSV() const
	{
		return TextureDSV{ m_implementation.get() };
	}

	TextureDSV TextureDSVOwner::resource() const
	{
		return TextureDSV{ m_implementation.get() };
	}

	TextureDSVOwner::operator TextureOwner()
	{
		return TextureOwner{ m_textureImplementation, m_textureReturner };
	}

	TextureDSVOwner::operator TextureOwner() const
	{
		return TextureOwner{ m_textureImplementation, m_textureReturner };
	}

	TextureDSVOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	TextureDSVOwner::TextureDSVOwner(
		engine::shared_ptr<implementation::TextureDSVImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::TextureDSVImplIf>)> returnImplementation,
		engine::shared_ptr<implementation::TextureImplIf> textureImpl,
		engine::shared_ptr<Returner<implementation::TextureImplIf>> textureReturnImplementation)
		: m_textureImplementation{ textureImpl }
		, m_textureReturner{ textureReturnImplementation }
		, m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::TextureDSVImplIf>>(returnImplementation, impl) }
	{}

	// ---------------------------------------------

	TextureRTVOwner::TextureRTVOwner()
		: m_implementation{ nullptr }
		, m_returner{ nullptr }
		, m_textureImplementation{ nullptr }
		, m_textureReturner{ nullptr }
	{}

	TextureRTVOwner::operator TextureRTV()
	{
		return TextureRTV{ m_implementation.get() };
	}

	TextureRTVOwner::operator const TextureRTV() const
	{
		return TextureRTV{ m_implementation.get() };
	}

	TextureRTV TextureRTVOwner::resource() const
	{
		return TextureRTV{ m_implementation.get() };
	}

	TextureRTVOwner::operator TextureOwner()
	{
		return TextureOwner{ m_textureImplementation, m_textureReturner };
	}

	TextureRTVOwner::operator TextureOwner() const
	{
		return TextureOwner{ m_textureImplementation, m_textureReturner };
	}

	TextureRTVOwner::operator bool() const
	{
		return m_implementation.operator bool();
	}

	TextureRTVOwner::TextureRTVOwner(
		engine::shared_ptr<implementation::TextureRTVImplIf> impl,
		std::function<void(engine::shared_ptr<implementation::TextureRTVImplIf>)> returnImplementation,
		engine::shared_ptr<implementation::TextureImplIf> textureImpl,
		engine::shared_ptr<Returner<implementation::TextureImplIf>> textureReturnImplementation)
		: m_textureImplementation{ textureImpl }
		, m_textureReturner{ textureReturnImplementation }
		, m_implementation{ impl }
		, m_returner{ engine::make_shared<Returner<implementation::TextureRTVImplIf>>(returnImplementation, impl) }
	{}

    // ---------------------------------------------

    BindlessTextureSRVOwner::BindlessTextureSRVOwner()
        : m_implementation{ nullptr }
        , m_returner{ nullptr }
    {}

    BindlessTextureSRVOwner::operator BindlessTextureSRV()
    {
        return BindlessTextureSRV{ m_implementation.get() };
    }

    BindlessTextureSRVOwner::operator const BindlessTextureSRV() const
    {
        return BindlessTextureSRV{ m_implementation.get() };
    }

    uint32_t BindlessTextureSRVOwner::push(TextureSRVOwner texture)
    {
        return m_implementation->push(texture);
    }

    size_t BindlessTextureSRVOwner::size() const
    {
        return m_implementation->size();
    }

    TextureSRV BindlessTextureSRVOwner::get(size_t index)
    {
        return m_implementation->get(index);
    }

    uint64_t BindlessTextureSRVOwner::resourceId() const
    {
        return m_implementation->resourceId();
    }

    BindlessTextureSRVOwner::BindlessTextureSRVOwner(
        engine::shared_ptr<implementation::BindlessTextureSRVImplIf> impl,
        std::function<void(engine::shared_ptr<implementation::BindlessTextureSRVImplIf>)> returnImplementation)
        : m_implementation{ impl }
        , m_returner{ engine::make_shared<Returner<implementation::BindlessTextureSRVImplIf>>(returnImplementation, impl) }
    {}

    BindlessTextureSRVOwner::BindlessTextureSRVOwner(engine::shared_ptr<implementation::BindlessTextureSRVImplIf> implementation,
        engine::shared_ptr<Returner<implementation::BindlessTextureSRVImplIf>> returner)
        : m_implementation{ implementation }
        , m_returner{ returner }
    {}

    // ---------------------------------------------

    BindlessTextureUAVOwner::BindlessTextureUAVOwner()
        : m_implementation{ nullptr }
        , m_returner{ nullptr }
    {}

    BindlessTextureUAVOwner::operator BindlessTextureUAV()
    {
        return BindlessTextureUAV{ m_implementation.get() };
    }

    BindlessTextureUAVOwner::operator const BindlessTextureUAV() const
    {
        return BindlessTextureUAV{ m_implementation.get() };
    }

    uint32_t BindlessTextureUAVOwner::push(TextureUAVOwner texture)
    {
        return m_implementation->push(texture);
    }

    size_t BindlessTextureUAVOwner::size() const
    {
        return m_implementation->size();
    }

    TextureUAV BindlessTextureUAVOwner::get(size_t index)
    {
        return m_implementation->get(index);
    }

    uint64_t BindlessTextureUAVOwner::resourceId() const
    {
        return m_implementation->resourceId();
    }

    BindlessTextureUAVOwner::BindlessTextureUAVOwner(
        engine::shared_ptr<implementation::BindlessTextureUAVImplIf> impl,
        std::function<void(engine::shared_ptr<implementation::BindlessTextureUAVImplIf>)> returnImplementation)
        : m_implementation{ impl }
        , m_returner{ engine::make_shared<Returner<implementation::BindlessTextureUAVImplIf>>(returnImplementation, impl) }
    {}

    BindlessTextureUAVOwner::BindlessTextureUAVOwner(engine::shared_ptr<implementation::BindlessTextureUAVImplIf> implementation,
        engine::shared_ptr<Returner<implementation::BindlessTextureUAVImplIf>> returner)
        : m_implementation{ implementation }
        , m_returner{ returner }
    {}

	// ---------------------------------------------
}
