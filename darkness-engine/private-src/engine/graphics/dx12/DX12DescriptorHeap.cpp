#include "engine/graphics/dx12/DX12Headers.h"

#include "engine/graphics/dx12/DX12DescriptorHeap.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12Conversions.h"

#include "engine/graphics/Device.h"
#include "tools/Debug.h"

using namespace tools;

namespace engine
{
    namespace implementation
    {
        DescriptorHeapImplDX12::DescriptorHeapImplDX12(
            const DeviceImplIf* device,
            D3D12_DESCRIPTOR_HEAP_TYPE type,
            D3D12_DESCRIPTOR_HEAP_FLAGS flags,
            uint32_t numDescriptors,
            uint32_t numBackDescriptors)
            : m_allocator{ nullptr }
            , m_backallocator{ nullptr }
            , m_heap{}
            , m_desc{ engine::make_shared<D3D12_DESCRIPTOR_HEAP_DESC>() }
            , m_cpuHandle{ }
            , m_gpuHandle{ }
            , m_descriptorSize{ 0 }
            , m_numdescriptors{ numDescriptors }
            , m_numFrontDescriptors{ m_numdescriptors - numBackDescriptors }
            , m_numBackDescriptors{ numBackDescriptors }
        {
            memset(m_desc.get(), 0, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
            m_desc->NumDescriptors = numDescriptors;
            m_desc->Type = type;
            m_desc->Flags = flags;
            m_desc->NodeMask = 0; // multi-adapter support 

            auto res = static_cast<const DeviceImplDX12*>(device)->device()->CreateDescriptorHeap(
                m_desc.get(),
                DARKNESS_IID_PPV_ARGS(m_heap.GetAddressOf()));
            ASSERT(SUCCEEDED(res));

            m_descriptorSize = static_cast<const DeviceImplDX12*>(device)->device()->GetDescriptorHandleIncrementSize(type);
            m_cpuHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
            m_gpuHandle = m_heap->GetGPUDescriptorHandleForHeapStart();

            //m_allocator = engine::make_shared<tools::MemoryAllocator>(
            //    ByteRange(m_cpuHandle.ptr, m_cpuHandle.ptr + (m_descriptorSize * m_numdescriptors)), 1);
            m_allocator = engine::make_shared<tools::FreeListContinuousOffsetAllocator>(m_numFrontDescriptors);
            if(numBackDescriptors > 0)
                m_backallocator = engine::make_shared<tools::FreeListContinuousOffsetAllocator>(m_numBackDescriptors);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapImplDX12::getCpuHeapStart()
        {
            return m_cpuHandle;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapImplDX12::getGpuHeapStart()
        {
            return m_gpuHandle;
        }

        DescriptorHandleDX12 DescriptorHeapImplDX12::getDescriptor(size_t count)
        {
            return DescriptorHandleDX12(
                m_allocator, 
                count, 
                m_descriptorSize, 
                m_cpuHandle, 
                m_gpuHandle);
        }

        DescriptorHandleDX12 DescriptorHeapImplDX12::getBackDescriptor(size_t count)
        {
            return DescriptorHandleDX12(
                m_backallocator,
                count, 
                m_descriptorSize, 
                D3D12_CPU_DESCRIPTOR_HANDLE{ m_cpuHandle.ptr + (m_numFrontDescriptors * m_descriptorSize)},
                D3D12_GPU_DESCRIPTOR_HANDLE{ m_gpuHandle.ptr + (m_numFrontDescriptors * m_descriptorSize)});
        }

        D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapImplDX12::type() const
        {
            return m_desc->Type;
        }

        void DescriptorHeapImplDX12::reset()
        {
            m_allocator = engine::make_shared<tools::FreeListContinuousOffsetAllocator>(m_numdescriptors);
        }

        ID3D12DescriptorHeap* DescriptorHeapImplDX12::native()
        {
            return m_heap.Get();
        }
    }
}
