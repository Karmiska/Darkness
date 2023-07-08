#pragma once

#include "containers/vector.h"
#include "containers/queue.h"
#include "engine/primitives/Vector2.h"

namespace engine
{
    class AtlasAllocator
    {
    public:
        AtlasAllocator(
            int cellWidth, 
            int cellHeight,
            int initialWidth = -1,
            int initialHeight = -1);
        virtual ~AtlasAllocator() {};

        struct AtlasAllocation
        {
            int x;
            int y;
            int width;
            int height;
        };

        AtlasAllocation allocate();
        void free(const AtlasAllocation& allocation);

        Vector2<int> simulateSize(int allocationCount);
    protected:
        struct AtlasData
        {
            int cellWidth;
            int cellHeight;
            int width;
            int height;
        };

        AtlasData atlasData() const;
        virtual void OnAtlasSizeChange(int /*width*/, int /*height*/) {};

    private:
        
        AtlasData m_atlasData;
        AtlasAllocation privateAllocate(engine::vector<engine::queue<AtlasAllocation>>& free, int& width, int& height, bool simulate);
        engine::vector<engine::queue<AtlasAllocation>> m_free;
    };
}
