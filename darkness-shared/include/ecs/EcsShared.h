#pragma once

#include <cstdint>
#include <set>
#include <limits>

namespace ecs
{
    using ComponentTypeId = uint64_t;
    using ComponentArcheTypeId = uint64_t;
    const ComponentTypeId InvalidTypeId = std::numeric_limits<uint64_t>::max();
    const ComponentArcheTypeId InvalidArcheTypeId = 0xffffff;
    static ComponentTypeId GlobalComponentTypeId = 0;
}