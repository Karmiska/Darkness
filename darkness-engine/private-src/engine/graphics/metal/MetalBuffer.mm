#include "engine/graphics/metal/MetalBuffer.h"
#include "engine/graphics/metal/MetalHeaders.h"

#include "tools/Debug.h"

namespace engine
{
    namespace implementation
    {
        BufferImpl::BufferImpl(const Device& device, const BufferDescription& desc)
        : m_description( desc.descriptor )
        , m_buffer{ nullptr }
        , m_memory{ nullptr }
        {
        }
        
        void* BufferImpl::map(const Device& device)
        {
            return nullptr;
        }
        
        void BufferImpl::unmap(const Device& device)
        {
        }
        
        const BufferDescription::Descriptor& BufferImpl::description() const
        {
            return m_description;
        }
        
        MetalBuffer& BufferImpl::native()
        {
            return *m_buffer;
        }
        
        const MetalBuffer& BufferImpl::native() const
        {
            return *m_buffer;
        }
        
        BufferViewImpl::BufferViewImpl(
                                       const Device& device,
                                       const Buffer& buffer,
                                       const BufferDescription& desc)
        : m_description(desc.descriptor)
        , m_view{ nullptr }
        , m_memory{ BufferImplGet::impl(buffer)->m_memory }
        {
        }
        
        void* BufferViewImpl::map(const Device& device)
        {
            return nullptr;
        }
        
        void BufferViewImpl::unmap(const Device& device)
        {
        }
        
        const BufferDescription::Descriptor& BufferViewImpl::description() const
        {
            return m_description;
        }
        
        MetalBufferView& BufferViewImpl::native()
        {
            return *m_view;
        }
        
        const MetalBufferView& BufferViewImpl::native() const
        {
            return *m_view;
        }
        
        TextureImpl::TextureImpl(const Device& device, const TextureDescription& desc)
        : m_description( desc.descriptor )
        , m_image{ nullptr }
        , m_memory{ nullptr }
        {
        }
        
        TextureImpl::TextureImpl(
                                 std::shared_ptr<MetalTexture> image,
                                 const TextureDescription& desc)
        : m_description(desc.descriptor)
        , m_image{ image }
        , m_memory{ nullptr }
        {
        }
        
        void* TextureImpl::map(const Device& device)
        {
            return nullptr;
        }
        
        void TextureImpl::unmap(const Device& device)
        {
        }
        
        const TextureDescription::Descriptor& TextureImpl::description() const
        {
            return m_description;
        }
        
        MetalTexture& TextureImpl::native()
        {
            return *m_image;
        }
        
        const MetalTexture& TextureImpl::native() const
        {
            return *m_image;
        }
        
        TextureSRVImpl::TextureSRVImpl(
                                       const Device& device,
                                       const Texture& texture,
                                       const TextureDescription& desc,
                                       SubResource subResources)
        : m_description(desc.descriptor)
        , m_view{ nullptr }
        , m_texture{ texture }
        {
        }
        
        void* TextureSRVImpl::map(const Device& device)
        {
            return nullptr;
        }
        
        void TextureSRVImpl::unmap(const Device& device)
        {
        }
        
        const TextureDescription::Descriptor& TextureSRVImpl::description() const
        {
            return m_description;
        }
        
        Texture TextureSRVImpl::texture()
        {
            return m_texture;
        }
        
        MetalTextureView& TextureSRVImpl::native()
        {
            return *m_view;
        }
        
        const MetalTextureView& TextureSRVImpl::native() const
        {
            return *m_view;
        }
        
        TextureUAVImpl::TextureUAVImpl(
                                       const Device& device,
                                       const Texture& texture,
                                       const TextureDescription& desc,
                                       SubResource subResources)
        : m_description(desc.descriptor)
        , m_view{ nullptr }
        , m_texture{ texture }
        {
        }
        
        void* TextureUAVImpl::map(const Device& device)
        {
            return nullptr;
        }
        
        void TextureUAVImpl::unmap(const Device& device)
        {
        }
        
        const TextureDescription::Descriptor& TextureUAVImpl::description() const
        {
            return m_description;
        }
        
        Texture TextureUAVImpl::texture()
        {
            return m_texture;
        }
        
        MetalTextureView& TextureUAVImpl::native()
        {
            return *m_view;
        }
        
        const MetalTextureView& TextureUAVImpl::native() const
        {
            return *m_view;
        }
        
        TextureDSVImpl::TextureDSVImpl(
                                       const Device& device,
                                       const Texture& texture,
                                       const TextureDescription& desc,
                                       SubResource subResources)
        : m_description(desc.descriptor)
        , m_view{ nullptr }
        , m_texture{ texture }
        {
        }
        
        void* TextureDSVImpl::map(const Device& device)
        {
            return nullptr;
        }
        
        void TextureDSVImpl::unmap(const Device& device)
        {
        }
        
        const TextureDescription::Descriptor& TextureDSVImpl::description() const
        {
            return m_description;
        }
        
        Texture TextureDSVImpl::texture()
        {
            return m_texture;
        }
        
        MetalTextureView& TextureDSVImpl::native()
        {
            return *m_view;
        }
        
        const MetalTextureView& TextureDSVImpl::native() const
        {
            return *m_view;
        }
        
        TextureRTVImpl::TextureRTVImpl(
                                       const Device& device,
                                       const Texture& texture,
                                       const TextureDescription& desc,
                                       SubResource subResources)
        : m_description(desc.descriptor)
        , m_view{ nullptr }
        , m_texture{ texture }
        {
        }
        
        void* TextureRTVImpl::map(const Device& device)
        {
            return nullptr;
        }
        
        void TextureRTVImpl::unmap(const Device& device)
        {
        }
        
        const TextureDescription::Descriptor& TextureRTVImpl::description() const
        {
            return m_description;
        }
        
        Texture TextureRTVImpl::texture()
        {
            return m_texture;
        }
        
        MetalTextureView& TextureRTVImpl::native()
        {
            return *m_view;
        }
        
        const MetalTextureView& TextureRTVImpl::native() const
        {
            return *m_view;
        }
    }
}
