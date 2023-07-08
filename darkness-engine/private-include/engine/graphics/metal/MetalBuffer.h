#pragma once

#include "engine/graphics/Buffer.h"
#include "engine/graphics/metal/MetalHeaders.h"
#include "engine/graphics/metal/MetalConversions.h"
#include "engine/primitives/Color.h"
#include "engine/graphics/DescriptorHandle.h"

struct MetalDeviceMemory{};
struct MetalBuffer{};
struct MetalBufferView{};
struct MetalTexture{};
struct MetalTextureView{};

namespace engine
{
    class Device;
    namespace implementation
    {
        class DeviceImpl;
        class BufferViewImpl;
        class BufferImpl
        {
        public:
            BufferImpl(
                       const Device& device,
                       const BufferDescription& desc);
            
            void* map(const Device& device);
            void unmap(const Device& device);
            
            const BufferDescription::Descriptor& description() const;
            
            MetalBuffer& native();
            const MetalBuffer& native() const;
            
        protected:
            friend class BufferViewImpl;
            const BufferDescription::Descriptor m_description;
            engine::shared_ptr<MetalBuffer> m_buffer{ nullptr };
            engine::shared_ptr<MetalDeviceMemory> m_memory{ nullptr };
        };
        
        class BufferViewImpl
        {
        public:
            BufferViewImpl(
                           const Device& device,
                           const Buffer& buffer,
                           const BufferDescription& desc);
            
            void* map(const Device& device);
            void unmap(const Device& device);
            
            const BufferDescription::Descriptor& description() const;
            
            MetalBufferView& native();
            const MetalBufferView& native() const;
            
        protected:
            BufferDescription::Descriptor m_description;
            engine::shared_ptr<MetalBufferView> m_view{ nullptr };
            engine::shared_ptr<MetalDeviceMemory> m_memory{ nullptr };
        };
        
        class TextureImpl
        {
        public:
            TextureImpl(
                        const Device& device,
                        const TextureDescription& desc);
            TextureImpl(
                        engine::shared_ptr<MetalTexture> image,
                        const TextureDescription& desc);
            
            void* map(const Device& device);
            void unmap(const Device& device);
            
            const TextureDescription::Descriptor& description() const;
            
            MetalTexture& native();
            const MetalTexture& native() const;
            
        protected:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<MetalTexture> m_image{ nullptr };
            engine::shared_ptr<MetalDeviceMemory> m_memory{ nullptr };
        };
        
        class TextureSRVImpl
        {
        public:
            TextureSRVImpl(
                           const Device& device,
                           const Texture& texture,
                           const TextureDescription& desc,
                           SubResource subResources = SubResource());
            
            void* map(const Device& device);
            void unmap(const Device& device);
            
            const TextureDescription::Descriptor& description() const;
            
            Texture texture();
            
            MetalTextureView& native();
            const MetalTextureView& native() const;
            
        private:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<MetalTextureView> m_view{ nullptr };
            Texture m_texture;
        };
        
        class TextureUAVImpl
        {
        public:
            TextureUAVImpl(
                           const Device& device,
                           const Texture& texture,
                           const TextureDescription& desc,
                           SubResource subResources = SubResource());
            
            void* map(const Device& device);
            void unmap(const Device& device);
            
            const TextureDescription::Descriptor& description() const;
            
            Texture texture();
            
            MetalTextureView& native();
            const MetalTextureView& native() const;
            
        private:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<MetalTextureView> m_view{ nullptr };
            Texture m_texture;
        };
        
        class TextureDSVImpl
        {
        public:
            TextureDSVImpl(
                           const Device& device,
                           const Texture& texture,
                           const TextureDescription& desc,
                           SubResource subResources = SubResource());
            
            void* map(const Device& device);
            void unmap(const Device& device);
            
            const TextureDescription::Descriptor& description() const;
            
            Texture texture();
            
            MetalTextureView& native();
            const MetalTextureView& native() const;
            
        private:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<MetalTextureView> m_view{ nullptr };
            Texture m_texture;
        };
        
        class TextureRTVImpl
        {
        public:
            TextureRTVImpl(
                           const Device& device,
                           const Texture& texture,
                           const TextureDescription& desc,
                           SubResource subResources = SubResource());
            
            void* map(const Device& device);
            void unmap(const Device& device);
            
            const TextureDescription::Descriptor& description() const;
            
            Texture texture();
            
            MetalTextureView& native();
            const MetalTextureView& native() const;
            
        private:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<MetalTextureView> m_view{ nullptr };
            Texture m_texture;
        };
    }
}

