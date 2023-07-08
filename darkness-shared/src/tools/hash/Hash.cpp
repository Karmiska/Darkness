#include "tools/hash/Hash.h"
#include "tools/hash/SpookyHashV2.h"

namespace tools
{
#ifdef HASH_DJB
    uintptr_t hashDJB(const engine::string& data)
    {
        return hashDJB(
            reinterpret_cast<const uint8_t*>(data.c_str()), 
            static_cast<unsigned int>(data.length()));
    }

    uintptr_t hashDJB(const uint8_t* ptr, unsigned int length)
    {
        uintptr_t hash = 5381;
        uintptr_t i = 0;

        for (i = 0; i < length; ++ptr, ++i)
        {
            hash = ((hash << 5) + hash) + (*ptr);
        }

        return hash;
    }
#endif

    uintptr_t hash(const engine::string& data)
    {
        return hash(
            reinterpret_cast<const uint8_t*>(data.c_str()),
            static_cast<unsigned int>(data.length()));
    }

    uintptr_t hash(const uint8_t* ptr, unsigned int length)
    {
#ifdef HASH_DJB
        return hashDJB(ptr, length);
#endif

#ifdef HASH_SPOOKY
        SpookyHash hash;
        hash.Init(123, 456);
        return static_cast<uintptr_t>(hash.Hash64(ptr, length, 789));
#endif

    }
}
