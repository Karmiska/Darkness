#pragma once

#include "engine/rendering/ModelResourceAllocator.h"
#include "engine/graphics/ResourceOwners.h"
#include "engine/rendering/BufferSettings.h"
#include "engine/graphics/Fence.h"
#include "tools/RingBuffer.h"
#include "containers/vector.h"
#include "containers/queue.h"

#include <mutex>
#include <thread>

namespace engine
{
    class Device;

    struct UploadAllocation
    {
        uint8_t* ptr;
		size_t bytes;
		size_t srcElement;
		size_t gpuIndex;
        BufferSRV buffer;
        BufferIBV indexBuffer;
    };

    struct RelocateAllocation
    {
        Buffer srcBuffer;
        Buffer dstBuffer;
		size_t fromIndex;
		size_t toIndex;
    };

    struct ModelResource
    {
        bool allocated = false;
        ModelResourceAllocation modelResource;
        engine::vector<UploadAllocation> uploads;
    };

    class MakeResidentTask
    {
    public:
        MakeResidentTask(Device& device, Buffer uploadBuffer, UploadAllocation resource);
    };

    class ResidencyManager
    {
    public:
        ResidencyManager(Device& device);
        UploadAllocation createUpdateAllocation(size_t bytes);
        void makeResident(const UploadAllocation& resource);
        void freeUpdateAllocation(const UploadAllocation& resource);

        Buffer uploadBuffer();
        Device& device();
    private:
        Device* m_device;
        BufferOwner m_uploadBuffer;
        BufferOwner m_relocationBuffer;
        uint8_t* m_uploadMemory;
		size_t m_memoryInUse;
        engine::vector<MakeResidentTask> m_tasks;
    };

	constexpr size_t ResidencyUploadSize = 1024ull * 1024ull * 5ull;
	class ResidencyManagerV2
	{
	private:
		struct ResidencyTask
		{
			// source pointer
			void* ptr;
			engine::unique_ptr<uint8_t[]> copyPtr;

			// buffer data
			Buffer dst;
			size_t dstIndexBytes;
			size_t bytes;

			// texture data
			Texture dstTexture;
			size_t dstMip;
			size_t dstSlice;
			size_t dstMipLevels;
			size_t dstX;
			size_t dstY;
			size_t srcWidth;
			size_t srcHeight;

			engine::unique_ptr<Fence> fence;
		};

	public:
		ResidencyManagerV2(Device& device);
		~ResidencyManagerV2();

		class ResidencyFuture
		{
		public:
			ResidencyFuture(const ResidencyFuture&) = delete;
			ResidencyFuture& operator=(const ResidencyFuture&) = delete;
			ResidencyFuture(ResidencyFuture&&) noexcept;
			ResidencyFuture& operator=(ResidencyFuture&&) noexcept;
			~ResidencyFuture();

			void blockUntilUploaded();
		private:
			friend class ResidencyManagerV2;
			ResidencyFuture();
			ResidencyFuture(ResidencyManagerV2* manager, ResidencyTask* task);

			ResidencyManagerV2* m_manager;
			ResidencyTask* m_task;
		};

		ResidencyFuture uploadTemp(
			Buffer dst,
			size_t dstIndexBytes,
			void* ptr, size_t bytes);

		ResidencyFuture upload(
			void* srcPointer,
			size_t srcWidth,
			size_t srcHeight,

			Texture dst,
			size_t dstX,
			size_t dstY,
			size_t dstMip,
			size_t dstSlice,
			size_t dstMipLevels,
			bool copyOnUpload = false);

	private:
		friend class ResidencyFuture;
		Device& m_device;
		engine::queue<std::shared_ptr<ResidencyTask>> m_tasks;
		engine::vector<std::shared_ptr<ResidencyTask>> m_finishedTasks;

		void finishTaskFromConsumer(ResidencyTask* task);

	private:
		/*class RingBuffer
		{
		public:
			RingBuffer(int size);
			int allocate(int bytes);
			void free(int allocation);
			int space() const;
			int maxAllocationSpace() const;

		private:
			struct AllocStruct
			{
				int start;
				int size;
			};
			engine::vector<AllocStruct> m_allocations;
			int m_size;
			int m_writePtr;
			int m_freePtr;
		};*/

	private:
		BufferOwner m_uploadBuffer;
		uint8_t* m_uploadMemory;
		tools::RingBuffer m_ringBuffer;
		Fence m_workerFence;
		Fence m_workerFenceDirect;
		engine::vector<tools::RingBuffer::AllocStruct> m_ringAllocations;
		bool m_alive;
		std::mutex m_mutex;
		std::thread m_thread;

		void worker();
		void workOnTask(engine::shared_ptr<ResidencyTask>&& task);

	
	};
}
