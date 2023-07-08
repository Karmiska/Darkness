#include "tools/AtlasAllocator.h"

namespace engine
{
    AtlasAllocator::AtlasAllocator(
        int cellWidth, 
        int cellHeight,
        int initialWidth,
        int initialHeight)
        : m_atlasData{
            cellWidth,
            cellHeight,
            0,
            0 }
    {
        if((initialWidth != -1) && (initialHeight != -1))
        { 
            m_atlasData.width = initialWidth / cellWidth;
            m_atlasData.height = initialHeight / cellHeight;

            for (int i = 0; i < m_atlasData.height; ++i)
            {
                m_free.emplace_back(engine::queue<AtlasAllocation>());
            }

            for (int y = 0; y < m_atlasData.height; ++y)
            {
                for (int x = 0; x < m_atlasData.width; ++x)
                {
                    m_free[y].push(AtlasAllocation{ 
                        x * m_atlasData.cellWidth, 
                        y * m_atlasData.cellHeight, 
                        m_atlasData.cellWidth,
                        m_atlasData.cellHeight });
                }
            }
        }
    }

    AtlasAllocator::AtlasData AtlasAllocator::atlasData() const
    {
        return m_atlasData;
    }

    AtlasAllocator::AtlasAllocation AtlasAllocator::allocate()
    {
        return privateAllocate(m_free, m_atlasData.width, m_atlasData.height, false);
    }

    AtlasAllocator::AtlasAllocation AtlasAllocator::privateAllocate(engine::vector<engine::queue<AtlasAllocation>>& free, int& width, int& height, bool simulate)
    {
        for (auto&& line : free)
        {
            if (line.size() > 0)
            {
                auto res = line.front();
                line.pop();
                return res;
            }
        }

        if (width == 0)
        {
            free.emplace_back(engine::queue<AtlasAllocation>());
            free.back().push(AtlasAllocation{ 0, 0, m_atlasData.cellWidth, m_atlasData.cellHeight });

            width = 1;
            height = 1;
        }
        else if (width > height)
        {
            // height realloc
            free.emplace_back(engine::queue<AtlasAllocation>());
            height += 1;

            int y = (static_cast<int>(free.size()) - 1) * m_atlasData.cellHeight;
            for (int i = 0; i < width; ++i)
            {
                free.back().push(AtlasAllocation{ i * m_atlasData.cellWidth, y, m_atlasData.cellWidth, m_atlasData.cellHeight });
            }
        }
        else
        {
            int x = width * m_atlasData.cellWidth;
            for (int i = 0; i < height; ++i)
            {
                free[i].push(AtlasAllocation{ x, i * m_atlasData.cellHeight, m_atlasData.cellWidth, m_atlasData.cellHeight });
            }
            width += 1;
        }
        if (!simulate)
        {
            OnAtlasSizeChange(width * m_atlasData.cellWidth, height * m_atlasData.cellHeight);
            return allocate();
        }
        else
        {
            return privateAllocate(free, width, height, true);
        }
    }

    void AtlasAllocator::free(const AtlasAllocation& allocation)
    {
        int freeListIndex = allocation.y / m_atlasData.cellHeight;
        m_free[freeListIndex].push(allocation);
    }

    engine::Vector2<int> AtlasAllocator::simulateSize(int allocationCount)
    {
        engine::vector<engine::queue<AtlasAllocation>> m_simulatedFree;
        int width = 0;
        int height = 0;

        for (int i = 0; i < allocationCount; ++i)
            privateAllocate(m_simulatedFree, width, height, true);

        return { width * m_atlasData.cellWidth, height * m_atlasData.cellHeight };
    }
}
