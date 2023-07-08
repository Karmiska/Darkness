#pragma once

#include "engine/graphics/ResourcesImplIf.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/vulkan/VulkanHeaders.h"
#include "engine/graphics/vulkan/VulkanConversions.h"
#include "engine/primitives/Color.h"
#include "tools/Codegen.h"
#include "tools/image/Image.h"

#include "containers/memory.h"
#include "containers/string.h"

#include <atomic>
extern std::atomic<uint64_t> GlobalUniqueHandleId;

namespace engine
{
    namespace implementation
    {
        class DeviceImplVulkan;
        class BufferViewImpl;
        class BufferImplVulkan : public BufferImplIf
        {
        public:
            BufferImplVulkan(
                const DeviceImplVulkan& device,
                const BufferDescription& desc);

            ~BufferImplVulkan();

            void* map(const DeviceImplIf* device) override;
            void unmap(const DeviceImplIf* device) override;

            const BufferDescription::Descriptor& description() const override;

            ResourceState state() const override;
            void state(ResourceState state) override;

            VkBuffer& native();
            const VkBuffer& native() const;

        protected:
            friend class BufferViewImplVulkan;
            friend class BufferSRVImplVulkan;
            friend class BufferUAVImplVulkan;
            friend class BufferIBVImplVulkan;
            friend class BufferVBVImplVulkan;
            friend class BufferCBVImplVulkan;
            BufferDescription::Descriptor m_description;
            engine::shared_ptr<VkBuffer> m_buffer{ nullptr };
            engine::shared_ptr<VkDeviceMemory> m_memory{ nullptr };
            ResourceState m_state;
        };

        class BufferSRVImplVulkan : public BufferSRVImplIf
        {
        public:
            BufferSRVImplVulkan(
                const DeviceImplVulkan& device,
                const Buffer& buffer,
                const BufferDescription& desc);

            ~BufferSRVImplVulkan();

            const BufferDescription::Descriptor& description() const override;

            VkBufferView& native();
            const VkBufferView& native() const;

            Buffer buffer() const override;

            uint64_t uniqueId() const override
            {
                return m_uniqueId;
            }

        protected:
            BufferDescription::Descriptor m_description;
            engine::shared_ptr<VkBufferView> m_view{ nullptr };
            engine::shared_ptr<VkDeviceMemory> m_memory{ nullptr };
            Buffer m_buffer;
        private:
            uint64_t m_uniqueId;
        };

        class BufferUAVImplVulkan : public BufferUAVImplIf
        {
        public:
            BufferUAVImplVulkan(
                const DeviceImplVulkan& device,
                const Buffer& buffer,
                const BufferDescription& desc);

            const BufferDescription::Descriptor& description() const override;

            VkBufferView& native();
            const VkBufferView& native() const;

            Buffer buffer() const override;

            uint64_t uniqueId() const override;
            size_t structureCounterOffsetBytes() const override;
        protected:
            BufferDescription::Descriptor m_description;
            engine::shared_ptr<VkBufferView> m_view{ nullptr };
            engine::shared_ptr<VkDeviceMemory> m_memory{ nullptr };
            Buffer m_buffer;

        protected:
            friend class PipelineImplVulkan;
            engine::shared_ptr<VkBuffer> m_counterBuffer{ nullptr };
            engine::shared_ptr<VkDeviceMemory> m_counterMemory{ nullptr };
        private:
            uint64_t m_uniqueId;
        };

        class BufferIBVImplVulkan : public BufferIBVImplIf
        {
        public:
            BufferIBVImplVulkan(
                const DeviceImplVulkan& device,
                const Buffer& buffer,
                const BufferDescription& desc);

            const BufferDescription::Descriptor& description() const;

            VkBufferView& native();
            const VkBufferView& native() const;

            Buffer buffer() const override;

            uint64_t uniqueId() const
            {
                return 0;
            }

        protected:
            BufferDescription::Descriptor m_description;
            engine::shared_ptr<VkBufferView> m_view{ nullptr };
            engine::shared_ptr<VkDeviceMemory> m_memory{ nullptr };
            Buffer m_buffer;
        };

        class BufferCBVImplVulkan : public BufferCBVImplIf
        {
        public:
            BufferCBVImplVulkan(
                const DeviceImplVulkan& device,
                const Buffer& buffer,
                const BufferDescription& desc);

            const BufferDescription::Descriptor& description() const;

            VkBufferView& native();
            const VkBufferView& native() const;

            Buffer buffer() const override;

            uint64_t uniqueId() const
            {
                return 0;
            }

        protected:
            BufferDescription::Descriptor m_description;
            engine::shared_ptr<VkBufferView> m_view{ nullptr };
            engine::shared_ptr<VkDeviceMemory> m_memory{ nullptr };
            Buffer m_buffer;
        };

        class BufferVBVImplVulkan : public BufferVBVImplIf
        {
        public:
            BufferVBVImplVulkan(
                const DeviceImplVulkan& device,
                const Buffer& buffer,
                const BufferDescription& desc);

            const BufferDescription::Descriptor& description() const;

            VkBufferView& native();
            const VkBufferView& native() const;

            Buffer buffer() const override;

            uint64_t uniqueId() const
            {
                return 0;
            }

        protected:
            BufferDescription::Descriptor m_description;
            engine::shared_ptr<VkBufferView> m_view{ nullptr };
            engine::shared_ptr<VkDeviceMemory> m_memory{ nullptr };
            Buffer m_buffer;
        };

        class BindlessBufferSRVImplVulkan : public BindlessBufferSRVImplIf
        {
        public:
            BindlessBufferSRVImplVulkan(const DeviceImplVulkan& device);

            bool operator==(const BindlessBufferSRVImplVulkan& buff) const;

            uint32_t push(BufferSRVOwner buffer) override;
            size_t size() const override;
            BufferSRV get(size_t index) override;
            uint64_t resourceId() const override;
            
            void updateDescriptors(DeviceImplIf* device) override;
            bool change() const override;
            void change(bool value) override;
        protected:
            engine::vector<BufferSRVOwner> m_buffers;
            uint64_t m_resourceId;
            bool m_change;
        };

        class BindlessBufferUAVImplVulkan : public BindlessBufferUAVImplIf
        {
        public:
            BindlessBufferUAVImplVulkan(const DeviceImplVulkan& device);

            bool operator==(const BindlessBufferUAVImplVulkan& buff) const;

            uint32_t push(BufferUAVOwner buffer);
            size_t size() const;
            BufferUAV get(size_t index);
            uint64_t resourceId() const;
            
            void updateDescriptors(DeviceImplIf* device) override;
            bool change() const;
            void change(bool value);
        protected:
            engine::vector<BufferUAVOwner> m_buffers;
            uint64_t m_resourceId;
            bool m_change;
        };

        class RaytracingAccelerationStructureImplVulkan : public RaytracingAccelerationStructureImplIf
        {
        public:
            RaytracingAccelerationStructureImplVulkan(
                const Device& device,
                BufferSRV vertexBuffer,
                BufferIBV indexBuffer,
                const BufferDescription& desc);

            RaytracingAccelerationStructureImplVulkan(RaytracingAccelerationStructureImplVulkan&&) = default;
            RaytracingAccelerationStructureImplVulkan& operator=(RaytracingAccelerationStructureImplVulkan&&) = default;
            RaytracingAccelerationStructureImplVulkan(const RaytracingAccelerationStructureImplVulkan&) = default;
            RaytracingAccelerationStructureImplVulkan& operator=(const RaytracingAccelerationStructureImplVulkan&) = default;
            ~RaytracingAccelerationStructureImplVulkan();

            const BufferDescription::Descriptor& description() const;
            ResourceState state() const;
            void state(ResourceState _state);

            bool operator==(const RaytracingAccelerationStructureImplVulkan& buff) const;

            uint64_t resourceId() const;

        protected:
            const BufferDescription::Descriptor m_description;
            BufferOwner m_bottomLevel;
            BufferOwner m_topLevel;
            BufferOwner m_scratch;
            BufferOwner m_instanceDesc;
            ResourceState m_state;
            size_t m_bufferSize;
        };

        class TextureImplVulkan : public TextureImplIf
        {
        public:
            TextureImplVulkan(
                const DeviceImplVulkan& device,
                const TextureDescription& desc);
            TextureImplVulkan(
                engine::shared_ptr<VkImage> image,
                const TextureDescription& desc);

            void* map(const DeviceImplIf* device);
            void unmap(const DeviceImplIf* device);

            const TextureDescription::Descriptor& description() const;

            VkImage& native();
            const VkImage& native() const;
            
            ResourceState state(int slice, int mip) const;
            void state(int slice, int mip, ResourceState state);
        protected:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<VkImage> m_image{ nullptr };
            engine::shared_ptr<VkDeviceMemory> m_memory{ nullptr };
            engine::vector<ResourceState> m_state;
        };

        class TextureSRVImplVulkan : public TextureSRVImplIf
        {
        public:
            TextureSRVImplVulkan(
                const DeviceImplVulkan& device,
                const Texture& texture, 
                const TextureDescription& desc,
                SubResource subResources = SubResource());

            const TextureDescription::Descriptor& description() const;

            Texture texture() const override;
            Format format() const override;
            size_t width() const override;
            size_t height() const override;
            size_t depth() const override;
            ResourceDimension dimension() const override;

            VkImageView& native();
            const VkImageView& native() const;
            
            uint64_t uniqueId() const override
            {
                return m_uniqueId;
            }

            const SubResource& subResource() const  override { return m_subResources; };
        private:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<VkImageView> m_view{ nullptr };
            Texture m_texture;
            SubResource m_subResources;
            uint64_t m_uniqueId;
        };

        class TextureUAVImplVulkan : public TextureUAVImplIf
        {
        public:
            TextureUAVImplVulkan(
                const DeviceImplVulkan& device,
                const Texture& texture,
                const TextureDescription& desc,
                SubResource subResources = SubResource());

            const TextureDescription::Descriptor& description() const;

            void setCounterValue(uint32_t /*value*/) { LOG("TextureUAVImplVulkan setCounterValue not implemented"); };
            uint32_t getCounterValue() {};

            Texture texture() const override;
            Format format() const override;
            size_t width() const override;
            size_t height() const override;
            size_t depth() const override;
            ResourceDimension dimension() const override;

            VkImageView& native();
            const VkImageView& native() const;

            uint64_t uniqueId() const override
            {
                return m_uniqueId;
            }

            const SubResource& subResource() const override { return m_subResources; };
        private:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<VkImageView> m_view{ nullptr };
            Texture m_texture;
            SubResource m_subResources;
            uint64_t m_uniqueId;
        };

        class TextureDSVImplVulkan : public TextureDSVImplIf
        {
        public:
            TextureDSVImplVulkan(
                const DeviceImplVulkan& device,
                const Texture& texture,
                const TextureDescription& desc,
                SubResource subResources = SubResource());

            const TextureDescription::Descriptor& description() const;

            Texture texture() const override;
            Format format() const override;
            size_t width() const override;
            size_t height() const override;

            VkImageView& native();
            const VkImageView& native() const;

            uint64_t uniqueId() const override
            {
                return 0;
            }

            const SubResource& subResource() const override { return m_subResources; };
        private:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<VkImageView> m_view{ nullptr };
            Texture m_texture;
            SubResource m_subResources;
        };

        class TextureRTVImplVulkan : public TextureRTVImplIf
        {
        public:
            TextureRTVImplVulkan(
                const DeviceImplVulkan& device,
                const Texture& texture,
                const TextureDescription& desc,
                SubResource subResources = SubResource());

            const TextureDescription::Descriptor& description() const;

            Texture texture() const override;
            Format format() const override;
            size_t width() const override;
            size_t height() const override;

            VkImageView& native();
            const VkImageView& native() const;

            uint64_t uniqueId() const override
            {
                return 0;
            }

            const SubResource& subResource() const override { return m_subResources; };
        private:
            const TextureDescription::Descriptor m_description;
            engine::shared_ptr<VkImageView> m_view{ nullptr };
            Texture m_texture;
            SubResource m_subResources;
        };

        class BindlessTextureSRVImplVulkan : public BindlessTextureSRVImplIf
        {
        public:
            BindlessTextureSRVImplVulkan(const DeviceImplVulkan& device);

            bool operator==(const BindlessTextureSRVImplVulkan& tex) const;

            uint32_t push(TextureSRVOwner texture);
            size_t size() const;
            TextureSRV get(size_t index);
            uint64_t resourceId() const;
            
            void updateDescriptors(DeviceImplIf* device) override;
            bool change() const;
            void change(bool value);
        protected:
            engine::vector<TextureSRVOwner> m_textures;
            uint64_t m_resourceId;
            bool m_change;
        };

        class BindlessTextureUAVImplVulkan : public BindlessTextureUAVImplIf
        {
        public:
            BindlessTextureUAVImplVulkan(const DeviceImplVulkan& device);

            bool operator==(const BindlessTextureUAVImplVulkan& tex) const;

            uint32_t push(TextureUAVOwner texture);
            size_t size() const;
            TextureUAV get(size_t index);
            uint64_t resourceId() const;
            
            void updateDescriptors(DeviceImplIf* device) override;
            bool change() const;
            void change(bool value);
        protected:
            engine::vector<TextureUAVOwner> m_textures;
            uint64_t m_resourceId;
            bool m_change;
        };
    }
}

