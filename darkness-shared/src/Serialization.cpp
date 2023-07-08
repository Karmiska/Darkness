#include "Serialization.h"

namespace serialization
{
    template<>
    void serialize<int>(Stream& stream, const int& value)
    {
        stream << value;
    }

    template<>
    void deserialize(Stream& stream, int& value)
    {
        stream >> value;
    }

    template<>
    void serialize<float>(Stream& stream, const float& value)
    {
        stream << value;
    }

    template<>
    void deserialize(Stream& stream, float& value)
    {
        stream >> value;
    }
}
