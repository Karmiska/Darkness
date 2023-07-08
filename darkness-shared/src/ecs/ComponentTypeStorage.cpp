#include "ecs/ComponentTypeStorage.h"

namespace ecs
{
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
