#include "platform/Intrinsic.h"

#ifdef _WIN32
#include <intrin.h>
#endif

namespace engine
{
#ifdef _WIN32
    uint16_t popcnt(uint16_t value)
    {
        return __popcnt16(value);
    }

    uint32_t popcnt(uint32_t value)
    {
        return __popcnt(value);
    }

    uint64_t popcnt(uint64_t value)
    {
        return __popcnt64(value);
    }
#endif
}
