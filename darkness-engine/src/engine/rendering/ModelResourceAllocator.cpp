#include "engine/rendering/ModelResourceAllocator.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/CommandList.h"
#include <algorithm>

namespace engine
{
	constexpr int IncreaseMemorySizeBytes = 1024 * 1024 * 1;
    ModelResourceAllocator::ModelResourceAllocator(
        Device& device,
		size_t maxElements,
        engine::vector<Format> formats,
        const char* resourceName,
        const engine::vector<ExtraView>& extraViews)
        : m_device{ device }
		, m_maxElements{ maxElements }
		, m_formats{ formats}
		, m_resourceName{ resourceName }
		, m_extraViews{ extraViews }
		, m_constructor{ 0 }
		, m_allocator{ engine::make_unique<tools::MemoryAllocator>( tools::ByteRange{ 0ull, maxElements }, 1ull ) }
        , m_size{ maxElements }
        , m_inUse{ 0ull }
        , m_elementSizeBytes{ 0ull }
    {
		if (maxElements > 0)
		{
			for (auto&& format : formats)
			{
				m_bufferOwnersUAV.emplace_back(device.createBufferUAV(BufferDescription()
					.format(format)
					.name(resourceName)
					.elements(maxElements)
					.elementSize(formatBytes(format))));
				m_elementSizeBytes += formatBytes(format);

				m_bufferOwners.emplace_back(device.createBufferSRV(m_bufferOwnersUAV.back()));
				m_bufferHandles.emplace_back(m_bufferOwners.back());
				m_bufferHandlesUAV.emplace_back(m_bufferOwnersUAV.back());
			}

			for (auto&& extraView : extraViews)
			{
				for (size_t i = extraView.start; i < extraView.start + extraView.count; ++i)
				{
					if (extraView.type == ExtraViewType::Vertex)
					{
						m_vertexBufferOwners.emplace_back(device.createBufferVBV(m_bufferOwners[i]));
						m_vertexBufferHandles.emplace_back(m_vertexBufferOwners.back());
					}
					else if (extraView.type == ExtraViewType::Index)
					{
						m_indexBufferOwners.emplace_back(device.createBufferIBV(m_bufferOwners[i]));
						m_indexBufferHandles.emplace_back(m_indexBufferOwners.back());
					}
				}
			}
		}
    }

    ModelResourceAllocator::ModelResourceAllocator(
		Device& device, 
		size_t maxElements,
		engine::vector<size_t> elementSize, 
		const char* resourceName)
		: m_device{ device }
		, m_maxElements{ maxElements }
		, m_elementSizes{ elementSize }
		, m_resourceName{ resourceName }
		, m_constructor{ 1 }
		, m_allocator{ engine::make_unique<tools::MemoryAllocator>( tools::ByteRange{ 0ull, maxElements }, 1ull ) }
        , m_size{ maxElements }
        , m_inUse{ 0ull }
        , m_elementSizeBytes{ 0ull }
    {
		if (maxElements > 0)
		{
			for (auto&& size : elementSize)
			{
				m_bufferOwnersUAV.emplace_back(device.createBufferUAV(BufferDescription()
					.usage(ResourceUsage::GpuRead)
					.name(resourceName)
					.structured(true)
					.elements(maxElements)
					.elementSize(size)));
				m_elementSizeBytes += size;

				m_bufferOwners.emplace_back(device.createBufferSRV(m_bufferOwnersUAV.back()));
				m_bufferHandles.emplace_back(m_bufferOwners.back());
				m_bufferHandlesUAV.emplace_back(m_bufferOwnersUAV.back());
			}
		}
    }

    ModelResourceAllocator::ModelResourceAllocator(
		Device& device, 
		size_t maxElements,
		engine::vector<Format> formats, 
		engine::vector<size_t> elementSize, 
		const char* resourceName)
		: m_device{ device }
		, m_maxElements{ maxElements }
		, m_formats{ formats }
		, m_elementSizes{ elementSize }
		, m_resourceName{ resourceName }
		, m_constructor{ 2 }
		, m_allocator{ engine::make_unique<tools::MemoryAllocator>( tools::ByteRange{ 0ull, maxElements }, 1ull ) }
        , m_size{ maxElements }
        , m_inUse{ 0ull }
        , m_elementSizeBytes{ 0ull }
    {
		if (maxElements > 0)
		{
			for (auto&& format : formats)
			{
				m_bufferOwnersUAV.emplace_back(device.createBufferUAV(BufferDescription()
					.usage(ResourceUsage::GpuRead)
					.format(format)
					.name(resourceName)
					.elements(maxElements)
					.elementSize(formatBytes(format))));
				m_elementSizeBytes += formatBytes(format);

				m_bufferOwners.emplace_back(device.createBufferSRV(m_bufferOwnersUAV.back()));
				m_bufferHandles.emplace_back(m_bufferOwners.back());
				m_bufferHandlesUAV.emplace_back(m_bufferOwnersUAV.back());
			}
			for (auto&& size : elementSize)
			{
				m_bufferOwnersUAV.emplace_back(device.createBufferUAV(BufferDescription()
					.usage(ResourceUsage::GpuRead)
					.name(resourceName)
					.structured(true)
					.elements(maxElements)
					.elementSize(size)));
				m_elementSizeBytes += size;

				m_bufferOwners.emplace_back(device.createBufferSRV(m_bufferOwnersUAV.back()));
				m_bufferHandles.emplace_back(m_bufferOwners.back());
				m_bufferHandlesUAV.emplace_back(m_bufferOwnersUAV.back());
			}
		}
    }

    size_t ModelResourceAllocator::elements() const
    {
        return m_size;
    }

    size_t ModelResourceAllocator::usedElements() const
    {
        return m_inUse;
    }

    size_t ModelResourceAllocator::elementSizeBytes() const
    {
        return m_elementSizeBytes;
    }

    engine::vector<BufferSRV>& ModelResourceAllocator::gpuBuffers()
    {
        return m_bufferHandles;
    }

	engine::vector<BufferUAV>& ModelResourceAllocator::gpuBuffersUAV()
	{
		return m_bufferHandlesUAV;
	}

    engine::vector<BufferIBV>& ModelResourceAllocator::gpuIndexBuffers()
    {
        return m_indexBufferHandles;
    }

    engine::vector<BufferVBV>& ModelResourceAllocator::gpuVertexBuffers()
    {
        return m_vertexBufferHandles;
    }

	engine::vector<BufferSRVOwner>& ModelResourceAllocator::gpuBufferOwners()
	{
		return m_bufferOwners;
	}

	engine::vector<BufferUAVOwner>& ModelResourceAllocator::gpuBufferOwnersUAV()
	{
		return m_bufferOwnersUAV;
	}
	
	engine::vector<BufferIBVOwner>& ModelResourceAllocator::gpuIndexBufferOwners()
	{
		return m_indexBufferOwners;
	}

	engine::vector<BufferVBVOwner>& ModelResourceAllocator::gpuVertexBufferOwners()
	{
		return m_vertexBufferOwners;
	}

    ModelResourceAllocation ModelResourceAllocator::allocate(size_t elements)
    {
        ModelResourceAllocation res;
        res.ptr = m_allocator->allocate(elements);
		if (res.ptr == reinterpret_cast<void*>(tools::MemoryAllocatorInvalidPtr))
		{
			// we ran out of memory. need to get more.
			resize(elements);
			res.ptr = m_allocator->allocate(elements);
		}
        auto offset = m_allocator->offset(res.ptr);
        res.gpuIndex = offset;
        res.elements = elements;
        res.range = tools::ByteRange(offset, offset + elements);
        m_inUse += elements;
        return res;
    }

	size_t ModelResourceAllocator::biggestFormatSize() const
	{
		size_t formatSize = 0u;
		switch (m_constructor)
		{
			case 0:
			{
				for (auto&& format : m_formats)
				{
					auto fbytes = formatBytes(format);
					if (fbytes > formatSize)
						formatSize = fbytes;
				}
				break;
			}
			case 1:
			{
				for (auto&& size : m_elementSizes)
				{
					if (size > formatSize)
						formatSize = size;
				}
				break;
			}
			case 2:
			{
				for (auto&& format : m_formats)
				{
					auto fbytes = formatBytes(format);
					if (fbytes > formatSize)
						formatSize = fbytes;
				}
				for (auto&& size : m_elementSizes)
				{
					if (size > formatSize)
						formatSize = size;
				}
				break;
			}
		}
		return formatSize;
	}

	void ModelResourceAllocator::resize(size_t _newElements)
	{
		// our biggest format size
		auto newSize = tools::gpuAllocationStrategy(biggestFormatSize(), m_size, _newElements);

		m_allocator->resize(tools::ByteRange{ 0ull, newSize });

		switch (m_constructor)
		{
			case 0:
			{
				m_maxElements = newSize;
				m_elementSizeBytes = 0;

				engine::vector<BufferSRVOwner> bufferOwners;
				engine::vector<BufferUAVOwner> bufferOwnersUAV;
				engine::vector<BufferSRV> bufferHandles;
				engine::vector<BufferUAV> bufferHandlesUAV;
				engine::vector<BufferVBVOwner> vertexBufferOwners;
				engine::vector<BufferIBVOwner> indexBufferOwners;
				engine::vector<BufferVBV> vertexBufferHandles;
				engine::vector<BufferIBV> indexBufferHandles;
				for (auto&& format : m_formats)
				{
					bufferOwnersUAV.emplace_back(m_device.createBufferUAV(BufferDescription()
						.usage(ResourceUsage::GpuRead)
						.format(format)
						.name(m_resourceName)
						.elements(m_maxElements)
						.elementSize(formatBytes(format))));
					m_elementSizeBytes += formatBytes(format);

					bufferOwners.emplace_back(m_device.createBufferSRV(bufferOwnersUAV.back()));
					bufferHandles.emplace_back(bufferOwners.back());
					bufferHandlesUAV.emplace_back(bufferOwnersUAV.back());
				}

				for (auto&& extraView : m_extraViews)
				{
					for (size_t i = extraView.start; i < extraView.start + extraView.count; ++i)
					{
						if (extraView.type == ExtraViewType::Vertex)
						{
							vertexBufferOwners.emplace_back(m_device.createBufferVBV(bufferOwners[i]));
							vertexBufferHandles.emplace_back(vertexBufferOwners.back());
						}
						else if (extraView.type == ExtraViewType::Index)
						{
							indexBufferOwners.emplace_back(m_device.createBufferIBV(bufferOwners[i]));
							indexBufferHandles.emplace_back(indexBufferOwners.back());
						}
					}
				}

				auto cmd = m_device.createCommandList("ModelResourceAllocator resize");
				for (int i = 0; i < m_bufferOwners.size(); ++i)
				{
					cmd.copyBuffer(
						m_bufferOwners[i].resource().buffer(), 
						bufferOwners[i].resource().buffer(), m_size);
				}
				m_device.submitBlocking(cmd);

				m_size = m_maxElements;
				m_bufferOwners = bufferOwners;
				m_bufferOwnersUAV = bufferOwnersUAV;
				m_bufferHandles = bufferHandles;
				m_bufferHandlesUAV = bufferHandlesUAV;
				m_vertexBufferOwners = vertexBufferOwners;
				m_indexBufferOwners = indexBufferOwners;
				m_vertexBufferHandles = vertexBufferHandles;
				m_indexBufferHandles = indexBufferHandles;

				break;
			}
			case 1:
			{
				m_maxElements = newSize;
				m_elementSizeBytes = 0;

				engine::vector<BufferSRVOwner> bufferOwners;
				engine::vector<BufferUAVOwner> bufferOwnersUAV;
				engine::vector<BufferSRV> bufferHandles;
				engine::vector<BufferUAV> bufferHandlesUAV;

				for (auto&& size : m_elementSizes)
				{
					bufferOwnersUAV.emplace_back(m_device.createBufferUAV(BufferDescription()
						.usage(ResourceUsage::GpuRead)
						.name(m_resourceName)
						.structured(true)
						.elements(m_maxElements)
						.elementSize(size)));
					m_elementSizeBytes += size;

					bufferOwners.emplace_back(m_device.createBufferSRV(bufferOwnersUAV.back()));
					bufferHandles.emplace_back(bufferOwners.back());
					bufferHandlesUAV.emplace_back(bufferOwnersUAV.back());
				}

				auto cmd = m_device.createCommandList("ModelResourceAllocator resize");
				for (int i = 0; i < m_bufferOwners.size(); ++i)
				{
					cmd.copyBuffer(
						m_bufferOwners[i].resource().buffer(),
						bufferOwners[i].resource().buffer(), m_size);
				}
				m_device.submitBlocking(cmd);

				m_size = m_maxElements;
				m_bufferOwners = bufferOwners;
				m_bufferOwnersUAV = bufferOwnersUAV;
				m_bufferHandles = bufferHandles;
				m_bufferHandlesUAV = bufferHandlesUAV;

				break;
			}
			case 2:
			{
				m_maxElements = newSize;
				m_elementSizeBytes = 0;

				engine::vector<BufferSRVOwner> bufferOwners;
				engine::vector<BufferUAVOwner> bufferOwnersUAV;
				engine::vector<BufferSRV> bufferHandles;
				engine::vector<BufferUAV> bufferHandlesUAV;
				for (auto&& format : m_formats)
				{
					bufferOwnersUAV.emplace_back(m_device.createBufferUAV(BufferDescription()
						.usage(ResourceUsage::GpuRead)
						.format(format)
						.name(m_resourceName)
						.elements(m_maxElements)
						.elementSize(formatBytes(format))));
					m_elementSizeBytes += formatBytes(format);

					bufferOwners.emplace_back(m_device.createBufferSRV(bufferOwnersUAV.back()));
					bufferHandles.emplace_back(bufferOwners.back());
					bufferHandlesUAV.emplace_back(bufferOwnersUAV.back());
				}

				for (auto&& size : m_elementSizes)
				{
					bufferOwnersUAV.emplace_back(m_device.createBufferUAV(BufferDescription()
						.usage(ResourceUsage::GpuRead)
						.name(m_resourceName)
						.structured(true)
						.elements(m_maxElements)
						.elementSize(size)));
					m_elementSizeBytes += size;

					bufferOwners.emplace_back(m_device.createBufferSRV(bufferOwnersUAV.back()));
					bufferHandles.emplace_back(bufferOwners.back());
					bufferHandlesUAV.emplace_back(bufferOwnersUAV.back());
				}

				auto cmd = m_device.createCommandList("ModelResourceAllocator resize");
				for (int i = 0; i < m_bufferOwners.size(); ++i)
				{
					cmd.copyBuffer(
						m_bufferOwners[i].resource().buffer(),
						bufferOwners[i].resource().buffer(), m_size);
				}
				m_device.submitBlocking(cmd);

				m_size = m_maxElements;
				m_bufferOwners = bufferOwners;
				m_bufferOwnersUAV = bufferOwnersUAV;
				m_bufferHandles = bufferHandles;
				m_bufferHandlesUAV = bufferHandlesUAV;
				break;
			}
		}
	}

    void ModelResourceAllocator::free(ModelResourceAllocation allocation)
    {
        m_inUse -= allocation.elements;
        m_allocator->free(allocation.ptr);
    }

    /*void ModelResourceAllocator::free(void* allocation)
    {
        m_allocator.free(allocation);
    }

    void ModelResourceAllocator::free(uint32_t allocation)
    {
        m_allocator.free(m_allocator.ptrFromOffset(allocation));
    }*/

    // ------------------------------------------------------------------

    ModelResourceLinearAllocator::ModelResourceLinearAllocator(
        Device& device, 
		size_t maxElements,
        engine::vector<ResourceBufferSettingsFormat> bufferSettings,
        engine::vector<ResourceBufferSettingsStructured> structuredSettings,
        const char* resourceName)
        : m_currentOffset{ 0ull }
        , m_size{ maxElements }
        , m_elementSizeBytes{ 0ull }
    {
        for (auto&& settings : bufferSettings)
        {
            m_bufferOwners.emplace_back(device.createBufferSRV(BufferDescription()
                .usage(settings.usage)
                .format(settings.format)
                .name(resourceName)
                .elements(maxElements)
                .elementSize(formatBytes(settings.format))));
			m_bufferHandles.emplace_back(m_bufferOwners.back());
            m_elementSizeBytes += formatBytes(settings.format);
        }
        for (auto&& structured : structuredSettings)
        {
			m_bufferOwners.emplace_back(device.createBufferSRV(BufferDescription()
                .usage(structured.usage)
                .name(resourceName)
                .structured(true)
                .elements(maxElements)
                .elementSize(structured.elementSize)));
			m_bufferHandles.emplace_back(m_bufferOwners.back());
            m_elementSizeBytes += structured.elementSize;
        }
    }

    engine::vector<BufferSRV>& ModelResourceLinearAllocator::gpuBuffers()
    {
        return m_bufferHandles;
    }

	engine::vector<BufferSRVOwner>& ModelResourceLinearAllocator::gpuBufferOwners()
	{
		return m_bufferOwners;
	}

#pragma warning( push )
#pragma warning( disable : 4312 )
#pragma warning( disable : 4311 )
#pragma warning( disable : 4302 )
    ModelResourceAllocation ModelResourceLinearAllocator::allocate(size_t elements)
    {
        ASSERT(elements == 1, "This allocator supports only element count 1");
        ModelResourceAllocation res;
        res.ptr = reinterpret_cast<void*>(m_currentOffset);
        res.gpuIndex = m_currentOffset;
        res.elements = elements;
        res.range = tools::ByteRange(m_currentOffset, m_currentOffset + elements);
        m_currentOffset += elements;
        return res;
    }

    void ModelResourceLinearAllocator::free(ModelResourceAllocation allocation)
    {
        ASSERT(reinterpret_cast<uintptr_t>(allocation.ptr) == m_currentOffset-1, "This allocator supports only freeing from end");
        m_currentOffset -= allocation.elements;
    }

    void ModelResourceLinearAllocator::free(void* allocation)
    {
        ASSERT(reinterpret_cast<uintptr_t>(allocation) == m_currentOffset-1, "This allocator supports only freeing from end");
        --m_currentOffset;
    }
#pragma warning( pop )

    void ModelResourceLinearAllocator::free(uintptr_t allocation)
    {
        ASSERT(allocation == m_currentOffset-1, "This allocator supports only freeing from end");
        --m_currentOffset;
    }

    size_t ModelResourceLinearAllocator::elements() const
    {
        return m_size;
    }

    size_t ModelResourceLinearAllocator::usedElements() const
    {
        return static_cast<size_t>(m_currentOffset);
    }

    size_t ModelResourceLinearAllocator::elementSizeBytes() const
    {
        return m_elementSizeBytes;
    }
}
