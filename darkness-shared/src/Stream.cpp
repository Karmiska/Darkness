#include "Stream.h"

namespace serialization
{
    Stream& operator<<(Stream& stream, const int& value)
    {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(int));
        return stream;
    }

    Stream& operator >> (Stream& stream, int& value)
    {
        stream.read(reinterpret_cast<char*>(&value), sizeof(int));
        return stream;
    }

    Stream& operator<<(Stream& stream, const float& value)
    {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(float));
        return stream;
    }

    Stream& operator >> (Stream& stream, float& value)
    {
        stream.read(reinterpret_cast<char*>(&value), sizeof(float));
        return stream;
    }
}
