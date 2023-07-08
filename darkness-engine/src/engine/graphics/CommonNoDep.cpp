#include "engine/graphics/CommonNoDep.h"
#include <algorithm>

namespace engine
{
    int AfterRefreshCounter = 0;

    size_t mipCount(size_t width, size_t height)
    {
        return static_cast<int>(1 + floor(log10((float)std::max(width, height)) / log10(2.0)));
    }

    size_t mipCount(size_t width, size_t height, size_t depth)
    {
        return static_cast<int>(1 + floor(log10((float)std::max(std::max(width, height), depth)) / log10(2.0)));
    }
}
