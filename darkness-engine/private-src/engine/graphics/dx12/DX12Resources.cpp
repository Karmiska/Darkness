#include "engine/graphics/dx12/DX12Resources.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "engine/graphics/dx12/DX12Device.h"
#include "engine/graphics/dx12/DX12CommandList.h"
#include "engine/graphics/dx12/DX12DescriptorHeap.h"
#include "engine/graphics/dx12/DX12Common.h"
#include "engine/graphics/Barrier.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/Common.h"

#include "tools/Debug.h"

#include <inttypes.h>

using namespace tools;

#undef PRINT_SIGNATURES_DESCRIPTORS
#undef RESOURCE_MEMORY_DEBUGGING

namespace engine
{
    namespace implementation
    {
        D3D12_HEAP_PROPERTIES getHeapPropertiesFromUsage(ResourceUsage usage)
        {
            D3D12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_DEFAULT };
            if (usage == ResourceUsage::Upload)
            {
                heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
            }
            else if (usage == ResourceUsage::GpuToCpu)
            {
                heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
            }
            else if (usage == ResourceUsage::GpuRead)
            {
                heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
            }
            else if (usage == ResourceUsage::AccelerationStructure)
            {
                heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
            }
            return heapProperties;
        }

        D3D12_RESOURCE_FLAGS flagsFromUsage(ResourceUsage usage, bool multisample = false)
        {
            switch (usage)
            {
            case ResourceUsage::Upload: return D3D12_RESOURCE_FLAG_NONE;
            case ResourceUsage::DepthStencil: return D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            case ResourceUsage::GpuRead: return D3D12_RESOURCE_FLAG_NONE;
            case ResourceUsage::GpuReadWrite: return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            case ResourceUsage::GpuRenderTargetRead: return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            case ResourceUsage::GpuRenderTargetReadWrite: 
			{
				if(multisample)
					return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
				else
					return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			}
            case ResourceUsage::GpuToCpu: return D3D12_RESOURCE_FLAG_NONE;
            case ResourceUsage::AccelerationStructure: return D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }
            return D3D12_RESOURCE_FLAG_NONE;
        }

        static uint64_t debugResourceAllocation = 0;
        static uint64_t debugBufferAllocation = 0;
        static uint64_t debugTextureAllocation = 0;
        uint64_t bytesToMB(uint64_t bytes) { return bytes / 1024u / 1024u; }
        uint64_t bytesToKB(uint64_t bytes) { return bytes / 1024u; }

#ifdef RESOURCE_MEMORY_DEBUGGING
		#undef ALLOCATION_BYTES_ONLY

		static engine::unordered_map<const char*, uint64_t> AllBufferAllocations;
		static engine::unordered_map<const char*, uint64_t> AllTextureAllocations;

		void debugMemory(const char* name, uint64_t newBytes, bool buffer)
		{
			debugResourceAllocation += newBytes;
			if (buffer)
			{
				AllBufferAllocations[name] += newBytes;
				debugBufferAllocation += newBytes;
			}
			else
			{
				AllTextureAllocations[name] += newBytes;
				debugTextureAllocation += newBytes;
			}

#ifdef ALLOCATION_BYTES_ONLY
			LOG_INFO("ADD, %" PRIu64 ", %s", newBytes, name);
#else
            if(newBytes > 1024u * 1024u)
                LOG_WARNING("[RESOURCES: %" PRIu64 " MB, BUFFERS: %" PRIu64 " MB, TEXTURES: %" PRIu64 " MB] new allocation: %" PRIu64 " MB, %s",
                    bytesToMB(debugResourceAllocation),
                    bytesToMB(debugBufferAllocation),
                    bytesToMB(debugTextureAllocation),
                    bytesToMB(newBytes),
                    name);
            else if(newBytes > 1024u)
                LOG_WARNING("[RESOURCES: %" PRIu64 " MB, BUFFERS: %" PRIu64 " MB, TEXTURES: %" PRIu64 " MB] new allocation: %" PRIu64 " KB, %s",
                    bytesToMB(debugResourceAllocation),
                    bytesToMB(debugBufferAllocation),
                    bytesToMB(debugTextureAllocation),
                    bytesToKB(newBytes),
                    name);
            else
                LOG_WARNING("[RESOURCES: %" PRIu64 " MB, BUFFERS: %" PRIu64 " MB, TEXTURES: %" PRIu64 " MB] new allocation: %" PRIu64 " Bytes, %s",
                    bytesToMB(debugResourceAllocation),
                    bytesToMB(debugBufferAllocation),
                    bytesToMB(debugTextureAllocation),
                    newBytes,
                    name);
#endif
        }

		void debugPrintCurrentStatus()
		{
			LOG_INFO("========================================================");
			LOG_INFO("     ALL ALLOCATIONS");
			LOG_INFO("========================================================");
			for (auto& all : AllBufferAllocations)
			{
				LOG_INFO("%s, %" PRIu64 "", all.first, all.second);
			}
			for (auto& all : AllTextureAllocations)
			{
				LOG_INFO("%s, %" PRIu64 "", all.first, all.second);
			}
			LOG_INFO("========================================================");
		}

        void debugMemoryRelease(const char* name, uint64_t newBytes, bool buffer)
        {
            debugResourceAllocation -= newBytes;
			if (buffer)
			{
				AllBufferAllocations[name] -= newBytes;
				if (AllBufferAllocations[name] == 0)
					AllBufferAllocations.erase(name);
				debugBufferAllocation -= newBytes;
			}
			else
			{
				AllTextureAllocations[name] -= newBytes;
				if (AllTextureAllocations[name] == 0)
					AllTextureAllocations.erase(name);
				debugTextureAllocation -= newBytes;
			}

#ifdef ALLOCATION_BYTES_ONLY
			LOG_INFO("REMOVE, %" PRIu64 ", %s", newBytes, name);
#else
            if (newBytes > 1024u * 1024u)
                LOG_WARNING("[RESOURCES: %" PRIu64 " MB, BUFFERS: %" PRIu64 " MB, TEXTURES: %" PRIu64 " MB] release allocation: %" PRIu64 " MB, %s",
                    bytesToMB(debugResourceAllocation),
                    bytesToMB(debugBufferAllocation),
                    bytesToMB(debugTextureAllocation),
                    bytesToMB(newBytes),
                    name);
            else if (newBytes > 1024u)
                LOG_WARNING("[RESOURCES: %" PRIu64 " MB, BUFFERS: %" PRIu64 " MB, TEXTURES: %" PRIu64 " MB] release allocation: %" PRIu64 " KB, %s",
                    bytesToMB(debugResourceAllocation),
                    bytesToMB(debugBufferAllocation),
                    bytesToMB(debugTextureAllocation),
                    bytesToKB(newBytes),
                    name);
            else
                LOG_WARNING("[RESOURCES: %" PRIu64 " MB, BUFFERS: %" PRIu64 " MB, TEXTURES: %" PRIu64 " MB] release allocation: %" PRIu64 " Bytes, %s",
                    bytesToMB(debugResourceAllocation),
                    bytesToMB(debugBufferAllocation),
                    bytesToMB(debugTextureAllocation),
                    newBytes,
                    name);
#endif
        }
#endif

        BufferImplDX12::BufferImplDX12(const DeviceImplDX12& device, const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_state{ ResourceState::Common }
            //, m_state{ getResourceStateFromUsage(m_description.usage) }
        {
            auto elementSize = m_description.elementSize == -1 ? formatBytes(m_description.format) : m_description.elementSize;
            m_bufferSize = m_description.elements * elementSize;

#ifdef RESOURCE_MEMORY_DEBUGGING
            debugMemory(m_description.name, m_bufferSize, true);
#endif

            ASSERT(m_description.name);
            D3D12_RESOURCE_DESC res = {};
            res.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            res.DepthOrArraySize = 1;
            res.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
            res.Flags = flagsFromUsage(m_description.usage);
            res.Format = DXGI_FORMAT_UNKNOWN;
            res.Height = 1;
            res.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            res.MipLevels = 1;
            res.SampleDesc.Count = 1;
            res.SampleDesc.Quality = 0;
            if(m_description.append)
                res.Width = roundUpToMultiple(m_bufferSize, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT) + sizeof(uint32_t);
            else
                res.Width = m_bufferSize;

#ifdef _DURANGO
			if (m_description.indirectArgument)
				res.Flags |= D3D12XBOX_RESOURCE_FLAG_ALLOW_INDIRECT_BUFFER;
#endif

            D3D12_HEAP_PROPERTIES heapProperties = getHeapPropertiesFromUsage(m_description.usage);
            
            if (heapProperties.Type == D3D12_HEAP_TYPE_UPLOAD)
            {
                m_state = ResourceState::GenericRead;
            }
            if (heapProperties.Type == D3D12_HEAP_TYPE_READBACK)
            {
                m_state = ResourceState::CopyDest;
            }
            
            D3D12_RESOURCE_STATES resState = dxResourceStates(m_state);

#ifdef DXR_BUILD
            if (m_description.usage == ResourceUsage::AccelerationStructure)
                resState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
#endif

            auto success = device.device()->CreateCommittedResource(
                &heapProperties, 
                D3D12_HEAP_FLAG_NONE,
                &res,
                resState,
                nullptr, 
                DARKNESS_IID_PPV_ARGS(m_buffer.GetAddressOf()));

            ASSERT(SUCCEEDED(success));

#ifndef RELEASE

#ifdef _UNICODE
            static WCHAR resourceName[1024] = {};
            size_t numCharacters;
            if(m_description.name)
                mbstowcs_s(&numCharacters, resourceName, m_description.name, 1024);
#else
            wchar_t resourceName[1024] = {};
            size_t numCharacters;
            mbstowcs_s(&numCharacters, resourceName, m_description.name, 1024);
#endif

            m_buffer->SetName(resourceName);
#endif

            /*if (desc.initialData)
            {
                void* data;
                auto mapres = m_buffer->Map(0, nullptr, &data);
                ASSERT(SUCCEEDED(mapres));
                memcpy(data, desc.initialData.data.data(), desc.initialData.data.size());
                m_buffer->Unmap(0, nullptr);
            }*/
        }

        BufferImplDX12::~BufferImplDX12()
        {
#ifdef RESOURCE_MEMORY_DEBUGGING
            debugMemoryRelease(m_description.name, m_bufferSize, true);
#endif
        }

        void* BufferImplDX12::map(const DeviceImplIf* /*device*/)
        {
            void* data = nullptr;
            D3D12_RANGE range;
            range.Begin = 0;
            range.End = m_bufferSize;
            auto mapres = m_buffer->Map(0, &range, &data);
            ASSERT(SUCCEEDED(mapres));
            return data;
        }

        void BufferImplDX12::unmap(const DeviceImplIf* /*device*/)
        {
            D3D12_RANGE range;
            range.Begin = 0;
            range.End = 0;
            m_buffer->Unmap(0, &range);
        }

        const BufferDescription::Descriptor& BufferImplDX12::description() const
        {
            return m_description;
        }

        ID3D12Resource* BufferImplDX12::native() const
        {
            return m_buffer.Get();
        }

        ResourceState BufferImplDX12::state() const
        {
            return m_state;
        }

        void BufferImplDX12::state(ResourceState _state)
        {
            m_state = _state;
        }

		bool BufferImplDX12::operator==(const BufferImplDX12& buff) const
		{
			return m_buffer.Get() == buff.m_buffer.Get();
		}

        BufferSRVImplDX12::BufferSRVImplDX12(
            const DeviceImplDX12& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_viewHandle{ device.heaps().cbv_srv_uav->getDescriptor() }
            , m_buffer{ buffer }
            , m_uniqueId{ m_viewHandle.uniqueId() }
        {
            auto elements = m_description.elements != -1 ? m_description.elements : buffer.description().elements;
            auto elementSize = m_description.elementSize != -1 ? m_description.elementSize : buffer.description().elementSize;

            D3D12_SHADER_RESOURCE_VIEW_DESC viewdesc;
            viewdesc.Format = desc.descriptor.structured ? DXGI_FORMAT_UNKNOWN : dxFormat(m_description.format);
            viewdesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            viewdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            viewdesc.Buffer.FirstElement = m_description.firstElement;
            viewdesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            viewdesc.Buffer.NumElements = static_cast<UINT>(elements);
            viewdesc.Buffer.StructureByteStride = desc.descriptor.structured ? static_cast<UINT>(elementSize) : 0;

            device.device()->CreateShaderResourceView(
                static_cast<BufferImplDX12*>(buffer.m_impl)->native(),
                &viewdesc,
                m_viewHandle.cpuHandle());
        }

        const BufferDescription::Descriptor& BufferSRVImplDX12::description() const
        {
            return m_description;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE& BufferSRVImplDX12::native()
        {
            return m_viewHandle.cpuHandle();
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& BufferSRVImplDX12::native() const
        {
            return m_viewHandle.cpuHandle();
        }

        Buffer BufferSRVImplDX12::buffer() const
        {
            return m_buffer;
        }

        uint64_t BufferSRVImplDX12::uniqueId() const
        {
            return m_uniqueId;
        }

        size_t BufferUAVImplDX12::structureCounterOffsetBytes() const
        {
            return roundUpToMultiple(m_description.elements * m_description.elementSize, D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT);
        }

        BufferUAVImplDX12::BufferUAVImplDX12(
            const DeviceImplDX12& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_viewHandle{ device.heaps().cbv_srv_uav->getDescriptor() }
            , m_buffer{ buffer }
            , m_uniqueId{ m_viewHandle.uniqueId() }
        {
            m_description.elements = m_description.elements != -1 ? m_description.elements : buffer.description().elements;
            m_description.elementSize = m_description.elementSize != -1 ? m_description.elementSize : static_cast<int32_t>(formatBytes(m_description.format));
            if (m_description.elementSize == -1)
                m_description.elementSize = buffer.description().elementSize;
            if (m_description.elementSize == -1)
                m_description.elementSize = static_cast<int32_t>(formatBytes(buffer.description().format));

            /*if (m_description.append)
            {
                D3D12_RESOURCE_DESC res = {};
                res.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;// D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
                res.DepthOrArraySize = 1;
                res.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
                res.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
                res.Format = dxFormat(m_description.format);
                res.Height = 1;
                res.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                res.MipLevels = 1;
                res.SampleDesc.Count = 1;
                res.SampleDesc.Quality = 0;
                res.Width = sizeof(uint32_t);

                D3D12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_READBACK };
                auto success = device.device()->CreateCommittedResource(
                    &heapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &res,
                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                    nullptr,
                    DARKNESS_IID_PPV_ARGS(m_counterBuffer.GetAddressOf()));

                ASSERT(SUCCEEDED(success));

                setCounterValue(0);
            }*/

            D3D12_UNORDERED_ACCESS_VIEW_DESC viewdesc;
            viewdesc.Format = dxFormat(m_description.format);
            viewdesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
            viewdesc.Buffer.FirstElement = m_description.firstElement;
            viewdesc.Buffer.NumElements = static_cast<UINT>(m_description.elements);
            viewdesc.Buffer.StructureByteStride = (m_description.structured || m_description.append) ? static_cast<UINT>(m_description.elementSize) : 0;
            if (m_description.append)
            {
                viewdesc.Buffer.NumElements += 1u;
                viewdesc.Buffer.CounterOffsetInBytes = structureCounterOffsetBytes();
                viewdesc.Format = DXGI_FORMAT_UNKNOWN;
            }
            else
                viewdesc.Buffer.CounterOffsetInBytes = 0;
            viewdesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

            device.device()->CreateUnorderedAccessView(
                static_cast<BufferImplDX12*>(buffer.m_impl)->native(),
                m_description.append ? static_cast<BufferImplDX12*>(buffer.m_impl)->native() : nullptr,
                //m_description.append ? m_counterBuffer.Get() : nullptr,
                &viewdesc,
                m_viewHandle.cpuHandle());
        }

        const BufferDescription::Descriptor& BufferUAVImplDX12::description() const
        {
            return m_description;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE& BufferUAVImplDX12::cpuHandle()
        {
            return m_viewHandle.cpuHandle();
        }
        const D3D12_CPU_DESCRIPTOR_HANDLE& BufferUAVImplDX12::cpuHandle() const
        {
            return m_viewHandle.cpuHandle();
        }

        D3D12_GPU_DESCRIPTOR_HANDLE& BufferUAVImplDX12::gpuHandle()
        {
            return m_viewHandle.gpuHandle();
        }

        const D3D12_GPU_DESCRIPTOR_HANDLE& BufferUAVImplDX12::gpuHandle() const
        {
            return m_viewHandle.gpuHandle();
        }

        Buffer BufferUAVImplDX12::buffer() const
        {
            return m_buffer;
        }

        uint64_t BufferUAVImplDX12::uniqueId() const
        {
            return m_uniqueId;
        }

        BufferIBVImplDX12::BufferIBVImplDX12(
            const DeviceImplDX12& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_viewHandle{ device.heaps().cbv_srv_uav->getDescriptor() }
            , m_buffer{ buffer }
        {
            auto elements = m_description.elements != -1 ? m_description.elements : buffer.description().elements;
            auto elementSize = m_description.elementSize != -1 ? m_description.elementSize : buffer.description().elementSize;

            D3D12_SHADER_RESOURCE_VIEW_DESC viewdesc;
            viewdesc.Format = dxFormat(m_description.format);
            if (viewdesc.Format == DXGI_FORMAT_UNKNOWN)
                viewdesc.Format = dxFormat(buffer.description().format);
            if (viewdesc.Format == DXGI_FORMAT_UNKNOWN)
                viewdesc.Format = m_description.elementSize == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
            viewdesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            viewdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            viewdesc.Buffer.FirstElement = m_description.firstElement;
            viewdesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            viewdesc.Buffer.NumElements = static_cast<UINT>(elements);
            viewdesc.Buffer.StructureByteStride = desc.descriptor.structured ? static_cast<UINT>(elementSize) : 0;

            m_view.BufferLocation = static_cast<BufferImplDX12*>(buffer.m_impl)->native()->GetGPUVirtualAddress();
            m_view.SizeInBytes = static_cast<UINT>(elements * elementSize);
            m_view.Format = viewdesc.Format;

            device.device()->CreateShaderResourceView(
                static_cast<BufferImplDX12*>(buffer.m_impl)->native(),
                &viewdesc,
                m_viewHandle.cpuHandle());
        }

        const BufferDescription::Descriptor& BufferIBVImplDX12::description() const
        {
            return m_description;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE& BufferIBVImplDX12::native()
        {
            return m_viewHandle.cpuHandle();
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& BufferIBVImplDX12::native() const
        {
            return m_viewHandle.cpuHandle();
        }

        Buffer BufferIBVImplDX12::buffer() const
        {
            return m_buffer;
        }

        const D3D12_INDEX_BUFFER_VIEW* BufferIBVImplDX12::view() const
        {
            return &m_view;
        }

        BufferCBVImplDX12::BufferCBVImplDX12(
            const DeviceImplDX12& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_viewHandle{ device.heaps().cbv_srv_uav->getDescriptor() }
            , m_buffer{ buffer }
        {
            auto elements = m_description.elements != -1 ? m_description.elements : buffer.description().elements;
            auto elementSize = m_description.elementSize != -1 ? m_description.elementSize : buffer.description().elementSize;

            D3D12_CONSTANT_BUFFER_VIEW_DESC view;
            view.BufferLocation = static_cast<BufferImplDX12*>(buffer.m_impl)->native()->GetGPUVirtualAddress();
            view.SizeInBytes = static_cast<UINT>(elements * elementSize);
            device.device()->CreateConstantBufferView(&view, m_viewHandle.cpuHandle());
        }

        const BufferDescription::Descriptor& BufferCBVImplDX12::description() const
        {
            return m_description;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE& BufferCBVImplDX12::native()
        {
            return m_viewHandle.cpuHandle();
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& BufferCBVImplDX12::native() const
        {
            return m_viewHandle.cpuHandle();
        }

        Buffer BufferCBVImplDX12::buffer() const
        {
            return m_buffer;
        }

        BufferVBVImplDX12::BufferVBVImplDX12(
            const DeviceImplDX12& device,
            const Buffer& buffer,
            const BufferDescription& desc)
            : m_description(desc.descriptor)
            , m_viewHandle{ device.heaps().cbv_srv_uav->getDescriptor() }
            , m_buffer{ buffer }
        {
            auto elements = m_description.elements != -1 ? m_description.elements : buffer.description().elements;
            auto elementSize = m_description.elementSize != -1 ? m_description.elementSize : buffer.description().elementSize;

            m_view.BufferLocation = static_cast<BufferImplDX12*>(buffer.m_impl)->native()->GetGPUVirtualAddress();
            m_view.SizeInBytes = static_cast<UINT>(elements * elementSize);
            m_view.StrideInBytes = desc.descriptor.structured ? static_cast<UINT>(elementSize) : static_cast<UINT>(formatBytes(desc.descriptor.format));

            /*if (strcmp(desc.descriptor.name, "Cluster Index Buffer") == 0)
            {
                m_view.StrideInBytes = 0;
            }*/

            D3D12_SHADER_RESOURCE_VIEW_DESC viewdesc;
            viewdesc.Format = dxFormat(m_description.format);

            viewdesc.Format = dxFormat(m_description.format);
            if (viewdesc.Format == DXGI_FORMAT_UNKNOWN)
                viewdesc.Format = dxFormat(buffer.description().format);
            if (viewdesc.Format == DXGI_FORMAT_UNKNOWN)
                viewdesc.Format = DXGI_FORMAT_R32_UINT;

            if (desc.descriptor.structured)
                viewdesc.Format = DXGI_FORMAT_UNKNOWN;

            viewdesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            viewdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            viewdesc.Buffer.FirstElement = m_description.firstElement;
            viewdesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            viewdesc.Buffer.NumElements = static_cast<UINT>(elements);
            viewdesc.Buffer.StructureByteStride = desc.descriptor.structured ? static_cast<UINT>(elementSize) : 0;

            device.device()->CreateShaderResourceView(
                static_cast<BufferImplDX12*>(buffer.m_impl)->native(),
                &viewdesc,
                m_viewHandle.cpuHandle());
        }

        const BufferDescription::Descriptor& BufferVBVImplDX12::description() const
        {
            return m_description;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE& BufferVBVImplDX12::native()
        {
            return m_viewHandle.cpuHandle();
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& BufferVBVImplDX12::native() const
        {
            return m_viewHandle.cpuHandle();
        }

        Buffer BufferVBVImplDX12::buffer() const
        {
            return m_buffer;
        }

        const D3D12_VERTEX_BUFFER_VIEW* BufferVBVImplDX12::view() const
        {
            return &m_view;
        }

        BindlessBufferSRVImplDX12::BindlessBufferSRVImplDX12(const DeviceImplDX12& device)
            : m_resourceId{ GlobalUniqueHandleId++ }
            , m_handle{ device.m_descriptorHeaps.shaderVisible_cbv_srv_uav->getBackDescriptor(BindlessInitialAllocationSize) }
            , m_lastDescriptorWritten{ 0 }
            , m_change{ true }
        {}

        bool BindlessBufferSRVImplDX12::operator==(const BindlessBufferSRVImplDX12& buff) const
        {
            return m_handle.cpuHandle().ptr == buff.m_handle.cpuHandle().ptr;
        }

        uint32_t BindlessBufferSRVImplDX12::push(BufferSRVOwner buffer)
        {
            auto res = m_buffers.size();
            m_buffers.emplace_back(buffer);
            m_resourceId = GlobalUniqueHandleId++;
            return static_cast<uint32_t>(res);
        }

        size_t BindlessBufferSRVImplDX12::size() const
        {
            return m_buffers.size();
        }

        BufferSRV BindlessBufferSRVImplDX12::get(size_t index)
        {
            return m_buffers[index];
        }

        uint64_t BindlessBufferSRVImplDX12::resourceId() const
        {
            return m_resourceId;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE BindlessBufferSRVImplDX12::descriptorTableGPUHandle() const
        {
            return m_handle.gpuHandle();
        }

        void BindlessBufferSRVImplDX12::updateDescriptors(DeviceImplIf* device)
        {
            auto changeCount = m_buffers.size() - m_lastDescriptorWritten;
            if (changeCount)
            {
                auto dev = static_cast<DeviceImplDX12*>(device)->device();
                D3D12_CPU_DESCRIPTOR_HANDLE resourceCpuDescriptor = m_handle.cpuHandle();
                resourceCpuDescriptor.ptr += m_lastDescriptorWritten * m_handle.handleSize();
                for (auto i = m_lastDescriptorWritten; i < m_buffers.size(); ++i)
                {
                    dev->CopyDescriptorsSimple(
                        1,
                        resourceCpuDescriptor,
                        static_cast<implementation::BufferSRVImplDX12*>(m_buffers[i].resource().m_impl)->native(),
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#ifdef PRINT_SIGNATURES_DESCRIPTORS
                    LOG("SRV. pool index: %i. writing Bindless Buffer SRV(%s): %zu to index: %i, cpu ptr: %" PRIu64 " gpu ptr: %" PRIu64 "",
                        static_cast<int>(static_cast<size_t>(resourceCpuDescriptor.ptr - device.heaps().shaderVisible_cbv_srv_uav->getCpuHeapStart().ptr) / m_handle.handleSize()),
                        m_buffers[i].resource().m_impl->description().name,
                        m_buffers[i].resource().m_impl->native().ptr,
                        static_cast<int>((m_handle.gpuHandle().ptr - device.m_descriptorHeaps.shaderVisible_cbv_srv_uav->getGpuHeapStart().ptr) / m_handle.handleSize()),
                        resourceCpuDescriptor.ptr,
                        m_handle.gpuHandle().ptr);
#endif

                    resourceCpuDescriptor.ptr += m_handle.handleSize();
                }
                m_lastDescriptorWritten = m_buffers.size();
            }
        }

        bool BindlessBufferSRVImplDX12::change() const
        {
            return m_change;
        }

        void BindlessBufferSRVImplDX12::change(bool value)
        {
            m_change = value;
        }

        BindlessBufferUAVImplDX12::BindlessBufferUAVImplDX12(const DeviceImplDX12& device)
            : m_resourceId{ GlobalUniqueHandleId++ }
            , m_handle{ device.m_descriptorHeaps.shaderVisible_cbv_srv_uav->getBackDescriptor(BindlessInitialAllocationSize) }
            , m_lastDescriptorWritten{ 0 }
            , m_change{ true }
        {}

        bool BindlessBufferUAVImplDX12::operator==(const BindlessBufferUAVImplDX12& buff) const
        {
            return m_handle.cpuHandle().ptr == buff.m_handle.cpuHandle().ptr;
        }

        uint32_t BindlessBufferUAVImplDX12::push(BufferUAVOwner buffer)
        {
            auto res = m_buffers.size();
            m_buffers.emplace_back(buffer);
            m_resourceId = GlobalUniqueHandleId++;
            return static_cast<uint32_t>(res);
        }

        size_t BindlessBufferUAVImplDX12::size() const
        {
            return m_buffers.size();
        }

        BufferUAV BindlessBufferUAVImplDX12::get(size_t index)
        {
            return m_buffers[index];
        }

        uint64_t BindlessBufferUAVImplDX12::resourceId() const
        {
            return m_resourceId;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE BindlessBufferUAVImplDX12::descriptorTableGPUHandle() const
        {
            return m_handle.gpuHandle();
        }

        void BindlessBufferUAVImplDX12::updateDescriptors(DeviceImplIf* device)
        {
            auto changeCount = m_buffers.size() - m_lastDescriptorWritten;
            if (changeCount)
            {
                auto dev = static_cast<DeviceImplDX12*>(device)->device();
                D3D12_CPU_DESCRIPTOR_HANDLE resourceCpuDescriptor = m_handle.cpuHandle();
                resourceCpuDescriptor.ptr += m_lastDescriptorWritten * m_handle.handleSize();
                for (auto i = m_lastDescriptorWritten; i < m_buffers.size(); ++i)
                {
                    dev->CopyDescriptorsSimple(
                        1,
                        resourceCpuDescriptor,
                        static_cast<BufferUAVImplDX12*>(m_buffers[i].resource().m_impl)->cpuHandle(),
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#ifdef PRINT_SIGNATURES_DESCRIPTORS
                    LOG("UAV. pool index: %i. writing Bindless Buffer UAV(%s): %zu to index: %i, cpu ptr: %" PRIu64 " gpu ptr: %" PRIu64 "",
                        static_cast<int>(static_cast<size_t>(resourceCpuDescriptor.ptr - device.heaps().shaderVisible_cbv_srv_uav->getCpuHeapStart().ptr) / m_handle.handleSize()),
                        m_buffers[i].resource().m_impl->description().name,
                        m_buffers[i].resource().m_impl->native().ptr,
                        static_cast<int>((m_handle.gpuHandle().ptr - device.m_descriptorHeaps.shaderVisible_cbv_srv_uav->getGpuHeapStart().ptr) / m_handle.handleSize()),
                        resourceCpuDescriptor.ptr,
                        m_handle.gpuHandle().ptr);
#endif

                    resourceCpuDescriptor.ptr += m_handle.handleSize();
                }
                m_lastDescriptorWritten = m_buffers.size();
            }
        }

        bool BindlessBufferUAVImplDX12::change() const
        {
            return m_change;
        }

        void BindlessBufferUAVImplDX12::change(bool value)
        {
            m_change = value;
        }

		RaytracingAccelerationStructureImplDX12::RaytracingAccelerationStructureImplDX12(
#ifdef DXR_BUILD
            const Device& device,
            BufferSRV vertexBuffer,
            BufferIBV indexBuffer,
            const BufferDescription& desc
#else
			const Device&, 
			BufferSRV,
			BufferIBV, 
			const BufferDescription& desc
#endif
        )
			: m_description(desc.descriptor)
			, m_state{ ResourceState::Common }
		{
#ifdef DXR_BUILD
			D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
			geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			geometryDesc.Triangles.IndexBuffer = indexBuffer.buffer().m_impl->native()->GetGPUVirtualAddress();
			geometryDesc.Triangles.IndexCount = indexBuffer.desc().elements;
			geometryDesc.Triangles.IndexFormat = dxFormat(indexBuffer.desc().format);
			geometryDesc.Triangles.Transform3x4 = 0;
            geometryDesc.Triangles.VertexFormat = dxFormat(vertexBuffer.desc().format);
			geometryDesc.Triangles.VertexCount = vertexBuffer.desc().elements;
			geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer.buffer().m_impl->native()->GetGPUVirtualAddress();
			geometryDesc.Triangles.VertexBuffer.StrideInBytes = vertexBuffer.desc().elementSize;

			// Mark the geometry as opaque. 
			// PERFORMANCE TIP: mark geometry as opaque whenever applicable as it can enable important ray processing optimizations.
			// Note: When rays encounter opaque geometry an any hit shader will not be executed whether it is present or not.
			geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

			// Get required sizes for an acceleration structure.
			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelBuildDesc.Inputs;
			bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			bottomLevelInputs.Flags = buildFlags;
			bottomLevelInputs.NumDescs = 1;
			bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			bottomLevelInputs.pGeometryDescs = &geometryDesc;

			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
			topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			topLevelInputs.Flags = buildFlags;
			topLevelInputs.NumDescs = 1;
			topLevelInputs.pGeometryDescs = nullptr;
			topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

			auto& deviceImpl = DeviceImplGet::impl(device);

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};
			deviceImpl.dxrDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);
			ASSERT(topLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "Failed to get DXR acceleration structure pre build info");

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
			deviceImpl.dxrDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
			ASSERT(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes > 0, "Failed to get DXR acceleration structure pre build info");

			//ComPtr<ID3D12Resource> scratchResource;
			//AllocateUAVBuffer(device, max(topLevelPrebuildInfo.ScratchDataSizeInBytes, bottomLevelPrebuildInfo.ScratchDataSizeInBytes), &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");
			m_scratch = const_cast<Device*>(&device)->createBuffer(BufferDescription()
				.elementSize(1)
				.elements(bottomLevelPrebuildInfo.ScratchDataSizeInBytes)
				.name("ScratchResource")
				.usage(ResourceUsage::GpuReadWrite));

			// Allocate resources for acceleration structures.
			// Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
			// Default heap is OK since the application doesnt need CPU read/write access to them.
			// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
			// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
			//  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
			//  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
			{
				//D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

				//AllocateUAVBuffer(device, bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_bottomLevelAccelerationStructure, initialResourceState, L"BottomLevelAccelerationStructure");
				//AllocateUAVBuffer(device, topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure, initialResourceState, L"TopLevelAccelerationStructure");

				m_bottomLevel = const_cast<Device*>(&device)->createBuffer(BufferDescription()
					.elementSize(1)
					.elements(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes)
					.name("BottomLevelAccelerationStructure")
					.usage(ResourceUsage::AccelerationStructure));

				m_topLevel = const_cast<Device*>(&device)->createBuffer(BufferDescription()
					.elementSize(1)
					.elements(topLevelPrebuildInfo.ResultDataMaxSizeInBytes)
					.name("TopLevelAccelerationStructure")
					.usage(ResourceUsage::AccelerationStructure));
			}

			// Create an instance desc for the bottom-level acceleration structure.
			D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
			instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
			instanceDesc.InstanceMask = 1;
			instanceDesc.AccelerationStructure = m_bottomLevel.resource().m_impl->native()->GetGPUVirtualAddress();

			m_instanceDesc = device.createBuffer(BufferDescription()
				.elementSize(1)
				.elements(sizeof(D3D12_RAYTRACING_INSTANCE_DESC))
				.name("DXR Acceleration structure instance desc")
				.usage(ResourceUsage::GpuRead)
				.setInitialData(BufferDescription::InitialData(
					tools::ByteRange(
						reinterpret_cast<uint8_t*>(&instanceDesc), 
						reinterpret_cast<uint8_t*>(&instanceDesc) + sizeof(D3D12_RAYTRACING_INSTANCE_DESC)), 1)));

			/*ComPtr<ID3D12Resource> instanceDescs;
			D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
			instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = 1;
			instanceDesc.InstanceMask = 1;
			instanceDesc.AccelerationStructure = m_bottomLevelAccelerationStructure->GetGPUVirtualAddress();
			AllocateUploadBuffer(device, &instanceDesc, sizeof(instanceDesc), &instanceDescs, L"InstanceDescs");*/

			// Bottom Level Acceleration Structure desc
			{
				bottomLevelBuildDesc.ScratchAccelerationStructureData = m_scratch.resource().m_impl->native()->GetGPUVirtualAddress();
				bottomLevelBuildDesc.DestAccelerationStructureData = m_bottomLevel.resource().m_impl->native()->GetGPUVirtualAddress();
			}

			// Top Level Acceleration Structure desc
			{
				topLevelBuildDesc.DestAccelerationStructureData = m_topLevel.resource().m_impl->native()->GetGPUVirtualAddress();
				topLevelBuildDesc.ScratchAccelerationStructureData = m_scratch.resource().m_impl->native()->GetGPUVirtualAddress();
				topLevelBuildDesc.Inputs.InstanceDescs = m_instanceDesc.resource().m_impl->native()->GetGPUVirtualAddress();
			}

			auto BuildAccelerationStructure = [&](auto* raytracingCommandList)
			{
				raytracingCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
				//commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_bottomLevelAccelerationStructure.Get()));
				raytracingCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
			};

			{
				auto cmd = device.createCommandList("Build DXR acceleration structure");

				// Build acceleration structure.
				BuildAccelerationStructure(CommandListImplGet::impl(cmd).dxrCommandlist());

				// Kick off acceleration structure construction.
				const_cast<Device*>(&device)->submitBlocking(cmd);
				//m_deviceResources->ExecuteCommandList();

				// Wait for GPU to finish as the locally created temporary GPU resources will get released once we go out of scope.
				//m_deviceResources->WaitForGpu();
			}
#endif
		}

        RaytracingAccelerationStructureImplDX12::~RaytracingAccelerationStructureImplDX12()
		{
			
		}

		const BufferDescription::Descriptor& RaytracingAccelerationStructureImplDX12::description() const
		{
			return m_description;
		}

		ResourceState RaytracingAccelerationStructureImplDX12::state() const
		{
			return m_state;
		}

		void RaytracingAccelerationStructureImplDX12::state(ResourceState _state)
		{
			m_state = _state;
		}

		bool RaytracingAccelerationStructureImplDX12::operator==(const RaytracingAccelerationStructureImplDX12& buff) const
		{
			return 
				(m_bottomLevel.resource() == buff.m_bottomLevel.resource()) &&
				(m_topLevel.resource() == buff.m_topLevel.resource());
		}

		uint64_t RaytracingAccelerationStructureImplDX12::resourceId() const
		{
			return 0;
		}

        typedef enum D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS {
            D3D11_STANDARD_MULTISAMPLE_PATTERN = 0xffffffff,
            D3D11_CENTER_MULTISAMPLE_PATTERN = 0xfffffffe
        } D3D11_STANDARD_MULTISAMPLE_QUALITY_LEVELS;

        TextureImplDX12::TextureImplDX12(
            const DeviceImplDX12& device,
            const TextureDescription& desc)
            : m_description(desc.descriptor)
			, m_attached{ false }
        {
            for (int slice = 0; slice < static_cast<int>(m_description.arraySlices); ++slice)
            {
                for (int mip = 0; mip < static_cast<int>(m_description.mipLevels); ++mip)
                {
                    m_state.emplace_back(ResourceState::Common);
                }
            }

#ifdef RESOURCE_MEMORY_DEBUGGING
            auto bytesAllocated = imageBytes(
                m_description.format, 
                m_description.width, 
                m_description.height, 
                (m_description.dimension == ResourceDimension::Texture3D) ? m_description.depth : m_description.arraySlices,
                m_description.mipLevels);
            debugMemory(m_description.name, bytesAllocated, false);
#endif

            ASSERT(m_description.name);
            D3D12_RESOURCE_DESC res = {};
            res.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            if(m_description.dimension == ResourceDimension::Texture3D)
                res.DepthOrArraySize = static_cast<UINT16>(m_description.depth);
            else
                res.DepthOrArraySize = static_cast<UINT16>(m_description.arraySlices);
            res.Dimension = dxResourceDimension(m_description.dimension);
            res.Flags = flagsFromUsage(desc.descriptor.usage, desc.descriptor.samples > 1 ? true : false);
            res.Format = dxTypelessFormat(m_description.format);
            
            if (m_description.format == Format::D32_FLOAT)
                res.Format = DXGI_FORMAT_R32_TYPELESS;

			res.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			if((m_description.usage == ResourceUsage::GpuToCpu) ||
				(m_description.usage == ResourceUsage::Upload))
				res.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            if (m_description.dimension == ResourceDimension::TextureCube)
            {
                ASSERT(m_description.width <= D3D12_REQ_TEXTURECUBE_DIMENSION, "Too big texture for cubemap");
                ASSERT(m_description.height <= D3D12_REQ_TEXTURECUBE_DIMENSION, "Too big texture for cubemap");
                ASSERT(res.DepthOrArraySize <= D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION, "Too many array slices for cubemap");
            }
            
            res.SampleDesc.Count = static_cast<UINT>(m_description.samples);
            res.SampleDesc.Quality = m_description.samples > 1 ? D3D11_STANDARD_MULTISAMPLE_PATTERN : 0;
            res.Width = m_description.width;
            res.Height = static_cast<UINT>(m_description.height);
            res.MipLevels = static_cast<UINT16>(m_description.mipLevels);

            D3D12_HEAP_PROPERTIES heapProperties = {};// = getHeapPropertiesFromUsage(m_description.usage);
            heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
            heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heapProperties.CreationNodeMask = 0;
            heapProperties.VisibleNodeMask = 0;

            D3D12_RESOURCE_STATES resState = dxResourceStates(ResourceState::Common);

            D3D12_CLEAR_VALUE clearValue = {};
            if ((m_description.usage == ResourceUsage::GpuRenderTargetRead) ||
                (m_description.usage == ResourceUsage::GpuRenderTargetReadWrite))
            {
                clearValue.Color[0] = m_description.optimizedClearValue.x;
                clearValue.Color[1] = m_description.optimizedClearValue.y;
                clearValue.Color[2] = m_description.optimizedClearValue.z;
                clearValue.Color[3] = m_description.optimizedClearValue.w;
                clearValue.Format = dxFormat(m_description.format);
            }
            else if (m_description.usage == ResourceUsage::DepthStencil)
            {
                clearValue.DepthStencil.Depth = m_description.optimizedDepthClearValue;
                clearValue.DepthStencil.Stencil = m_description.optimizedStencilClearValue;
                if (m_description.format == Format::R32_FLOAT)
                    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
                else if(m_description.format == Format::R16_FLOAT)
                    clearValue.Format = DXGI_FORMAT_D16_UNORM;
                else
                    clearValue.Format = dxFormat(m_description.format);
            }
            else
                clearValue.Format = dxFormat(m_description.format);

            bool setClearValue =
                ((res.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) == D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) ||
                ((res.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

            auto success = device.device()->CreateCommittedResource(
                &heapProperties,
                desc.descriptor.shared ? D3D12_HEAP_FLAG_SHARED : D3D12_HEAP_FLAG_NONE,
                &res,
                resState,
                setClearValue ? &clearValue : nullptr,
                DARKNESS_IID_PPV_ARGS(m_texture.GetAddressOf()));

            ASSERT(SUCCEEDED(success));

#ifndef RELEASE

#ifdef _UNICODE
            static WCHAR resourceName[1024] = {};
            size_t numCharacters;
            if(m_description.name)
                mbstowcs_s(&numCharacters, resourceName, m_description.name, 1024);
#else
            wchar_t resourceName[1024] = {};
            size_t numCharacters;
            mbstowcs_s(&numCharacters, resourceName, m_description.name, 1024);
#endif

            m_texture->SetName(resourceName);
#endif

            if (desc.initialData)
            {
                /*void* data;
                D3D12_RANGE range;
                range.Begin = desc.initialData.elementStart * desc.initialData.elementSize;
                range.End = desc.initialData.data.size();
                auto mapres = m_buffer->Map(0, &range, &data);
                ASSERT(SUCCEEDED(mapres));
                memcpy(data, desc.initialData.data.data(), desc.initialData.data.size());
                m_buffer->Unmap(0, &range);*/
            }
        }

        TextureImplDX12::~TextureImplDX12()
        {
#ifdef RESOURCE_MEMORY_DEBUGGING
			if (!m_attached)
			{
				auto bytesAllocated = imageBytes(
					m_description.format,
					m_description.width,
					m_description.height,
					(m_description.dimension == ResourceDimension::Texture3D) ? m_description.depth : m_description.arraySlices,
					m_description.mipLevels);
				debugMemoryRelease(m_description.name, bytesAllocated, false);
			}
#endif
        }

        TextureImplDX12::TextureImplDX12(
            const DeviceImplDX12& /*device*/,
            const TextureDescription& desc,
            ID3D12Resource* resource,
            ResourceState currentState)
            : m_description(desc.descriptor)
            , m_state{ currentState }
			, m_attached{ true }
        {
            m_texture.Attach(resource);
        }

        void* TextureImplDX12::map(const DeviceImplIf* /*device*/)
        {
            void* data;
            D3D12_RANGE range;
            range.Begin = 0;
            range.End = 0;
            auto mapres = m_texture->Map(0, &range, &data);
            ASSERT(SUCCEEDED(mapres));
            return data;
        }

        void TextureImplDX12::unmap(const DeviceImplIf* /*device*/)
        {
            D3D12_RANGE range;
            range.Begin = 0;
            range.End = 0;
            m_texture->Unmap(0, nullptr);
        }

        const TextureDescription::Descriptor& TextureImplDX12::description() const
        {
            return m_description;
        }

        ID3D12Resource* TextureImplDX12::native() const
        {
            return m_texture.Get();
        }

        ResourceState TextureImplDX12::state(int slice, int mip) const
        {
            return m_state[mip + (slice * m_description.mipLevels)];
        }

        void TextureImplDX12::state(int slice, int mip, ResourceState state)
        {
            m_state[mip + (slice * m_description.mipLevels)] = state;
        }

		bool TextureImplDX12::operator==(const TextureImplDX12& tex) const
		{
			return m_texture.Get() == tex.m_texture.Get();
		}

        TextureSRVImplDX12::TextureSRVImplDX12(
            const DeviceImplDX12& device,
            const Texture& texture,
            const TextureDescription& desc,
            SubResource subResources)
            : m_description(desc.descriptor)
            , m_viewHandle{ device.heaps().cbv_srv_uav->getDescriptor() }
            , m_texture{ texture }
            , m_subResources{ subResources }
            , m_uniqueId{ m_viewHandle.uniqueId() }
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC viewdesc;
            viewdesc.Format = dxFormat(m_description.format);

            // D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(2,1,0,3);
            viewdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            switch (m_description.dimension)
            {
                case ResourceDimension::Texture1D:
                {
                    viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
                    viewdesc.Texture1D.MipLevels = static_cast<UINT>(subResources.mipCount != AllMipLevels ? subResources.mipCount : texture.mipLevels());
                    viewdesc.Texture1D.MostDetailedMip = static_cast<UINT>(subResources.firstMipLevel);
                    viewdesc.Texture1D.ResourceMinLODClamp = 0.0f;
                    break;
                }
                case ResourceDimension::Texture1DArray:
                {
                    viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
                    viewdesc.Texture1DArray.MipLevels = static_cast<UINT>(subResources.mipCount != AllMipLevels ? subResources.mipCount : texture.mipLevels());
                    viewdesc.Texture1DArray.MostDetailedMip = static_cast<UINT>(subResources.firstMipLevel);
                    viewdesc.Texture1DArray.ResourceMinLODClamp = 0.0f;
                    viewdesc.Texture1DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                    viewdesc.Texture1DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
                    break;
                }
                case ResourceDimension::Texture2D:
                {
                    if (m_description.samples > 1)
                    {
                        viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                    }
                    else
                    {
                        viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                        viewdesc.Texture2D.MipLevels = static_cast<UINT>(subResources.mipCount != AllMipLevels ? subResources.mipCount : texture.mipLevels());
                        viewdesc.Texture2D.MostDetailedMip = static_cast<UINT>(subResources.firstMipLevel);
                        viewdesc.Texture2D.ResourceMinLODClamp = 0.0f;
                        viewdesc.Texture2D.PlaneSlice = 0u;
                    }
                    break;
                }
                case ResourceDimension::Texture2DArray:
                {
                    if (m_description.samples > 1)
                    {
                        viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                        viewdesc.Texture2DMSArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                        viewdesc.Texture2DMSArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
                    }
                    else
                    {
                        viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                        viewdesc.Texture2DArray.MipLevels = static_cast<UINT>(subResources.mipCount != AllMipLevels ? subResources.mipCount : texture.mipLevels());
                        viewdesc.Texture2DArray.MostDetailedMip = static_cast<UINT>(subResources.firstMipLevel);
                        viewdesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
                        viewdesc.Texture2DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                        viewdesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
                        viewdesc.Texture2DArray.PlaneSlice = 0;
                    }
                    break;
                }
                case ResourceDimension::Texture3D:
                {
                    viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
                    viewdesc.Texture3D.MipLevels = static_cast<UINT>(subResources.mipCount != AllMipLevels ? subResources.mipCount : texture.mipLevels());
                    viewdesc.Texture3D.MostDetailedMip = static_cast<UINT>(subResources.firstMipLevel);
                    viewdesc.Texture3D.ResourceMinLODClamp = 0.0f;
                    break;
                }
                case ResourceDimension::TextureCube:
                {
                    viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
                    viewdesc.TextureCube.MipLevels = static_cast<UINT>(subResources.mipCount != AllMipLevels ? subResources.mipCount : texture.mipLevels());
                    viewdesc.TextureCube.MostDetailedMip = static_cast<UINT>(subResources.firstMipLevel);
                    viewdesc.TextureCube.ResourceMinLODClamp = 0.0f;
                    break;
                }
                case ResourceDimension::TextureCubeArray:
                {
                    viewdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                    viewdesc.TextureCubeArray.MipLevels = static_cast<UINT>(subResources.mipCount != AllMipLevels ? subResources.mipCount : texture.mipLevels());
                    viewdesc.TextureCubeArray.MostDetailedMip = static_cast<UINT>(subResources.firstMipLevel);
                    viewdesc.TextureCubeArray.ResourceMinLODClamp = 0.0f;
                    viewdesc.TextureCubeArray.First2DArrayFace = static_cast<UINT>(subResources.firstArraySlice);
                    viewdesc.TextureCubeArray.NumCubes = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                    break;
                }
            }

            device.device()->CreateShaderResourceView(
                static_cast<TextureImplDX12*>(texture.m_impl)->native(),
                &viewdesc,
                m_viewHandle.cpuHandle());
        }

        const TextureDescription::Descriptor& TextureSRVImplDX12::description() const
        {
            return m_description;
        }

        Texture TextureSRVImplDX12::texture() const
        {
            return m_texture;
        }

        Format TextureSRVImplDX12::format() const
        {
            return m_description.format;
        }

        size_t TextureSRVImplDX12::width() const
        {
            return m_description.width;
        }

        size_t TextureSRVImplDX12::height() const
        {
            return m_description.height;
        }

        size_t TextureSRVImplDX12::depth() const
        {
            return m_description.depth;
        }

		ResourceDimension TextureSRVImplDX12::dimension() const
		{
			return m_description.dimension;
		}

        D3D12_CPU_DESCRIPTOR_HANDLE& TextureSRVImplDX12::native()
        {
            return m_viewHandle.cpuHandle();
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& TextureSRVImplDX12::native() const
        {
            return m_viewHandle.cpuHandle();
        }

        uint64_t TextureSRVImplDX12::uniqueId() const
        {
            return m_uniqueId;
        }

        const SubResource& TextureSRVImplDX12::subResource() const
        {
            return m_subResources;
        }

        TextureUAVImplDX12::TextureUAVImplDX12(
            const DeviceImplDX12& device,
            const Texture& texture,
            const TextureDescription& desc,
            SubResource subResources)
            : m_description(desc.descriptor)
            , m_viewHandle{ device.heaps().cbv_srv_uav->getDescriptor() }
            , m_texture{ texture }
            , m_subResources{ subResources }
            , m_uniqueId{ m_viewHandle.uniqueId() }
        {
            if (m_description.append)
            {
                D3D12_RESOURCE_DESC res = {};
                res.Alignment = D3D12_UAV_COUNTER_PLACEMENT_ALIGNMENT;
                if (m_description.dimension == ResourceDimension::Texture3D)
                    res.DepthOrArraySize = static_cast<UINT16>(m_description.depth);
                else
                    res.DepthOrArraySize = static_cast<UINT16>(m_description.arraySlices);
                res.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
                res.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                    D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL |
                    D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS |
                    D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER |
                    D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
                res.Format = dxFormat(m_description.format);
                res.Height = 1;
                res.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                res.MipLevels = 1;
                res.SampleDesc.Count = 1;
                res.SampleDesc.Quality = 0;
                res.Width = sizeof(uint32_t);

                D3D12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_UPLOAD };
                auto success = device.device()->CreateCommittedResource(
                    &heapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &res,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    DARKNESS_IID_PPV_ARGS(m_counterBuffer.GetAddressOf()));

                ASSERT(SUCCEEDED(success));

                //setCounterValue(0);
            }

            D3D12_UNORDERED_ACCESS_VIEW_DESC viewdesc;
            viewdesc.Format = dxFormat(m_description.format);
            switch (m_description.dimension)
            {
				case ResourceDimension::Texture1D:
				{
					viewdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
					viewdesc.Texture1D.MipSlice = subResources.firstMipLevel;
					break;
				}
				case ResourceDimension::Texture1DArray:
				{
					viewdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
					viewdesc.Texture1DArray.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
					viewdesc.Texture1DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
					viewdesc.Texture1DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
					break;
				}
				case ResourceDimension::Texture2D:
				{
					viewdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					viewdesc.Texture2D.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
					viewdesc.Texture2D.PlaneSlice = 0;
					break;
				}
				case ResourceDimension::Texture2DArray:
				{
					viewdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					viewdesc.Texture2DArray.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
					viewdesc.Texture2DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
					viewdesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
					viewdesc.Texture2DArray.PlaneSlice = 0;
					break;
				}
				case ResourceDimension::Texture3D:
				{
					viewdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
					viewdesc.Texture3D.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
					viewdesc.Texture3D.FirstWSlice = static_cast<UINT>(subResources.firstArraySlice);
                    viewdesc.Texture3D.WSize = static_cast<UINT>(m_description.depth);
					break;
				}
				case ResourceDimension::TextureCube:
				{
					viewdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					viewdesc.Texture2D.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
					viewdesc.Texture2D.PlaneSlice = 0;
					break;
				}
				case ResourceDimension::TextureCubeArray:
				{
					viewdesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					viewdesc.Texture2DArray.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
					viewdesc.Texture2DArray.PlaneSlice = 0;
					viewdesc.Texture2DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
					viewdesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
					break;
				}
            }

            device.device()->CreateUnorderedAccessView(
                static_cast<TextureImplDX12*>(texture.m_impl)->native(),
                m_description.append ? m_counterBuffer.Get() : nullptr,
                &viewdesc,
                m_viewHandle.cpuHandle());
        }

        const TextureDescription::Descriptor& TextureUAVImplDX12::description() const
        {
            return m_description;
        }

        /*void TextureUAVImpl::setCounterValue(uint32_t value)
        {
            uint8_t* counterData = nullptr;
            D3D12_RANGE counterRange = { 0, 0 };
            m_counterBuffer->Map(0, &counterRange, reinterpret_cast<void**>(&counterData));
            memcpy(counterData, &value, sizeof(uint32_t));
            m_counterBuffer->Unmap(0, nullptr);
        }

        uint32_t TextureUAVImpl::getCounterValue()
        {
            uint32_t res = 0;
            uint8_t* counterData = nullptr;
            D3D12_RANGE counterRange = { 0, 0 };
            m_counterBuffer->Map(0, &counterRange, reinterpret_cast<void**>(&counterData));
            memcpy(&res, counterData, sizeof(uint32_t));
            m_counterBuffer->Unmap(0, nullptr);
            return res;
        }*/

        Texture TextureUAVImplDX12::texture() const
        {
            return m_texture;
        }

        Format TextureUAVImplDX12::format() const
        {
            return m_description.format;
        }

        size_t TextureUAVImplDX12::width() const
        {
            return m_description.width;
        }

        size_t TextureUAVImplDX12::height() const
        {
            return m_description.height;
        }

        size_t TextureUAVImplDX12::depth() const
        {
            return m_description.depth;
        }

		ResourceDimension TextureUAVImplDX12::dimension() const
		{
			return m_description.dimension;
		}

        D3D12_CPU_DESCRIPTOR_HANDLE& TextureUAVImplDX12::native()
        {
            return m_viewHandle.cpuHandle();
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& TextureUAVImplDX12::native() const
        {
            return m_viewHandle.cpuHandle();
        }

        uint64_t TextureUAVImplDX12::uniqueId() const
        {
            return m_uniqueId;
        }

        const SubResource& TextureUAVImplDX12::subResource() const
        {
            return m_subResources;
        }

        TextureDSVImplDX12::TextureDSVImplDX12(
            const DeviceImplDX12& device,
            const Texture& texture,
            const TextureDescription& desc,
            SubResource subResources)
            : m_description(desc.descriptor)
            , m_viewHandle{ device.heaps().dsv->getDescriptor() }
            , m_texture{ texture }
            , m_subResources{ subResources }
        {
            D3D12_DEPTH_STENCIL_VIEW_DESC viewdesc;
            viewdesc.Format = dxFormat(m_description.format);
            viewdesc.Flags = D3D12_DSV_FLAG_NONE;
            switch (m_description.dimension)
            {
            case ResourceDimension::Texture1D:
            {
                viewdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
                viewdesc.Texture1D.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
                break;
            }
            case ResourceDimension::Texture1DArray:
            {
                viewdesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
                viewdesc.Texture1DArray.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
                viewdesc.Texture1DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
                viewdesc.Texture1DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                break;
            }
            case ResourceDimension::Texture2D:
            {
                viewdesc.ViewDimension = m_description.samples > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMS : D3D12_DSV_DIMENSION_TEXTURE2D;
                viewdesc.Texture2D.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
                break;
            }
            case ResourceDimension::Texture2DArray:
            {
                viewdesc.ViewDimension = m_description.samples > 1 ? D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
                viewdesc.Texture2DArray.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
                viewdesc.Texture2DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                viewdesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
                break;
            }
            }

            device.device()->CreateDepthStencilView(
                static_cast<TextureImplDX12*>(texture.m_impl)->native(),
                &viewdesc,
                m_viewHandle.cpuHandle());
        }

        const TextureDescription::Descriptor& TextureDSVImplDX12::description() const
        {
            return m_description;
        }

        Texture TextureDSVImplDX12::texture() const
        {
            return m_texture;
        }

        Format TextureDSVImplDX12::format() const
        {
            return m_description.format;
        }

        size_t TextureDSVImplDX12::width() const
        {
            return m_description.width;
        }

        size_t TextureDSVImplDX12::height() const
        {
            return m_description.height;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE& TextureDSVImplDX12::native()
        {
            return m_viewHandle.cpuHandle();
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& TextureDSVImplDX12::native() const
        {
            return m_viewHandle.cpuHandle();
        }

        const SubResource& TextureDSVImplDX12::subResource() const
        {
            return m_subResources;
        }

        TextureRTVImplDX12::TextureRTVImplDX12(
            const DeviceImplDX12& device,
            const Texture& texture,
            const TextureDescription& desc,
            SubResource subResources)
            : m_description(desc.descriptor)
            , m_viewHandle{ device.heaps().rtv->getDescriptor() }
            , m_texture{ texture }
            , m_subResources{ subResources }
        {
            D3D12_RENDER_TARGET_VIEW_DESC viewdesc;
            viewdesc.Format = dxFormat(m_description.format);

            m_description.width >>= subResources.firstMipLevel;
            m_description.height >>= subResources.firstMipLevel;
            if (m_description.width < 1)
                m_description.width = 1;
            if (m_description.height < 1)
                m_description.height = 1;

            if(subResources.mipCount != AllMipLevels)
                m_description.mipLevels = subResources.mipCount;

            switch (m_description.dimension)
            {
            case ResourceDimension::Texture1D:
            {
                viewdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
                viewdesc.Texture1D.MipSlice = subResources.firstMipLevel;
                break;
            }
            case ResourceDimension::Texture1DArray:
            {
                viewdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
                viewdesc.Texture1DArray.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
                viewdesc.Texture1DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
                viewdesc.Texture1DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                break;
            }
            case ResourceDimension::Texture2D:
            {
                viewdesc.ViewDimension = m_description.samples > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMS : D3D12_RTV_DIMENSION_TEXTURE2D;
                viewdesc.Texture2D.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
                viewdesc.Texture2D.PlaneSlice = 0;
                break;
            }
            case ResourceDimension::Texture2DArray:
            {
                viewdesc.ViewDimension = m_description.samples > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                viewdesc.Texture2DArray.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
                viewdesc.Texture2DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                viewdesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
                viewdesc.Texture2DArray.PlaneSlice = 0;
                break;
            }
            case ResourceDimension::Texture3D:
            {
                viewdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
                viewdesc.Texture3D.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
                viewdesc.Texture3D.FirstWSlice = static_cast<UINT>(subResources.firstArraySlice);
                viewdesc.Texture3D.WSize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                break;
            }
            case ResourceDimension::TextureCube:
            {
                viewdesc.ViewDimension = m_description.samples > 1 ? D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
                viewdesc.Texture2DArray.MipSlice = static_cast<UINT>(subResources.firstMipLevel);
                viewdesc.Texture2DArray.ArraySize = static_cast<UINT>(subResources.arraySliceCount != AllArraySlices ? subResources.arraySliceCount : texture.arraySlices());
                viewdesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(subResources.firstArraySlice);
                viewdesc.Texture2DArray.PlaneSlice = 0;
                break;
            }
            case ResourceDimension::TextureCubeArray:
            {
                ASSERT(false, "Not implemented");
                break;
            }
            }

            device.device()->CreateRenderTargetView(
                static_cast<TextureImplDX12*>(texture.m_impl)->native(),
                &viewdesc,
                m_viewHandle.cpuHandle());
        }

        const TextureDescription::Descriptor& TextureRTVImplDX12::description() const
        {
            return m_description;
        }

        Texture TextureRTVImplDX12::texture() const
        {
            return m_texture;
        }

        Format TextureRTVImplDX12::format() const
        {
            return m_description.format;
        }

        size_t TextureRTVImplDX12::width() const
        {
            return m_description.width;
        }

        size_t TextureRTVImplDX12::height() const
        {
            return m_description.height;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE& TextureRTVImplDX12::native()
        {
            return m_viewHandle.cpuHandle();
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& TextureRTVImplDX12::native() const
        {
            return m_viewHandle.cpuHandle();
        }

        const SubResource& TextureRTVImplDX12::subResource() const
        {
            return m_subResources;
        }

        BindlessTextureSRVImplDX12::BindlessTextureSRVImplDX12(const DeviceImplDX12& device)
            : m_resourceId{ GlobalUniqueHandleId++ }
            , m_handle{ device.m_descriptorHeaps.shaderVisible_cbv_srv_uav->getBackDescriptor(BindlessInitialAllocationSize) }
            , m_lastDescriptorWritten{ 0 }
            , m_change{ true }
        {
        }

        bool BindlessTextureSRVImplDX12::operator==(const BindlessTextureSRVImplDX12& tex) const
        {
            return m_handle.cpuHandle().ptr == tex.m_handle.cpuHandle().ptr;
        }

        uint32_t BindlessTextureSRVImplDX12::push(TextureSRVOwner texture)
        {
            for(int i = 0; i < m_textures.size(); ++i)
            {
                if (m_textures[i].resource().resourceId() == texture.resource().resourceId())
                {
                    return i;
                }
            }
            
            auto res = m_textures.size();
            m_textures.emplace_back(texture);
            m_resourceId = GlobalUniqueHandleId++;
            return static_cast<uint32_t>(res);
        }

        size_t BindlessTextureSRVImplDX12::size() const
        {
            return m_textures.size();
        }

        TextureSRV BindlessTextureSRVImplDX12::get(size_t index)
        {
            return m_textures[index];
        }

        uint64_t BindlessTextureSRVImplDX12::resourceId() const
        {
            return m_resourceId;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE BindlessTextureSRVImplDX12::descriptorTableGPUHandle() const
        {
            return m_handle.gpuHandle();
        }

        void BindlessTextureSRVImplDX12::updateDescriptors(DeviceImplIf* device)
        {
            auto changeCount = m_textures.size() - m_lastDescriptorWritten;
            if (changeCount)
            {
                m_change = true;
                auto dev = static_cast<DeviceImplDX12*>(device)->device();
                D3D12_CPU_DESCRIPTOR_HANDLE resourceCpuDescriptor = m_handle.cpuHandle();
                resourceCpuDescriptor.ptr += m_lastDescriptorWritten * m_handle.handleSize();
                for (auto i = m_lastDescriptorWritten; i < m_textures.size(); ++i)
                {
                    dev->CopyDescriptorsSimple(
                        1,
                        resourceCpuDescriptor,
                        static_cast<TextureSRVImplDX12*>(m_textures[i].resource().m_impl)->native(),
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#ifdef PRINT_SIGNATURES_DESCRIPTORS
                    LOG("SRV. pool index: %i. writing Bindless Texture SRV(%s): %zu to index: %i, cpu ptr: %" PRIu64 " gpu ptr: %" PRIu64 "",
                        static_cast<int>(static_cast<size_t>(resourceCpuDescriptor.ptr - device.heaps().shaderVisible_cbv_srv_uav->getCpuHeapStart().ptr) / m_handle.handleSize()),
                        m_textures[i].resource().m_impl->description().name,
                        m_textures[i].resource().m_impl->native().ptr,
                        static_cast<int>((m_handle.gpuHandle().ptr - device.m_descriptorHeaps.shaderVisible_cbv_srv_uav->getGpuHeapStart().ptr) / m_handle.handleSize()),
                        resourceCpuDescriptor.ptr,
                        m_handle.gpuHandle().ptr);
#endif

                    resourceCpuDescriptor.ptr += m_handle.handleSize();
                }
                m_lastDescriptorWritten = m_textures.size();
            }
        }

        bool BindlessTextureSRVImplDX12::change() const
        {
            return m_change;
        }

        void BindlessTextureSRVImplDX12::change(bool value)
        {
            m_change = value;
        }

        BindlessTextureUAVImplDX12::BindlessTextureUAVImplDX12(const DeviceImplDX12& device)
            : m_resourceId{ GlobalUniqueHandleId++ }
            , m_handle{ device.m_descriptorHeaps.shaderVisible_cbv_srv_uav->getBackDescriptor(BindlessInitialAllocationSize) }
            , m_lastDescriptorWritten{ 0 }
            , m_change{ true }
        {
        }

        bool BindlessTextureUAVImplDX12::operator==(const BindlessTextureUAVImplDX12& tex) const
        {
            return m_handle.cpuHandle().ptr == tex.m_handle.cpuHandle().ptr;
        }

        uint32_t BindlessTextureUAVImplDX12::push(TextureUAVOwner texture)
        {
            auto res = m_textures.size();
            m_textures.emplace_back(texture);
            m_resourceId = GlobalUniqueHandleId++;
            return static_cast<uint32_t>(res);
        }

        size_t BindlessTextureUAVImplDX12::size() const
        {
            return m_textures.size();
        }

        TextureUAV BindlessTextureUAVImplDX12::get(size_t index)
        {
            return m_textures[index];
        }

        uint64_t BindlessTextureUAVImplDX12::resourceId() const
        {
            return m_resourceId;
        }

        D3D12_GPU_DESCRIPTOR_HANDLE BindlessTextureUAVImplDX12::descriptorTableGPUHandle() const
        {
            return m_handle.gpuHandle();
        }

        void BindlessTextureUAVImplDX12::updateDescriptors(DeviceImplIf* device)
        {
            auto changeCount = m_textures.size() - m_lastDescriptorWritten;
            if (changeCount)
            {
                m_change = true;
                auto dev = static_cast<DeviceImplDX12*>(device)->device();
                D3D12_CPU_DESCRIPTOR_HANDLE resourceCpuDescriptor = m_handle.cpuHandle();
                resourceCpuDescriptor.ptr += m_lastDescriptorWritten * m_handle.handleSize();
                for (auto i = m_lastDescriptorWritten; i < m_textures.size(); ++i)
                {
                    dev->CopyDescriptorsSimple(
                        1,
                        resourceCpuDescriptor,
                        static_cast<TextureUAVImplDX12*>(m_textures[i].resource().m_impl)->native(),
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

#ifdef PRINT_SIGNATURES_DESCRIPTORS
                    LOG("UAV. pool index: %i. writing Bindless Texture UAV(%s): %zu to index: %i, cpu ptr: %" PRIu64 " gpu ptr: %" PRIu64 "",
                        static_cast<int>(static_cast<size_t>(resourceCpuDescriptor.ptr - device.heaps().shaderVisible_cbv_srv_uav->getCpuHeapStart().ptr) / m_handle.handleSize()),
                        m_textures[i].resource().m_impl->description().name,
                        m_textures[i].resource().m_impl->native().ptr,
                        static_cast<int>((m_handle.gpuHandle().ptr - device.m_descriptorHeaps.shaderVisible_cbv_srv_uav->getGpuHeapStart().ptr) / m_handle.handleSize()),
                        resourceCpuDescriptor.ptr,
                        m_handle.gpuHandle().ptr);
#endif

                    resourceCpuDescriptor.ptr += m_handle.handleSize();
                }
                m_lastDescriptorWritten = m_textures.size();
            }
        }

        bool BindlessTextureUAVImplDX12::change() const
        {
            return m_change;
        }

        void BindlessTextureUAVImplDX12::change(bool value)
        {
            m_change = value;
        }
    }
}
