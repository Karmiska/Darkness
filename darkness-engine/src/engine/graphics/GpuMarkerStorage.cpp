#include "engine/graphics/GpuMarkerStorage.h"
#include "tools/Debug.h"

namespace engine
{
    GpuMarkerContainer::GpuMarkerContainer(
        uint32_t maxQueryCount,
        uint32_t maxDepth)
        : m_stack(maxDepth)
        , m_stackCount{ 0u }
        , m_markerStorage(maxQueryCount)
        , m_markerCount{ 0u }
        , m_queryCount{ 0u }
    {
    }

    uint32_t GpuMarkerContainer::startQuery(const char* queryName)
    {
        ASSERT(m_stackCount < m_stack.size(), 
            "GpuMarkerContainer ran out of stack space");

        ASSERT(m_markerCount < m_markerStorage.size(), 
            "GpuMarkerContainer ran out of storage space");

        m_markerStorage[m_markerCount].parent = m_stackCount > 0 ? m_stack[m_stackCount - 1] : -1;
        m_markerStorage[m_markerCount].query = queryName;
        m_markerStorage[m_markerCount].start = m_queryCount;
        m_stack[m_stackCount] = m_markerCount;

        ++m_markerCount;
        ++m_stackCount;
        ++m_queryCount;
        return m_queryCount - 1;
    }

    uint32_t GpuMarkerContainer::stopQuery(uint32_t /*queryId*/)
    {
        --m_stackCount;
        auto start = m_stack[m_stackCount];

        auto res = m_queryCount;
        m_markerStorage[start].stop = res;
        ++m_queryCount;

        return res;
    }

    void GpuMarkerContainer::reset()
    {
        m_stackCount = 0u;
        m_markerCount = 0u;
        m_queryCount = 0u;
    }

    uint32_t GpuMarkerContainer::markerCount() const
    {
        return m_markerCount;
    }

    uint32_t GpuMarkerContainer::queryCount() const
    {
        return m_queryCount;
    }

    engine::unique_ptr<GpuMarkerContainer> GpuMarkerStorage::getNewContainer()
    {
        if (m_freeMarkerContainers.size() > 0)
        {
            engine::unique_ptr<GpuMarkerContainer> res;
            std::swap(m_freeMarkerContainers.back(), res);
            m_freeMarkerContainers.erase(m_freeMarkerContainers.end()-1);
            return res;
        }
        return engine::make_unique<GpuMarkerContainer>(10000u);
    }

    void GpuMarkerStorage::returnContainer(engine::unique_ptr<GpuMarkerContainer>&& container)
    {
        m_freeMarkerContainers.emplace_back(std::move(container));
        m_freeMarkerContainers.back()->reset();
    }
}
