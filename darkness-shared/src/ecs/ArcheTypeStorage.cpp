#include "ecs/ArcheTypeStorage.h"

namespace ecs
{
    constexpr uint64_t fnv1aHash(uint64_t value, uint64_t hash)
    {
        hash ^= (value & 0xff);
        hash *= FnvPrime;
        hash ^= (value & 0xff00) >> 8;
        hash *= FnvPrime;
        hash ^= (value & 0xff0000) >> 16;
        hash *= FnvPrime;
        hash ^= (value & 0xff000000) >> 24;
        hash *= FnvPrime;

        hash ^= (value & 0xff00000000) >> 32;
        hash *= FnvPrime;
        hash ^= (value & 0xff0000000000) >> 40;
        hash *= FnvPrime;
        hash ^= (value & 0xff000000000000) >> 48;
        hash *= FnvPrime;
        hash ^= (value & 0xff00000000000000) >> 56;
        hash *= FnvPrime;
        return hash;

    }

    uint64_t ArcheTypeHash(const std::set<ComponentTypeId>& types)
    {
        auto type = types.begin();
        if (type == types.end())
            return 0;

        uint64_t hash = fnv1aHash(*type);
        ++type;
        while (type != types.end())
        {
            hash ^= fnv1aHash(*type);
            ++type;
        }
        return hash;
    }
}
