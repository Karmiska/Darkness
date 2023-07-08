#pragma once

#include <cstdint>
#include "containers/vector.h"
#include <stack>
#include "containers/memory.h"

namespace engine
{
    class GpuMarkerContainer
    {
    public:
        struct MarkerItem
        {
            int parent;
            const char* query;
            uint32_t start;
            uint32_t stop;
        };

    public:
        GpuMarkerContainer(uint32_t maxQueryCount, uint32_t maxDepth = 50);
        
        uint32_t    startQuery(const char* queryName);
        uint32_t    stopQuery(uint32_t queryId);
        
        void        reset();

        uint32_t    markerCount() const;
        uint32_t    queryCount() const;

        engine::vector<MarkerItem>& items()
        {
            return m_markerStorage;
        }

    private:
        engine::vector<uint32_t> m_stack;
        uint32_t m_stackCount;

        engine::vector<MarkerItem> m_markerStorage;
        uint32_t m_markerCount;

        uint32_t m_queryCount;
    };

    class GpuMarkerStorage
    {
    public:
        engine::unique_ptr<GpuMarkerContainer> getNewContainer();
        void returnContainer(engine::unique_ptr<GpuMarkerContainer>&& container);
    private:
        engine::vector<engine::unique_ptr<GpuMarkerContainer>> m_freeMarkerContainers;
    };
}
