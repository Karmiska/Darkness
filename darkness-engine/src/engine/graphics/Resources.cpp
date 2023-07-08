#include "engine/graphics/Resources.h"
#include "engine/graphics/Barrier.h"

#if defined(GRAPHICS_API_DX12)
#include "engine/graphics/dx12/DX12Resources.h"
#endif

#if defined(GRAPHICS_API_VULKAN)
#include "engine/graphics/vulkan/VulkanResources.h"
#endif

#ifdef __APPLE__
#include "engine/graphics/metal/MetalBuffer.h"
#endif


using namespace engine::implementation;
using namespace engine;

namespace engine
{
    Buffer::Buffer(implementation::BufferImplIf* impl)
        : m_impl{ impl }
    {

    }

    void* Buffer::map(const Device& device)
    {
        return m_impl->map(device.native());
    }
    void Buffer::unmap(const Device& device)
    {
        m_impl->unmap(device.native());
    }

    const BufferDescription::Descriptor& Buffer::description() const
    {
        return m_impl->description();
    }

    ResourceState Buffer::state() const
    {
        return m_impl->state();
    }

    void Buffer::state(ResourceState state) const
    {
        m_impl->state(state);
    }

	bool Buffer::operator==(const Buffer& buffer) const
	{
		return m_impl == buffer.m_impl;
	}

	bool Buffer::operator!=(const Buffer& buffer) const
	{
		return m_impl != buffer.m_impl;
	}

	Buffer::operator bool() const
	{
		return m_impl != nullptr;
	}

	BufferSRV::BufferSRV(implementation::BufferSRVImplIf* impl)
		: m_impl{ impl }
	{
        if (m_impl)
            m_uniqueId = m_impl->uniqueId();
        else
            m_uniqueId = 0u;
    }

	BufferSRV::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool BufferSRV::operator==(const BufferSRV& buffer) const
	{
		return m_impl == buffer.m_impl;
	}

	bool BufferSRV::operator!=(const BufferSRV& buffer) const
	{
		return m_impl != buffer.m_impl;
	}

    bool BufferSRV::valid() const { return m_impl; };

    const BufferDescription::Descriptor& BufferSRV::desc() const
    {
        return m_impl->description();
    }

    Buffer BufferSRV::buffer() const
    {
        return m_impl->buffer();
    }

    uint64_t BufferSRV::resourceId() const
    {
        return m_uniqueId;
    }

    BufferUAV::BufferUAV(implementation::BufferUAVImplIf* impl)
        : m_impl{ impl }
    {
        if (m_impl)
            m_uniqueId = m_impl->uniqueId();
        else
            m_uniqueId = 0u;
    }

	BufferUAV::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool BufferUAV::operator==(const BufferUAV& buffer) const
	{
		return m_impl == buffer.m_impl;
	}

	bool BufferUAV::operator!=(const BufferUAV& buffer) const
	{
		return m_impl != buffer.m_impl;
	}

    bool BufferUAV::valid() const { return m_impl; };

    Buffer BufferUAV::buffer() const
    {
        return m_impl->buffer();
    }

    uint64_t BufferUAV::resourceId() const
    {
        return m_uniqueId;
    }

    const BufferDescription::Descriptor& BufferUAV::desc() const
    {
        return m_impl->description();
    }

    BufferIBV::BufferIBV(implementation::BufferIBVImplIf* impl)
        : m_impl{ impl }
    {
    }

	BufferIBV::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool BufferIBV::operator==(const BufferIBV& buffer) const
	{
		return m_impl == buffer.m_impl;
	}

	bool BufferIBV::operator!=(const BufferIBV& buffer) const
	{
		return m_impl != buffer.m_impl;
	}

    bool BufferIBV::valid() const { return m_impl; };

    Buffer BufferIBV::buffer() const
    {
        return m_impl->buffer();
    }

    const BufferDescription::Descriptor& BufferIBV::desc() const
    {
        return m_impl->description();
    }

    BufferCBV::BufferCBV(implementation::BufferCBVImplIf* impl)
        : m_impl{ impl }
    {
    }

	BufferCBV::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool BufferCBV::operator==(const BufferCBV& buffer) const
	{
		return m_impl == buffer.m_impl;
	}

	bool BufferCBV::operator!=(const BufferCBV& buffer) const
	{
		return m_impl != buffer.m_impl;
	}

    bool BufferCBV::valid() const { return m_impl; };

    Buffer BufferCBV::buffer() const
    {
        return m_impl->buffer();
    }

    const BufferDescription::Descriptor& BufferCBV::desc() const
    {
        return m_impl->description();
    }
    
    BufferVBV::BufferVBV(implementation::BufferVBVImplIf* impl)
        : m_impl{ impl }
    {
    }

	BufferVBV::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool BufferVBV::operator==(const BufferVBV& buffer) const
	{
		return m_impl == buffer.m_impl;
	}

	bool BufferVBV::operator!=(const BufferVBV& buffer) const
	{
		return m_impl != buffer.m_impl;
	}

    bool BufferVBV::valid() const { return m_impl; };

    const BufferDescription::Descriptor& BufferVBV::desc() const
    {
        return m_impl->description();
    }

    Buffer BufferVBV::buffer() const
    {
        return m_impl->buffer();
    }

    BindlessBufferSRV::BindlessBufferSRV(implementation::BindlessBufferSRVImplIf* impl)
        : m_impl{ impl }
    {
    }

    bool BindlessBufferSRV::operator==(const BindlessBufferSRV& buffer) const
    {
        return m_impl == buffer.m_impl;
    }

    bool BindlessBufferSRV::operator!=(const BindlessBufferSRV& buffer) const
    {
        return m_impl != buffer.m_impl;
    }

    BindlessBufferSRV::operator bool() const
    {
        return m_impl != nullptr;
    }

    uint32_t BindlessBufferSRV::push(BufferSRVOwner buffer)
    {
        return m_impl->push(buffer);
    }

    size_t BindlessBufferSRV::size() const
    {
        return m_impl->size();
    }

    BufferSRV BindlessBufferSRV::get(size_t index) const
    {
        return m_impl->get(index);
    }

    uint64_t BindlessBufferSRV::resourceId() const
    {
        if(m_impl)
            return m_impl->resourceId();
        return 0u;
    }

    bool BindlessBufferSRV::change() const
    {
        return m_impl->change();
    }

    void BindlessBufferSRV::change(bool value)
    {
        m_impl->change(value);
    }

    BindlessBufferUAV::BindlessBufferUAV(implementation::BindlessBufferUAVImplIf* impl)
        : m_impl{ impl }
    {
    }

    bool BindlessBufferUAV::operator==(const BindlessBufferUAV& buffer) const
    {
        return m_impl == buffer.m_impl;
    }

    bool BindlessBufferUAV::operator!=(const BindlessBufferUAV& buffer) const
    {
        return m_impl != buffer.m_impl;
    }

    BindlessBufferUAV::operator bool() const
    {
        return m_impl != nullptr;
    }

    uint32_t BindlessBufferUAV::push(BufferUAVOwner buffer)
    {
        return m_impl->push(buffer);
    }

    size_t BindlessBufferUAV::size() const
    {
        return m_impl->size();
    }

    BufferUAV BindlessBufferUAV::get(size_t index) const
    {
        return m_impl->get(index);
    }

    uint64_t BindlessBufferUAV::resourceId() const
    {
        if(m_impl)
            return m_impl->resourceId();
        return 0u;
    }

    bool BindlessBufferUAV::change() const
    {
        return m_impl->change();
    }

    void BindlessBufferUAV::change(bool value)
    {
        m_impl->change(value);
    }

	RaytracingAccelerationStructure::RaytracingAccelerationStructure(implementation::RaytracingAccelerationStructureImplIf* impl)
		: m_impl{ impl }
	{

	}

	uint64_t RaytracingAccelerationStructure::resourceId() const
	{
		if (m_impl)
			return m_impl->resourceId();
		return 0u;
	}

	ResourceState RaytracingAccelerationStructure::state() const
	{
		return m_impl->state();
	}

	void RaytracingAccelerationStructure::state(ResourceState state) const
	{
		m_impl->state(state);
	}

	bool RaytracingAccelerationStructure::operator==(const RaytracingAccelerationStructure& buffer) const
	{
		return m_impl == buffer.m_impl;
	}

	bool RaytracingAccelerationStructure::operator!=(const RaytracingAccelerationStructure& buffer) const
	{
		return m_impl != buffer.m_impl;
	}

	RaytracingAccelerationStructure::operator bool() const
	{
		return m_impl != nullptr;
	}

    Texture::Texture(implementation::TextureImplIf* texture)
        : m_impl{ texture }
    {
    }

    void* Texture::map(const Device& device)
    {
        return m_impl->map(device.native());
    }
    void Texture::unmap(const Device& device)
    {
        m_impl->unmap(device.native());
    }

    bool Texture::valid() const
    {
        return m_impl;
    }

    Format Texture::format() const
    {
        return m_impl->description().format;
    }

    size_t Texture::width() const
    {
        return m_impl->description().width;
    }

    size_t Texture::height() const
    {
        return m_impl->description().height;
    }

    size_t Texture::depth() const
    {
        return m_impl->description().depth;
    }

    size_t Texture::arraySlices() const
    {
        return m_impl->description().arraySlices;
    }

    size_t Texture::mipLevels() const
    {
        return m_impl->description().mipLevels;
    }

    size_t Texture::samples() const
    {
        return m_impl->description().samples;
    }

    ResourceDimension Texture::dimension() const
    {
        return m_impl->description().dimension;
    }

    const TextureDescription::Descriptor& Texture::description() const
    {
        return m_impl->description();
    }

    ResourceState Texture::state(int slice, int mip) const
    {
        return m_impl->state(slice, mip);
    }

    void Texture::state(int slice, int mip, ResourceState state) const
    {
        m_impl->state(slice, mip, state);
    }

	bool Texture::operator==(const Texture& texture) const
	{
		return m_impl == texture.m_impl;
	}

	bool Texture::operator!=(const Texture& texture) const
	{
		return m_impl != texture.m_impl;
	}

	Texture::operator bool() const
	{
		return m_impl != nullptr;
	}

    TextureSRV::TextureSRV(implementation::TextureSRVImplIf* view)
        : m_impl{ view }
    {
        if (m_impl)
            m_uniqueId = m_impl->uniqueId();
        else
            m_uniqueId = 0u;
    }

    Format TextureSRV::format() const
    {
        return m_impl->format();
    }

    size_t TextureSRV::width() const
    {
        return m_impl->width();
    }

    size_t TextureSRV::height() const
    {
        return m_impl->height();
    }

    size_t TextureSRV::depth() const
    {
        return m_impl->depth();
    }

	ResourceDimension TextureSRV::dimension() const
	{
		return m_impl->dimension();
	}

    Texture TextureSRV::texture() const
    {
        return m_impl->texture();
    }

    const SubResource& TextureSRV::subResource() const
    {
        return m_impl->subResource();
    }

    uint64_t TextureSRV::resourceId() const
    {
        return m_uniqueId;
    }

	TextureSRV::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool TextureSRV::operator==(const TextureSRV& texture) const
	{
		return m_impl == texture.m_impl;
	}

	bool TextureSRV::operator!=(const TextureSRV& texture) const
	{
		return m_impl != texture.m_impl;
	}

    bool TextureSRV::valid() const { return m_impl; };

    TextureUAV::TextureUAV(implementation::TextureUAVImplIf* view)
        : m_impl{ view }
    {
        if (m_impl)
            m_uniqueId = m_impl->uniqueId();
        else
            m_uniqueId = 0u;
    }

	TextureUAV::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool TextureUAV::operator==(const TextureUAV& texture) const
	{
		return m_impl == texture.m_impl;
	}

	bool TextureUAV::operator!=(const TextureUAV& texture) const
	{
		return m_impl != texture.m_impl;
	}

    bool TextureUAV::valid() const { return m_impl; };

    Format TextureUAV::format() const
    {
        return m_impl->format();
    }

    size_t TextureUAV::width() const
    {
        return m_impl->width();
    }

    size_t TextureUAV::height() const
    {
        return m_impl->height();
    }

    size_t TextureUAV::depth() const
    {
        return m_impl->depth();
    }

	ResourceDimension TextureUAV::dimension() const
	{
		return m_impl->dimension();
	}

    Texture TextureUAV::texture() const
    {
        return m_impl->texture();
    }

    const SubResource& TextureUAV::subResource() const
    {
        return m_impl->subResource();
    }

    uint64_t TextureUAV::resourceId() const
    {
        return m_uniqueId;
    }

    TextureDSV::TextureDSV(implementation::TextureDSVImplIf* view)
        : m_impl{ view }
    {
    }

    Format TextureDSV::format() const
    {
        return m_impl->format();
    }

    size_t TextureDSV::width() const
    {
        return m_impl->width();
    }

    size_t TextureDSV::height() const
    {
        return m_impl->height();
    }

	TextureDSV::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool TextureDSV::operator==(const TextureDSV& texture) const
	{
		return m_impl == texture.m_impl;
	}

	bool TextureDSV::operator!=(const TextureDSV& texture) const
	{
		return m_impl != texture.m_impl;
	}

    bool TextureDSV::valid() const { return m_impl; };

    Texture TextureDSV::texture() const
    {
        return m_impl->texture();
    }

    const SubResource& TextureDSV::subResource() const
    {
        return m_impl->subResource();
    }

    TextureRTV::TextureRTV(implementation::TextureRTVImplIf* view)
        : m_impl{ view }
    {
    }

	TextureRTV::operator bool() const
	{
		return m_impl != nullptr;
	}

	bool TextureRTV::operator==(const TextureRTV& texture) const
	{
		return m_impl == texture.m_impl;
	}

	bool TextureRTV::operator!=(const TextureRTV& texture) const
	{
		return m_impl != texture.m_impl;
	}

    bool TextureRTV::valid() const { return m_impl; };

    Format TextureRTV::format() const
    {
        return m_impl->format();
    }

    size_t TextureRTV::width() const
    {
        return m_impl->width();
    }

    size_t TextureRTV::height() const
    {
        return m_impl->height();
    }

    const SubResource& TextureRTV::subResource() const
    {
        return m_impl->subResource();
    }

    Texture TextureRTV::texture() const
    {
        return m_impl->texture();
    }

    BindlessTextureSRV::BindlessTextureSRV(implementation::BindlessTextureSRVImplIf* texture)
        : m_impl{ texture }
    {
    }

    bool BindlessTextureSRV::operator==(const BindlessTextureSRV& texture) const
    {
        return m_impl == texture.m_impl;
    }

    bool BindlessTextureSRV::operator!=(const BindlessTextureSRV& texture) const
    {
        return m_impl != texture.m_impl;
    }

    BindlessTextureSRV::operator bool() const
    {
        return m_impl != nullptr;
    }

    uint32_t BindlessTextureSRV::push(TextureSRVOwner texture)
    {
        return m_impl->push(texture);
    }

    size_t BindlessTextureSRV::size() const
    {
        return m_impl->size();
    }

    TextureSRV BindlessTextureSRV::get(size_t index) const
    {
        return m_impl->get(index);
    }

    uint64_t BindlessTextureSRV::resourceId() const
    {
        if(m_impl)
            return m_impl->resourceId();
        return 0u;
    }

    bool BindlessTextureSRV::change() const
    {
        return m_impl->change();
    }

    void BindlessTextureSRV::change(bool value)
    {
        m_impl->change(value);
    }

    BindlessTextureUAV::BindlessTextureUAV(implementation::BindlessTextureUAVImplIf* texture)
        : m_impl{ texture }
    {
    }

    bool BindlessTextureUAV::operator==(const BindlessTextureUAV& texture) const
    {
        return m_impl == texture.m_impl;
    }

    bool BindlessTextureUAV::operator!=(const BindlessTextureUAV& texture) const
    {
        return m_impl != texture.m_impl;
    }

    BindlessTextureUAV::operator bool() const
    {
        return m_impl != nullptr;
    }

    uint32_t BindlessTextureUAV::push(TextureUAVOwner texture)
    {
        return m_impl->push(texture);
    }

    size_t BindlessTextureUAV::size() const
    {
        return m_impl->size();
    }

    TextureUAV BindlessTextureUAV::get(size_t index) const
    {
        return m_impl->get(index);
    }

    uint64_t BindlessTextureUAV::resourceId() const
    {
        if(m_impl)
            return m_impl->resourceId();
        return 0u;
    }

    bool BindlessTextureUAV::change() const
    {
        return m_impl->change();
    }

    void BindlessTextureUAV::change(bool value)
    {
        m_impl->change(value);
    }
}
