#pragma once

#include "containers/memory.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "tools/MemoryAllocator.h"
#include "tools/FreeListOffsetAllocator.h"
#include "tools/ByteRange.h"
#include "tools/Debug.h"
#include "tools/RefCounted.h"
#include "engine/graphics/Resources.h"

#include <atomic>
extern std::atomic<uint64_t> GlobalUniqueHandleId;

namespace engine
{
    class BufferCBV;
    namespace implementation
    {
        struct ConstantBufferUpdates
        {
            BufferCBV buffer;
            tools::ByteRange range;
        };

        struct UploadTask
        {
            tools::ByteRange data;
            BufferCBV target;
        };

        class DescriptorHandleDX12;
        namespace hidden
        {
            class DescriptorHandleData
            {
            public:
                DescriptorHandleData();

                DescriptorHandleData(
                    engine::shared_ptr<tools::FreeListContinuousOffsetAllocator> allocator,
                    tools::ContinousHandle handle);

                // handle move
                DescriptorHandleData(DescriptorHandleData&& _handle) noexcept;
                DescriptorHandleData& operator=(DescriptorHandleData&& _handle) noexcept;

                // disable copy
                DescriptorHandleData(const DescriptorHandleData&) = delete;
                DescriptorHandleData& operator=(const DescriptorHandleData&) = delete;

                ~DescriptorHandleData();
            private:
                friend class DescriptorHandleDX12;
                engine::shared_ptr<tools::FreeListContinuousOffsetAllocator> m_allocator;
                tools::ContinousHandle m_handle;
            };
        }

        class DescriptorHeapImplDX12;
        class DescriptorHandleDX12
        {
        public:
            DescriptorHandleDX12();

            operator bool() const;

            size_t count() const;
            size_t handleSize() const;
            const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle() const;
            const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle() const;
            D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle();
            D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle();
            uint64_t uniqueId() const;

        protected:
            friend class DescriptorHeapImplDX12;

            DescriptorHandleDX12(
                engine::shared_ptr<tools::FreeListContinuousOffsetAllocator> allocator,
                size_t numDescriptors,
                size_t descriptorSize,
                const D3D12_CPU_DESCRIPTOR_HANDLE& cpuStartHandle,
                const D3D12_GPU_DESCRIPTOR_HANDLE& gpuStartHandle);
        private:
            engine::shared_ptr<hidden::DescriptorHandleData> m_data;
            size_t m_descriptorSize;
            D3D12_CPU_DESCRIPTOR_HANDLE m_basecpuStartHandle;
            D3D12_GPU_DESCRIPTOR_HANDLE m_basegpuStartHandle;
            D3D12_CPU_DESCRIPTOR_HANDLE m_localcpuStartHandle;
            D3D12_GPU_DESCRIPTOR_HANDLE m_localgpuStartHandle;
            uint64_t m_uniqueId;
        };

        class DescriptorHeapImplDX12;
        struct DescriptorHeapsDX12
        {
            engine::shared_ptr<DescriptorHeapImplDX12> cbv_srv_uav;
            engine::shared_ptr<DescriptorHeapImplDX12> sampler;
            engine::shared_ptr<DescriptorHeapImplDX12> rtv;
            engine::shared_ptr<DescriptorHeapImplDX12> dsv;

            engine::shared_ptr<DescriptorHeapImplDX12> shaderVisible_cbv_srv_uav;
            engine::shared_ptr<DescriptorHeapImplDX12> shaderVisible_sampler;
        };
    }
}
