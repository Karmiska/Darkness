#pragma once

#include "tools/ByteRange.h"
#include "tools/MemoryAllocator.h"
#include "engine/graphics/Resources.h"
#include "containers/vector.h"

namespace engine
{
    class Device;

    struct ModelResourceAllocation
    {
        void* ptr;
        size_t gpuIndex;
        size_t elements;
        tools::ByteRange range;
    };

    enum class ExtraViewType
    {
        Vertex,
        Index
    };

    struct ExtraView
    {
        ExtraViewType type;
        size_t start;
        size_t count;
    };

    class ModelResourceAllocator
    {
    public:
        explicit ModelResourceAllocator(Device& device, size_t maxElements, engine::vector<Format> formats, const char* resourceName, const engine::vector<ExtraView>& extraViews);
        explicit ModelResourceAllocator(Device& device, size_t maxElements, engine::vector<size_t> elementSize, const char* resourceName);
        explicit ModelResourceAllocator(Device& device, size_t maxElements, engine::vector<Format> formats, engine::vector<size_t> elementSize, const char* resourceName);
        engine::vector<BufferSRV>& gpuBuffers();
		engine::vector<BufferUAV>& gpuBuffersUAV();
        engine::vector<BufferIBV>& gpuIndexBuffers();
        engine::vector<BufferVBV>& gpuVertexBuffers();
		engine::vector<BufferSRVOwner>& gpuBufferOwners();
        engine::vector<BufferUAVOwner>& gpuBufferOwnersUAV();
		engine::vector<BufferIBVOwner>& gpuIndexBufferOwners();
		engine::vector<BufferVBVOwner>& gpuVertexBufferOwners();

        ModelResourceAllocation allocate(size_t elements);
        void free(ModelResourceAllocation allocation);

        size_t elements() const;
        size_t usedElements() const;
        size_t elementSizeBytes() const;
    private:
		Device& m_device;
        size_t m_maxElements;
		engine::vector<Format> m_formats;
		const char* m_resourceName;
		engine::vector<ExtraView> m_extraViews;
		engine::vector<size_t> m_elementSizes;
		int m_constructor;

        engine::vector<BufferSRVOwner> m_bufferOwners;
		engine::vector<BufferUAVOwner> m_bufferOwnersUAV;
        engine::vector<BufferVBVOwner> m_vertexBufferOwners;
        engine::vector<BufferIBVOwner> m_indexBufferOwners;
		engine::vector<BufferSRV> m_bufferHandles;
		engine::vector<BufferUAV> m_bufferHandlesUAV;
		engine::vector<BufferVBV> m_vertexBufferHandles;
		engine::vector<BufferIBV> m_indexBufferHandles;
        engine::unique_ptr<tools::MemoryAllocator> m_allocator;
        size_t m_size;
        size_t m_inUse;
        size_t m_elementSizeBytes;

		void resize(size_t newElements);
        size_t biggestFormatSize() const;
    };

    class ModelResourceLinearAllocator
    {
    public:
        struct ResourceBufferSettingsFormat
        {
            Format format = Format::UNKNOWN;
            ResourceUsage usage = ResourceUsage::GpuRead;
        };

        struct ResourceBufferSettingsStructured
        {
            size_t elementSize = 0;
            ResourceUsage usage = ResourceUsage::GpuRead;
        };

        ModelResourceLinearAllocator(
            Device& device, 
            size_t maxElements,
            engine::vector<ResourceBufferSettingsFormat> bufferSettings, 
            engine::vector<ResourceBufferSettingsStructured> structuredSettings,
            const char* resourceName);

        engine::vector<BufferSRV>& gpuBuffers();
		engine::vector<BufferSRVOwner>& gpuBufferOwners();

        ModelResourceAllocation allocate(size_t elements);
        void free(ModelResourceAllocation allocation);
        void free(void* allocation);
        void free(uintptr_t allocation);

        size_t elements() const;
        size_t usedElements() const;
        size_t elementSizeBytes() const;
    private:
        engine::vector<BufferSRVOwner> m_bufferOwners;
		engine::vector<BufferSRV> m_bufferHandles;
        uintptr_t m_currentOffset;
        size_t m_size;
        size_t m_elementSizeBytes;
    };
}
