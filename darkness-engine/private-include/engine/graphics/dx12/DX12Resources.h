#pragma once

#include "engine/graphics/ResourcesImplIf.h"
#include "engine/graphics/Resources.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Common.h"
#include "engine/graphics/dx12/DX12Conversions.h"
#include "engine/primitives/Color.h"
#include "engine/graphics/dx12/DX12FormatSetups.h"
#include "tools/ComPtr.h"

struct ID3D12Resource;

namespace engine
{
    class Buffer;

    namespace implementation
    {
		void debugPrintCurrentStatus();

        constexpr size_t BindlessInitialAllocationSize = 1000;
        class DeviceImplDX12;

        class BufferImplDX12 : public BufferImplIf
        {
        public:
            BufferImplDX12(
                const DeviceImplDX12& device,
                const BufferDescription& desc);

            BufferImplDX12(BufferImplDX12&&) = default;
            BufferImplDX12& operator=(BufferImplDX12&&) = default;
            BufferImplDX12(const BufferImplDX12&) = delete;
            BufferImplDX12& operator=(const BufferImplDX12&) = delete;
            ~BufferImplDX12();

            void* map(const DeviceImplIf* device) override;
            void unmap(const DeviceImplIf* device) override;

            const BufferDescription::Descriptor& description() const override;
            ResourceState state() const override;
            void state(ResourceState _state) override;

            ID3D12Resource* native() const;
			bool operator==(const BufferImplDX12& buff) const;

        protected:
            const BufferDescription::Descriptor m_description;
            tools::ComPtr<ID3D12Resource> m_buffer;
            ResourceState m_state;
            size_t m_bufferSize;
        };

        class BufferSRVImplDX12 : public BufferSRVImplIf
        {
        public:
            BufferSRVImplDX12(
                const DeviceImplDX12& device,
                const Buffer& buffer,
                const BufferDescription& desc);
            BufferSRVImplDX12(const BufferSRVImplDX12&) = delete;
            BufferSRVImplDX12& operator=(const BufferSRVImplDX12&) = delete;

            const BufferDescription::Descriptor& description() const override;

            D3D12_CPU_DESCRIPTOR_HANDLE& native();
            const D3D12_CPU_DESCRIPTOR_HANDLE& native() const;

            Buffer buffer() const override;

            uint64_t uniqueId() const override;

        protected:
            BufferDescription::Descriptor m_description;
            DescriptorHandleDX12 m_viewHandle;
            Buffer m_buffer;
            uint64_t m_uniqueId;
        };

        class BufferUAVImplDX12 : public BufferUAVImplIf
        {
        public:
            BufferUAVImplDX12(
                const DeviceImplDX12& device,
                const Buffer& buffer,
                const BufferDescription& desc);
            BufferUAVImplDX12(const BufferUAVImplDX12&) = delete;
            BufferUAVImplDX12& operator=(const BufferUAVImplDX12&) = delete;

            const BufferDescription::Descriptor& description() const override;

            D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle();
            const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle() const;

            D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle();
            const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle() const;

            Buffer buffer() const override;

            uint64_t uniqueId() const override;

            size_t structureCounterOffsetBytes() const override;

            DescriptorHandleDX12& viewClearHandle() { return m_viewClearHandle; }

        protected:
            BufferDescription::Descriptor m_description;
            DescriptorHandleDX12 m_viewHandle;
            DescriptorHandleDX12 m_viewClearHandle;
            //tools::ComPtr<ID3D12Resource> m_counterBuffer;
            Buffer m_buffer;
            uint64_t m_uniqueId;
        };

        class BufferIBVImplDX12 : public BufferIBVImplIf
        {
        public:
            BufferIBVImplDX12(
                const DeviceImplDX12& device,
                const Buffer& buffer,
                const BufferDescription& desc);

            const BufferDescription::Descriptor& description() const override;

            D3D12_CPU_DESCRIPTOR_HANDLE& native();
            const D3D12_CPU_DESCRIPTOR_HANDLE& native() const;

            Buffer buffer() const override;
            const D3D12_INDEX_BUFFER_VIEW* view() const;

            uint64_t uniqueId() const override
            {
                return m_viewHandle.uniqueId();
            }
        protected:
            BufferDescription::Descriptor m_description;
            DescriptorHandleDX12 m_viewHandle;
            tools::ComPtr<ID3D12Resource> m_counterBuffer;
            Buffer m_buffer;

        private:
            D3D12_INDEX_BUFFER_VIEW m_view;
        };

        class BufferCBVImplDX12 : public BufferCBVImplIf
        {
        public:
            BufferCBVImplDX12(
                const DeviceImplDX12& device,
                const Buffer& buffer,
                const BufferDescription& desc);

            const BufferDescription::Descriptor& description() const override;

            D3D12_CPU_DESCRIPTOR_HANDLE& native();
            const D3D12_CPU_DESCRIPTOR_HANDLE& native() const;

            Buffer buffer() const override;

            uint64_t uniqueId() const override
            {
                return m_viewHandle.uniqueId();
            }

        protected:
            BufferDescription::Descriptor m_description;
            DescriptorHandleDX12 m_viewHandle;
            tools::ComPtr<ID3D12Resource> m_counterBuffer;
            Buffer m_buffer;
        };

        class BufferVBVImplDX12 : public BufferVBVImplIf
        {
        public:
            BufferVBVImplDX12(
                const DeviceImplDX12& device,
                const Buffer& buffer,
                const BufferDescription& desc);

            const BufferDescription::Descriptor& description() const override;

            D3D12_CPU_DESCRIPTOR_HANDLE& native();
            const D3D12_CPU_DESCRIPTOR_HANDLE& native() const;

            Buffer buffer() const override;
            const D3D12_VERTEX_BUFFER_VIEW* view() const;

            uint64_t uniqueId() const override
            {
                return m_viewHandle.uniqueId();
            }
        protected:
            BufferDescription::Descriptor m_description;
            DescriptorHandleDX12 m_viewHandle;
            tools::ComPtr<ID3D12Resource> m_counterBuffer;
            Buffer m_buffer;

        private:
            D3D12_VERTEX_BUFFER_VIEW m_view;
        };

        class BindlessBufferSRVImplDX12 : public BindlessBufferSRVImplIf
        {
        public:
            BindlessBufferSRVImplDX12(const DeviceImplDX12& device);

            bool operator==(const BindlessBufferSRVImplDX12& buff) const;

            uint32_t push(BufferSRVOwner buffer) override;
            size_t size() const override;
            BufferSRV get(size_t index) override;
            uint64_t resourceId() const override;
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableGPUHandle() const;
            void updateDescriptors(DeviceImplIf* device) override;
            bool change() const override;
            void change(bool value) override;
        protected:
            engine::vector<BufferSRVOwner> m_buffers;
            uint64_t m_resourceId;
            DescriptorHandleDX12 m_handle;
            size_t m_lastDescriptorWritten;
            bool m_change;
        };

        class BindlessBufferUAVImplDX12 : public BindlessBufferUAVImplIf
        {
        public:
            BindlessBufferUAVImplDX12(const DeviceImplDX12& device);

            bool operator==(const BindlessBufferUAVImplDX12& buff) const;

            uint32_t push(BufferUAVOwner buffer) override;
            size_t size() const override;
            BufferUAV get(size_t index) override;
            uint64_t resourceId() const override;
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableGPUHandle() const;
            void updateDescriptors(DeviceImplIf* device) override;
            bool change() const override;
            void change(bool value) override;
        protected:
            engine::vector<BufferUAVOwner> m_buffers;
            uint64_t m_resourceId;
            DescriptorHandleDX12 m_handle;
            size_t m_lastDescriptorWritten;
            bool m_change;
        };

		class RaytracingAccelerationStructureImplDX12 : public RaytracingAccelerationStructureImplIf
		{
		public:
			RaytracingAccelerationStructureImplDX12(
				const Device& device,
				BufferSRV vertexBuffer,
				BufferIBV indexBuffer,
				const BufferDescription& desc);

			RaytracingAccelerationStructureImplDX12(RaytracingAccelerationStructureImplDX12&&) = default;
			RaytracingAccelerationStructureImplDX12& operator=(RaytracingAccelerationStructureImplDX12&&) = default;
			RaytracingAccelerationStructureImplDX12(const RaytracingAccelerationStructureImplDX12&) = default;
			RaytracingAccelerationStructureImplDX12& operator=(const RaytracingAccelerationStructureImplDX12&) = default;
			~RaytracingAccelerationStructureImplDX12();

			const BufferDescription::Descriptor& description() const override;
			ResourceState state() const override;
			void state(ResourceState _state) override;

			bool operator==(const RaytracingAccelerationStructureImplDX12& buff) const;

			uint64_t resourceId() const override;

		protected:
			const BufferDescription::Descriptor m_description;
			BufferOwner m_bottomLevel;
			BufferOwner m_topLevel;
			BufferOwner m_scratch;
			BufferOwner m_instanceDesc;
			ResourceState m_state;
			size_t m_bufferSize;
		};

        class TextureImplDX12 : public TextureImplIf
        {
        public:
            TextureImplDX12(
                const DeviceImplDX12& device,
                const TextureDescription& desc);

            TextureImplDX12(
                const DeviceImplDX12& device,
                const TextureDescription& desc,
                ID3D12Resource* resource,
                ResourceState currentState);

            TextureImplDX12(TextureImplDX12&&) = default;
            TextureImplDX12& operator=(TextureImplDX12&&) = default;
            TextureImplDX12(const TextureImplDX12&) = default;
            TextureImplDX12& operator=(const TextureImplDX12&) = default;
            ~TextureImplDX12();

            void* map(const DeviceImplIf* device) override;
            void unmap(const DeviceImplIf* device) override;

            const TextureDescription::Descriptor& description() const override;

            ID3D12Resource* native() const;

            ResourceState state(int slice, int mip) const override;
            void state(int slice, int mip, ResourceState state) override;

			bool operator==(const TextureImplDX12& tex) const;

        protected:
            TextureDescription::Descriptor m_description;
            tools::ComPtr<ID3D12Resource> m_texture;
            engine::vector<ResourceState> m_state;
			bool m_attached;
        };

        class TextureSRVImplDX12 : public TextureSRVImplIf
        {
        public:
            TextureSRVImplDX12(
                const DeviceImplDX12& device,
                const Texture& texture,
                const TextureDescription& desc,
                SubResource subResources = SubResource());

            const TextureDescription::Descriptor& description() const override;

            Texture texture() const override;

            Format format() const override;
            size_t width() const override;
            size_t height() const override;
            size_t depth() const override;
			ResourceDimension dimension() const override;

            D3D12_CPU_DESCRIPTOR_HANDLE& native();
            const D3D12_CPU_DESCRIPTOR_HANDLE& native() const;

            uint64_t uniqueId() const override;

            const SubResource& subResource() const override;

        protected:
            TextureDescription::Descriptor m_description;
            DescriptorHandleDX12 m_viewHandle;
            Texture m_texture;
            SubResource m_subResources;
            uint64_t m_uniqueId;
        };

        class TextureUAVImplDX12 : public TextureUAVImplIf
        {
        public:
            TextureUAVImplDX12(
                const DeviceImplDX12& device,
                const Texture& texture,
                const TextureDescription& desc,
                SubResource subResources = SubResource());

            const TextureDescription::Descriptor& description() const override;

            /*void setCounterValue(uint32_t value);
            uint32_t getCounterValue();*/

            Texture texture() const override;

            Format format() const override;
            size_t width() const override;
            size_t height() const override;
            size_t depth() const override;
			ResourceDimension dimension() const override;

            D3D12_CPU_DESCRIPTOR_HANDLE& native();
            const D3D12_CPU_DESCRIPTOR_HANDLE& native() const;

            uint64_t uniqueId() const override;

            const SubResource& subResource() const override;
        protected:
            TextureDescription::Descriptor m_description;
            DescriptorHandleDX12 m_viewHandle;
            tools::ComPtr<ID3D12Resource> m_counterBuffer;
            Texture m_texture;
            SubResource m_subResources;
            uint64_t m_uniqueId;
        };

        class TextureDSVImplDX12 : public TextureDSVImplIf
        {
        public:
            TextureDSVImplDX12(
                const DeviceImplDX12& device,
                const Texture& texture,
                const TextureDescription& desc,
                SubResource subResources = SubResource());

            const TextureDescription::Descriptor& description() const override;

            Texture texture() const override;

            Format format() const override;
            size_t width() const override;
            size_t height() const override;

            D3D12_CPU_DESCRIPTOR_HANDLE& native();
            const D3D12_CPU_DESCRIPTOR_HANDLE& native() const;

            uint64_t uniqueId() const override
            {
                return m_viewHandle.uniqueId();
            }
            const SubResource& subResource() const override;
        protected:
            TextureDescription::Descriptor m_description;
            DescriptorHandleDX12 m_viewHandle;
            Texture m_texture;
            SubResource m_subResources;
        };

        class TextureRTVImplDX12 : public TextureRTVImplIf
        {
        public:
            TextureRTVImplDX12(
                const DeviceImplDX12& device,
                const Texture& texture,
                const TextureDescription& desc,
                SubResource subResources = SubResource());

            const TextureDescription::Descriptor& description() const override;

            Texture texture() const override;

            Format format() const override;
            size_t width() const override;
            size_t height() const override;

            D3D12_CPU_DESCRIPTOR_HANDLE& native();
            const D3D12_CPU_DESCRIPTOR_HANDLE& native() const;

            uint64_t uniqueId() const override
            {
                return m_viewHandle.uniqueId();
            }
            const SubResource& subResource() const override;
        protected:
            TextureDescription::Descriptor m_description;
            DescriptorHandleDX12 m_viewHandle;
            Texture m_texture;
            SubResource m_subResources;
        };

        class BindlessTextureSRVImplDX12 : public BindlessTextureSRVImplIf
        {
        public:
            BindlessTextureSRVImplDX12(const DeviceImplDX12& device);

            bool operator==(const BindlessTextureSRVImplDX12& tex) const;

            uint32_t push(TextureSRVOwner texture) override;
            size_t size() const override;
            TextureSRV get(size_t index) override;
            uint64_t resourceId() const override;
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableGPUHandle() const;
            void updateDescriptors(DeviceImplIf* device) override;
            bool change() const override;
            void change(bool value) override;
        protected:
            engine::vector<TextureSRVOwner> m_textures;
            uint64_t m_resourceId;
            DescriptorHandleDX12 m_handle;
            size_t m_lastDescriptorWritten;
            bool m_change;
        };

        class BindlessTextureUAVImplDX12 : public BindlessTextureUAVImplIf
        {
        public:
            BindlessTextureUAVImplDX12(const DeviceImplDX12& device);

            bool operator==(const BindlessTextureUAVImplDX12& tex) const;

            uint32_t push(TextureUAVOwner texture) override;
            size_t size() const override;
            TextureUAV get(size_t index) override;
            uint64_t resourceId() const override;
            D3D12_GPU_DESCRIPTOR_HANDLE descriptorTableGPUHandle() const;
            void updateDescriptors(DeviceImplIf* device) override;
            bool change() const override;
            void change(bool value) override;
        protected:
            engine::vector<TextureUAVOwner> m_textures;
            uint64_t m_resourceId;
            DescriptorHandleDX12 m_handle;
            size_t m_lastDescriptorWritten;
            bool m_change;
        };

    }
}

