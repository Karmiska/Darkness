#include "tools/ToolsCommon.h"
#include "tools/Debug.h"
#include <intrin.h>

size_t alignToIndex(size_t align)
{
    ASSERT(align > 0, "Can not align to 0 alignment");
    unsigned long index;
    unsigned char isNonzero;
    isNonzero = _BitScanReverse64(&index, static_cast<unsigned long long>(align));
    if (isNonzero)
        return index;
    else
        return 0;
}

size_t indexToAlign(size_t index)
{
    return static_cast<size_t>(static_cast<size_t>(1) << index);
}

bool isPowerOfTwo(size_t align)
{
    return (align & (align - 1)) == 0;
}
