#pragma once

#include "engine/graphics/DescriptorHeapImplIf.h"
#include "tools/ComPtr.h"
#include "tools/MemoryAllocator.h"
#include "tools/FreeListOffsetAllocator.h"
#include "engine/graphics/dx12/DX12Common.h"
#include <cstdint>
#include "containers/memory.h"

/*struct ID3D12DescriptorHeap;
enum D3D12_DESCRIPTOR_HEAP_TYPE;
enum D3D12_DESCRIPTOR_HEAP_FLAGS;
struct D3D12_CPU_DESCRIPTOR_HANDLE;
struct D3D12_GPU_DESCRIPTOR_HANDLE;
struct D3D12_DESCRIPTOR_HEAP_DESC;*/

namespace engine
{
    class DescriptorHandleDX12;
    class RootSignature;
    class Buffer;
    class Sampler;
    class BufferView;
    class TextureSRV;
    namespace shaders
    {
        class PipelineConfiguration;
    }

    namespace implementation
    {
        class DeviceImplIf;
        class DescriptorHeapImplDX12 : public DescriptorHeapImplIf
        {
        public:
            DescriptorHeapImplDX12(
                const DeviceImplIf* device, 
                D3D12_DESCRIPTOR_HEAP_TYPE type,
                D3D12_DESCRIPTOR_HEAP_FLAGS flags,
                uint32_t numDescriptors,
                uint32_t numBackDescriptors = 0);

            D3D12_DESCRIPTOR_HEAP_TYPE type() const;

            DescriptorHandleDX12 getDescriptor(size_t count = 1);
            DescriptorHandleDX12 getBackDescriptor(size_t count = 1);

            D3D12_CPU_DESCRIPTOR_HANDLE getCpuHeapStart();
            D3D12_GPU_DESCRIPTOR_HANDLE getGpuHeapStart();

            void reset() override;
            ID3D12DescriptorHeap* native();
        private:
            engine::shared_ptr<tools::FreeListContinuousOffsetAllocator> m_allocator;
            engine::shared_ptr<tools::FreeListContinuousOffsetAllocator> m_backallocator;
            tools::ComPtr<ID3D12DescriptorHeap> m_heap;
            engine::shared_ptr<D3D12_DESCRIPTOR_HEAP_DESC> m_desc;
            D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
            D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
            size_t m_descriptorSize;

            size_t m_numdescriptors;
            size_t m_numFrontDescriptors;
            size_t m_numBackDescriptors;
        };
    }
}
