#include "engine/rendering/ResidencyManager.h"
#include "engine/graphics/Device.h"
#include "engine/graphics/Queue.h"
#include "engine/graphics/Fence.h"
#include "engine/graphics/CommandList.h"
#include "engine/graphics/dx12/DX12Headers.h"
#include "tools/ByteRange.h"

#include <thread>
#include <chrono>
#include <future>
#include <algorithm>

namespace engine
{
    MakeResidentTask::MakeResidentTask(Device& device, Buffer uploadBuffer, UploadAllocation resource)
    {
        // eventually this will be threaded
        auto cmd = device.createCommandList("MakeResidentTask", CommandListType::Copy);

        cmd.copyBuffer(
            uploadBuffer, 
            resource.buffer.valid() ? resource.buffer.buffer() : resource.indexBuffer.buffer(),
            resource.bytes,
            resource.srcElement,
            resource.gpuIndex);

        device.submitBlocking(cmd, CommandListType::Copy);
    }

    ResidencyManager::ResidencyManager(Device& device)
        : m_device{ &device }
        , m_uploadBuffer{ device.createBuffer(BufferDescription()
            .elementSize(1)
            .elements(ResidencyUploadBufferSizeBytes)
            .usage(ResourceUsage::Upload)
            .name("ResidencyManager UploadBuffer")) }
        , m_relocationBuffer{ device.createBuffer(BufferDescription()
            .elementSize(1)
            .elements(ResidencyUploadBufferSizeBytes)
            .name("ResidencyManager UploadBuffer")) }

        , m_uploadMemory{ static_cast<uint8_t*>(m_uploadBuffer.resource().map(device)) }
        , m_memoryInUse{ 0u }
    {}

    // TODO: ALL OF THIS MEMORY MANAGEMENT IS DODGY AT BEST
    //       IT MAKES A LOT OF ASSUMPTIONS ON THE WAY IT'S CALLED
    //       AND FREED. SHOULD REPLACE WITH REAL MEMORY MANAGEMENT!!
    UploadAllocation ResidencyManager::createUpdateAllocation(size_t bytes)
    {
        ASSERT(bytes < ResidencyUploadBufferSizeBytes, "Residency manager can't handle this big allocation: %u", bytes);

        while (bytes > ResidencyUploadBufferSizeBytes - m_memoryInUse)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        UploadAllocation res;
        res.bytes = bytes;
        res.ptr = m_uploadMemory + m_memoryInUse;
        res.srcElement = m_memoryInUse;
        m_memoryInUse += bytes;
        return res;
    }

    void ResidencyManager::makeResident(const UploadAllocation& resource)
    {
        m_tasks.emplace_back(MakeResidentTask{ *m_device, m_uploadBuffer, resource });
    }

    void ResidencyManager::freeUpdateAllocation(const UploadAllocation& resource)
    {
        m_memoryInUse -= resource.bytes;
    }

    Buffer ResidencyManager::uploadBuffer()
    {
        return m_relocationBuffer;
    }

    Device& ResidencyManager::device()
    {
        return *m_device;
    }

	// ======================================================

	ResidencyManagerV2::ResidencyFuture::ResidencyFuture()
		: m_manager{ nullptr }
		, m_task{ nullptr }
	{}

	ResidencyManagerV2::ResidencyFuture::~ResidencyFuture()
	{
		if (m_task)
		{
			m_task->fence->blockUntilSignaled();
			m_manager->finishTaskFromConsumer(m_task);
		}
	}

	ResidencyManagerV2::ResidencyFuture::ResidencyFuture(ResidencyFuture&& future) noexcept
		: m_manager{ nullptr }
		, m_task{ nullptr }
	{
		std::swap(m_task, future.m_task);
		std::swap(m_manager, future.m_manager);
	}

	ResidencyManagerV2::ResidencyFuture& ResidencyManagerV2::ResidencyFuture::operator=(ResidencyFuture&& future) noexcept
	{
		std::swap(m_task, future.m_task);
		std::swap(m_manager, future.m_manager);
		return *this;
	}

	void ResidencyManagerV2::ResidencyFuture::blockUntilUploaded()
	{
		m_task->fence->blockUntilSignaled();
	}

	ResidencyManagerV2::ResidencyFuture::ResidencyFuture(ResidencyManagerV2 * manager, ResidencyTask * task)
		: m_manager{ manager }
		, m_task{ task }
	{}

	ResidencyManagerV2::ResidencyManagerV2(Device& device)
		: m_device{ device }
		, m_uploadBuffer{ device.createBuffer(BufferDescription()
			.elementSize(1)
			.elements(ResidencyUploadSize)
			.usage(ResourceUsage::Upload)
			.name("ResidencyManager UploadBuffer")) }
		, m_uploadMemory{ static_cast<uint8_t*>(m_uploadBuffer.resource().map(device)) }
		, m_ringBuffer{ tools::ByteRange{ reinterpret_cast<uint8_t*>(0ull), reinterpret_cast<uint8_t*>(ResidencyUploadSize) }, 1 }
		, m_workerFence{ m_device.createFence("ResidencyManagerV2 worker fence") }
		, m_workerFenceDirect{ m_device.createFence("ResidencyManagerV2 worker direct fence") }
		, m_alive{ true }
		, m_mutex{}
		, m_thread{[this]() { this->worker(); } }
	{}

	ResidencyManagerV2::~ResidencyManagerV2()
	{
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_alive = false;
		}
		m_thread.join();
		while (m_tasks.size() > 0) m_tasks.pop();
		m_finishedTasks.clear();
	}

	void ResidencyManagerV2::worker()
	{
		while (m_alive)
		{
			engine::shared_ptr<ResidencyTask> task;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				if (!m_tasks.empty())
				{
					task = std::move(m_tasks.front());
					m_tasks.pop();
				}
			}

			if (task)
			{
				workOnTask(std::move(task));
				m_device.processCommandLists(false);
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			
		}
	}

	void ResidencyManagerV2::workOnTask(engine::shared_ptr<ResidencyTask>&& _task)
	{
		ResidencyTask& task = *_task;
		auto ptr = static_cast<uint8_t*>(task.ptr);

		auto useCopyQueue = [&](const engine::Vector3<size_t>& size)->bool
		{
			auto gran = m_device.queue(CommandListType::Copy).transferGranularity();
			return
				(size.x % gran.x == 0ull) &&
				(size.y % gran.y == 0ull) &&
				(size.z % gran.z == 0ull);
		};

		bool copyQueueWork = false;
		bool directQueueWork = false;

		auto stall = [&]()
		{
			if (copyQueueWork)
			{
				m_workerFence.blockUntilSignaled();
				copyQueueWork = false;
			}
			if (directQueueWork)
			{
				m_workerFenceDirect.blockUntilSignaled();
				directQueueWork = false;
			}

			for (auto&& a : m_ringAllocations)
				m_ringBuffer.free(a);
			m_ringBuffer.reset();
			m_ringAllocations.clear();
		};

		if (task.dst)
		{
			auto toCopy = task.bytes;
			size_t doneCopying = 0;

			// buffer copy
			while (doneCopying < toCopy)
			{
				auto copyThisTime = std::min(toCopy - doneCopying, m_ringBuffer.maxAllocationSpace());
				if (copyThisTime == 0)
				{
					stall();

					copyThisTime = std::min(toCopy - doneCopying, m_ringBuffer.maxAllocationSpace());
					ASSERT(copyThisTime > 0, "Ran out of memory");
				}

				auto allocation = m_ringBuffer.allocate(copyThisTime);
				if (!allocation.size)
				{
					stall();

					copyThisTime = std::min(toCopy - doneCopying, m_ringBuffer.maxAllocationSpace());
					ASSERT(copyThisTime > 0, "Ran out of memory");
					allocation = m_ringBuffer.allocate(copyThisTime);
					ASSERT(allocation.ptr, "Invalid allocation");
				}

				m_ringAllocations.emplace_back(allocation);
				
				memcpy(&m_uploadMemory[m_ringBuffer.offset(allocation.ptr)], ptr + doneCopying, copyThisTime);

				auto cmd = m_device.createCommandList("MakeResidentTask", CommandListType::Copy);

				if (task.dst)
				{
					size_t bytes = copyThisTime;
					size_t fromBytes = m_ringBuffer.offset(allocation.ptr);
					size_t toBytes = task.dstIndexBytes + doneCopying;

					/*LOG("Residency copy from: %s [0..%i], to: %s. [0..%i]. bytes: %i, from: %i, to: %i",
						m_uploadBuffer.resource().description().descriptor.name,
						m_uploadBuffer.resource().description().descriptor.elementSize * m_uploadBuffer.resource().description().descriptor.elements,
						task.dst.description().descriptor.name,
						task.dst.description().descriptor.elementSize * task.dst.description().descriptor.elements,
						bytes,
						fromBytes,
						toBytes);*/

					cmd.copyBufferBytes(
						m_uploadBuffer,
						task.dst,
						bytes,
						fromBytes,
						toBytes);
				}

				m_device.submit(cmd, CommandListType::Copy);

				m_workerFence.increaseCPUValue();
				m_device.queue(CommandListType::Copy).signal(m_workerFence, m_workerFence.currentCPUValue());
				copyQueueWork = true;

				doneCopying += copyThisTime;
				
			}
		}
		else if (task.dstTexture)
		{
			// texture copy
			SurfacePieceImage srcImage;
			srcImage.width = task.srcWidth;
			srcImage.height = task.srcHeight;
			if(task.ptr)
				srcImage.ptr = reinterpret_cast<uint8_t*>(task.ptr);
			else
				srcImage.ptr = task.copyPtr.get();
			srcImage.rowPitchAlign = 1;
			
			SurfacePieceImage dstImage;
			dstImage.width = task.srcWidth;
			dstImage.height = task.srcHeight;
			dstImage.ptr = reinterpret_cast<uint8_t*>(0);
#ifndef _DURANGO
			dstImage.rowPitchAlign = static_cast<size_t>(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
#else
			dstImage.rowPitchAlign = static_cast<size_t>(D3D12XBOX_TEXTURE_DATA_PITCH_ALIGNMENT)));
#endif

			auto pieces = surfacePieces(
				task.dstTexture.format(),
				srcImage,
				dstImage,
				0, 0,
				0, 0,
				task.srcWidth,
				task.srcHeight);

			auto fbytes = formatBytes(task.dstTexture.format());
			auto blockCompressed = isBlockCompressedFormat(task.dstTexture.format());

			auto getRealPitch = [&](size_t width, size_t /*height*/)
			{
				const size_t compressedBlockSize = 4ull;
				auto minBlockSize = blockCompressed ? compressedBlockSize : 1;

				auto pieceWidth = roundUpToMultiple(width, minBlockSize) / minBlockSize;
				//auto pieceHeight = roundUpToMultiple(height, minBlockSize) / minBlockSize;

				return roundUpToMultiple(pieceWidth * fbytes, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
			};

			auto getRealPieceBytes = [&](size_t width, size_t height)
			{
				const auto compressedBlockSize = 4;
				auto minBlockSize = blockCompressed ? compressedBlockSize : 1;

				auto pieceWidth = roundUpToMultiple(roundUpToMultiple(width, minBlockSize) / minBlockSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
				auto pieceHeight = roundUpToMultiple(height, minBlockSize) / minBlockSize;

				return pieceWidth * pieceHeight * fbytes;
			};

			for (auto&& piece : pieces)
			{
				auto realPitch = getRealPitch(
					blockCompressed ? piece.width * 4ull : piece.width, 
					blockCompressed ? piece.height * 4ull : piece.height);
				auto realPieceBytes = getRealPieceBytes(
					blockCompressed ? piece.width * 4ull : piece.width,
					blockCompressed ? piece.height * 4ull : piece.height);

				if (m_ringBuffer.maxAllocationSpace() < realPieceBytes)
					stall();
				auto allocation = m_ringBuffer.allocate(realPieceBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
				if(!allocation.size)
					ASSERT(false, "Invalid allocation");

				m_ringAllocations.emplace_back(allocation);

				auto offset = m_ringBuffer.offset(allocation.ptr);
				size_t offsetInc = 0ull;
				size_t copiedLineBytes = 0ull;
				for (int i = 0; i < piece.height; ++i)
				{
					memcpy(&m_uploadMemory[offset + offsetInc], piece.srcptr + copiedLineBytes, piece.width * fbytes);
					copiedLineBytes += piece.srcRowPitch;
					offsetInc += realPitch;
				}

				if (useCopyQueue({ piece.width, piece.height, 1ull }))
				{
					auto cmd = m_device.createCommandList("MakeResidentTask", CommandListType::Copy);

					cmd.copyTexture(
						m_uploadBuffer.resource(),
						m_ringBuffer.offset(allocation.ptr),
						blockCompressed ? piece.width * 4 : piece.width,
						blockCompressed ? piece.height * 4 : piece.height,
						realPitch,
						task.dstTexture,
						task.dstX + piece.dstX,
						task.dstY + piece.dstY,
						task.dstMip,
						task.dstSlice,
						task.dstMipLevels);

					m_device.submit(cmd, CommandListType::Copy);

					m_workerFence.increaseCPUValue();
					m_device.queue(CommandListType::Copy).signal(m_workerFence, m_workerFence.currentCPUValue());
					copyQueueWork = true;
				}
				else
				{
					auto cmd = m_device.createCommandList("MakeResidentTask", CommandListType::Direct);

					cmd.copyTexture(
						m_uploadBuffer.resource(),
						m_ringBuffer.offset(allocation.ptr),
						blockCompressed ? piece.width * 4 : piece.width,
						blockCompressed ? piece.height * 4 : piece.height,
						realPitch,
						task.dstTexture,
						task.dstX + piece.dstX,
						task.dstY + piece.dstY,
						task.dstMip,
						task.dstSlice,
						task.dstMipLevels);

					m_device.submit(cmd, CommandListType::Direct);

					m_workerFenceDirect.increaseCPUValue();
					m_device.queue(CommandListType::Direct).signal(m_workerFenceDirect, m_workerFenceDirect.currentCPUValue());
					directQueueWork = true;
				}
			}
		}
		else
		{
			ASSERT(false, "Residency manager got empty task");
		}
	
		stall();

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_finishedTasks.emplace_back(std::move(_task));
		}

		m_device.queue(CommandListType::Copy).signal(*task.fence, task.fence->currentCPUValue());
	}

	ResidencyManagerV2::ResidencyFuture ResidencyManagerV2::uploadTemp(
		Buffer dst,
		size_t dstIndexBytes,
		void* ptr, size_t bytes)
	{
		auto task = engine::make_shared<ResidencyTask>();
		task->dst = dst;
		task->dstIndexBytes = dstIndexBytes;
		task->ptr = ptr;
		task->bytes = bytes;
		task->fence = engine::make_unique<Fence>(m_device.createFence("Buffer upload task finished fence"));
		task->fence->increaseCPUValue();
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_tasks.push(task);
		}
		
		return ResidencyFuture(this, task.get());
	};

	ResidencyManagerV2::ResidencyFuture ResidencyManagerV2::upload(
		void* srcPointer,
		size_t srcWidth,
		size_t srcHeight,

		Texture dst,
		size_t dstX,
		size_t dstY,
		size_t dstMip,
		size_t dstSlice,
		size_t dstMipLevels,
		bool copyOnUpload)
	{
		auto task = engine::make_shared<ResidencyTask>();
		
		if (copyOnUpload)
		{
			auto imageBytes = formatBytes(dst.format(), srcWidth, srcHeight);
			task->copyPtr = engine::unique_ptr<uint8_t[]>(new uint8_t[imageBytes]);
			memcpy(task->copyPtr.get(), srcPointer, imageBytes);
			task->ptr = nullptr;
		}
		else
			task->ptr = srcPointer;

		task->dstTexture = dst;
		task->dstMip = dstMip;
		task->dstSlice = dstSlice;
		task->dstMipLevels = dstMipLevels;
		task->dstX = dstX;
		task->dstY = dstY;
		task->srcWidth = srcWidth;
		task->srcHeight = srcHeight;

		task->fence = engine::make_unique<Fence>(m_device.createFence("Texture upload task finished fence"));
		task->fence->increaseCPUValue();
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_tasks.push(task);
		}

		return { this, task.get() };
	};

	void ResidencyManagerV2::finishTaskFromConsumer(ResidencyTask* task)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		for (auto ftask = m_finishedTasks.begin(); ftask != m_finishedTasks.end(); ++ftask)
		{
			if ((*ftask).get() == task)
			{
				m_finishedTasks.erase(ftask);
				break;
			}
		}
	}

#if 0
	ResidencyManagerV2::RingBuffer::RingBuffer(int size)
		: m_size{ size }
		, m_writePtr{ 0 }
		, m_freePtr{ 0 }
	{}

	int ResidencyManagerV2::RingBuffer::allocate(int bytes)
	{
		int res = 0;
		if (m_writePtr + bytes <= m_size)
		{
			if (m_writePtr < m_freePtr && m_writePtr + bytes > m_freePtr)
			{
				ASSERT(false, "Ran out of memory");
			}

			res = m_writePtr;
			m_writePtr += bytes;
			if (m_writePtr == m_size)
				m_writePtr = 0;
		}
		else
		{
			// we couldn't fit the allocation to the end of the ring
			// but freePtr is further, so it's between the end and write
			if (m_freePtr > m_writePtr)
			{
				ASSERT(false, "Ran out of memory");
			}

			// loop as we couldn't fit and we need single linear allocation
			m_writePtr = 0;

			if (m_writePtr + bytes > m_freePtr)
			{
				ASSERT(false, "Ran out of memory");
			}

			res = m_writePtr;
			m_writePtr += bytes;
		}
		m_allocations.emplace_back(AllocStruct{ res, bytes });
		return res;
	}

	void ResidencyManagerV2::RingBuffer::free(int allocation)
	{
		int lastEnd = 0;
		for (auto a = m_allocations.begin(); a != m_allocations.end(); ++a)
		{
			if (allocation == (*a).start)
			{
				lastEnd = (*a).start + (*a).size;
				m_allocations.erase(a);
				break;
			}
		}

		auto distanceToWrite = [&](int point)->int
		{
			if (point < m_writePtr)
				return m_writePtr - point;
			return (m_size - point) + m_writePtr;
		};

		int distance = 0;
		int reserveStart = 0;
		for (auto&& a : m_allocations)
		{
			auto distTo = distanceToWrite(a.start + a.size);
			if (distTo > distance)
			{
				distance = distTo;
				reserveStart = a.start;
			}
		}
		if (m_allocations.size() > 0)
			m_freePtr = reserveStart;
		else
			m_freePtr = lastEnd;
	}

	int ResidencyManagerV2::RingBuffer::space() const
	{
		if (m_freePtr <= m_writePtr)
		{
			if (m_freePtr == m_writePtr && m_allocations.size() > 0)
			{
				// we're full
				return 0;
			}
			else if (m_freePtr == m_writePtr && m_allocations.size() == 0)
			{
				// we're empty
				return m_size;
			}
			else
			{
				return m_size - (m_writePtr - m_freePtr);
			}
		}
		else
		{
			return m_freePtr - m_writePtr;
		}
	}

	int ResidencyManagerV2::RingBuffer::maxAllocationSpace() const
	{
		if (m_freePtr == m_writePtr && m_allocations.size() > 0)
		{
			return 0;
		}
		else if (m_freePtr == m_writePtr && m_allocations.size() == 0)
		{
			return m_size;
		}
		else if (m_freePtr < m_writePtr)
			return (m_size - m_writePtr) > m_freePtr ? m_size - m_writePtr : m_freePtr;
		else
			return m_freePtr - m_writePtr;
	}
#endif
}
