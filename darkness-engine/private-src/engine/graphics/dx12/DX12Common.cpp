#include "engine/graphics/dx12/DX12Common.h"
#include "engine/graphics/dx12/DX12CommandList.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Common.h"

#include <inttypes.h>

namespace engine
{
    namespace implementation
    {
        namespace hidden
        {
            DescriptorHandleData::DescriptorHandleData()
                : m_allocator{ nullptr }
                , m_handle{}
            {}

            DescriptorHandleData::DescriptorHandleData(
                engine::shared_ptr<tools::FreeListContinuousOffsetAllocator> allocator,
                tools::ContinousHandle handle)
                : m_allocator{ allocator }
                , m_handle{ handle }
            {}

            // handle move
            DescriptorHandleData::DescriptorHandleData(DescriptorHandleData&& _handle) noexcept
                : m_allocator{ nullptr }
                , m_handle{}
            {
                std::swap(m_allocator, _handle.m_allocator);
                std::swap(m_handle, _handle.m_handle);
            }

            DescriptorHandleData& DescriptorHandleData::operator=(DescriptorHandleData&& _handle) noexcept
            {
                std::swap(m_allocator, _handle.m_allocator);
                std::swap(m_handle, _handle.m_handle);
                return *this;
            }

            DescriptorHandleData::~DescriptorHandleData()
            {
                if (m_allocator && m_handle.offset != tools::InvalidContinousHandle)
                {
                    m_allocator->free(m_handle);
                }
            }
        }

        DescriptorHandleDX12::DescriptorHandleDX12()
            : m_data{ nullptr }
            , m_descriptorSize{ 0 }
            , m_basecpuStartHandle{ 0 }
            , m_basegpuStartHandle{ 0 }
            , m_localcpuStartHandle{ 0 }
            , m_localgpuStartHandle{ 0 }
            , m_uniqueId{ 0 }
        {}

        DescriptorHandleDX12::operator bool() const
        {
            return (bool)m_data;
        }

        size_t DescriptorHandleDX12::count() const
        {
            if (m_data)
                return m_data->m_handle.length;
            return 0;
        }
        size_t DescriptorHandleDX12::handleSize() const
        {
            return m_descriptorSize;
        }
        const D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandleDX12::cpuHandle() const
        {
            return m_localcpuStartHandle;
        }
        const D3D12_GPU_DESCRIPTOR_HANDLE& DescriptorHandleDX12::gpuHandle() const
        {
            return m_localgpuStartHandle;
        }
        D3D12_CPU_DESCRIPTOR_HANDLE& DescriptorHandleDX12::cpuHandle()
        {
            return m_localcpuStartHandle;
        }
        D3D12_GPU_DESCRIPTOR_HANDLE& DescriptorHandleDX12::gpuHandle()
        {
            return m_localgpuStartHandle;
        }
        uint64_t DescriptorHandleDX12::uniqueId() const
        {
            return m_uniqueId;
        }

        DescriptorHandleDX12::DescriptorHandleDX12(
            engine::shared_ptr<tools::FreeListContinuousOffsetAllocator> allocator,
            size_t numDescriptors,
            size_t descriptorSize,
            const D3D12_CPU_DESCRIPTOR_HANDLE& cpuStartHandle,
            const D3D12_GPU_DESCRIPTOR_HANDLE& gpuStartHandle)
            : m_data{
                engine::make_shared<hidden::DescriptorHandleData>(
                    allocator,
                    allocator->allocate(numDescriptors)) }
            , m_descriptorSize{ descriptorSize }
            , m_basecpuStartHandle{ cpuStartHandle }
            , m_basegpuStartHandle{ gpuStartHandle }
            , m_localcpuStartHandle{ 0 }
            , m_localgpuStartHandle{ 0 }
            , m_uniqueId{ GlobalUniqueHandleId++ }
        {
            m_localcpuStartHandle = { m_basecpuStartHandle.ptr + (m_data->m_handle.offset * m_descriptorSize) };
            m_localgpuStartHandle = { m_basegpuStartHandle.ptr + (m_data->m_handle.offset * m_descriptorSize) };
        }
    }
}
