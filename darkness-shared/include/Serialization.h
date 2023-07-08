#pragma once

#include "Stream.h"

namespace serialization
{
    template<typename T>
    void serialize(Stream&, const T&);

    template<typename T>
    void deserialize(Stream&, T&);
}
