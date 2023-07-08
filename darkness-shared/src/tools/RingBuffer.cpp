#include "tools/RingBuffer.h"
#include "tools/ToolsCommon.h"
#include "tools/Debug.h"

namespace tools
{
    RingBuffer::RingBuffer(tools::ByteRange range, size_t align)
        : m_alignment{ align }
        , m_range{ range }
		, m_writePtr{ m_range.start }
		, m_lengthPtr{ 0 }
		, m_endSkip{ 0 }
    {
        //LOG("RingBuffer allocated: %zu", range.sizeBytes() / 1024 / 1024);
    }

    RingBuffer::AllocStruct RingBuffer::allocate(size_t bytes)
    {
        return allocate(bytes, m_alignment);
    }

	void RingBuffer::reset()
	{
		//ASSERT(m_allocations.size() == 0, "There are allocations. We can't reset. Need to freem them first.");
		m_writePtr = m_range.start;
		m_lengthPtr = 0;
	}

	RingBuffer::AllocStruct RingBuffer::allocate(size_t bytes, size_t align)
    {
		uintptr_t writePtr = roundUpToMultiple(m_writePtr, align);
		bytes = roundUpToMultiple(bytes, align);

		auto allocationIncrease = (writePtr + bytes) - m_writePtr;
		if (m_lengthPtr + allocationIncrease > m_range.sizeBytes())
			return { nullptr, 0 };

		if (writePtr + bytes > m_range.stop)
		{
			m_endSkip = m_range.stop - m_writePtr;
			m_writePtr = m_range.start;
			m_lengthPtr += m_endSkip;

			writePtr = roundUpToMultiple(m_writePtr, align);
			m_endSkip += writePtr - m_writePtr;

			allocationIncrease = (writePtr + bytes) - m_writePtr;
			if (m_lengthPtr + allocationIncrease > m_range.sizeBytes())
				return { nullptr, 0 };
		}

		m_writePtr += allocationIncrease;
		if (m_writePtr > m_range.stop)
			m_writePtr = m_range.start;
		m_lengthPtr += allocationIncrease;

		//LOG("Returning allocation: %p, %u", writePtr, static_cast<unsigned>(bytes));
		return { reinterpret_cast<void*>(writePtr), bytes };



		/*uintptr_t res = 0;
		if (writePtr + bytes <= m_range.stop)
		{
			if (writePtr < m_freePtr && writePtr + bytes > m_freePtr)
			{
				LOG("Ran out of memory");
				return { nullptr, 0 };
			}

			res = writePtr;
			writePtr += bytes;
			if (writePtr == m_range.stop)
				writePtr = m_range.start;
		}
		else
		{
			// we couldn't fit the allocation to the end of the ring
			// but freePtr is further, so it's between the end and write
			if (m_freePtr > writePtr)
			{
				LOG("Ran out of memory");
				return { nullptr, 0 };
			}

			// loop as we couldn't fit and we need single linear allocation
			writePtr = m_range.start;

			if (writePtr + bytes > m_freePtr)
			{
				LOG("Ran out of memory");
				return { nullptr, 0 };
			}

			res = writePtr;
			writePtr += bytes;
		}
		
		m_writePtr = writePtr;

		//m_allocations.emplace_back(AllocStruct{ res, bytes });
		return { reinterpret_cast<void*>(res), bytes };*/
    }

    void RingBuffer::free(RingBuffer::AllocStruct ptr)
    {
		//LOG("Free allocation: %p, %u", ptr.ptr, static_cast<unsigned>(ptr.size));

		struct LastAllocation
		{
			uintptr_t internalStart;
			uintptr_t actualStart;
		};
		auto lastAllocation = [&]()->LastAllocation
		{
			LastAllocation res{};
			if (m_lengthPtr > (m_writePtr - m_range.start))
			{
				res.internalStart =  m_range.stop - (m_lengthPtr - (m_writePtr - m_range.start));
			}
			else
				res.internalStart = m_writePtr - m_lengthPtr;
			res.actualStart = res.internalStart;


			if (res.internalStart + m_endSkip == m_range.stop)
				res.actualStart = m_range.start;

			return res;
		};

		uintptr_t allocation = reinterpret_cast<uintptr_t>(ptr.ptr);

		auto removeLast = [&](AllocStruct& , const LastAllocation& lastalloc)
		{
			m_lengthPtr -= ptr.size;

			if (lastalloc.actualStart != lastalloc.internalStart)
			{
				m_lengthPtr -= m_range.stop - lastalloc.internalStart;
				m_endSkip = 0;
			}
		};

		auto lalloc = lastAllocation();
		if (allocation == lalloc.actualStart)
		{
			removeLast(ptr, lalloc);
		}
		else
		{
			m_frees.emplace_back(ptr);
		}

		bool opp = true;
		while (opp)
		{
			opp = false;
			for (auto fr = m_frees.begin(); fr != m_frees.end(); ++fr)
			{
				if (reinterpret_cast<uintptr_t>((*fr).ptr) == lalloc.actualStart)
				{
					removeLast(*fr, lalloc);
					opp = true;
					m_frees.erase(fr);
					lalloc = lastAllocation();
					break;
				}
			}
		}

#if 0
		uintptr_t allocation = reinterpret_cast<uintptr_t>(ptr);
		uintptr_t lastEnd = 0;
		/*for (auto a = m_allocations.begin(); a != m_allocations.end(); ++a)
		{
			if (allocation == (*a).start)
			{
				lastEnd = (*a).start + (*a).size;
				m_allocations.erase(a);
				break;
			}
		}*/

		auto distanceToWrite = [&](uintptr_t point)->uintptr_t
		{
			if (point < m_writePtr)
				return m_writePtr - point;
			return (m_range.stop - point) + m_writePtr;
		};

		uintptr_t distance = 0;
		uintptr_t reserveStart = 0;
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
#endif
    }

    size_t RingBuffer::offset(void* ptr) const
    {
        return static_cast<size_t>(reinterpret_cast<uintptr_t>(ptr) - m_range.start);
    }

    void* RingBuffer::ptrFromOffset(size_t offset) const
    {
        return reinterpret_cast<void*>(m_range.start + static_cast<uintptr_t>(offset));
    }

	size_t RingBuffer::maxAllocationSpace() const
	{
		return m_range.sizeBytes() - m_lengthPtr;

		/*if (m_freePtr == m_writePtr && m_allocations.size() > 0)
		{
			return 0;
		}
		else if (m_freePtr == m_writePtr && m_allocations.size() == 0)
		{
			return m_range.size();
		}
		else if (m_freePtr < m_writePtr)
			return (m_range.size() - m_writePtr) > m_freePtr ? m_range.size() - m_writePtr : m_freePtr;
		else
			return m_freePtr - m_writePtr;
			*/
	}
}
