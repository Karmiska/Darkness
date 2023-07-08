#pragma once

#include <cstdint>
#include "containers/string.h"

#undef HASH_DJB
#define HASH_SPOOKY

namespace tools
{
#ifdef HASH_DJB
    uintptr_t hashDJB(const engine::string& data);
    uintptr_t hashDJB(const uint8_t* ptr, unsigned int length);
#endif

    uintptr_t hash(const engine::string& data);
    uintptr_t hash(const uint8_t* ptr, unsigned int length);
}
