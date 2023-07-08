#include "Serialization.h"
#include "Transform.h"

namespace serialization
{
    template<>
    void serialize<engine::Transform>(Stream& stream, const engine::Transform& value)
    {
        serialize(stream, value.position());
        serialize(stream, value.rotation());
        serialize(stream, value.scale());
    }

    template<>
    void deserialize<engine::Transform>(Stream& stream, engine::Transform& value)
    {
        deserialize(stream, value.position());
        deserialize(stream, value.rotation());
        deserialize(stream, value.scale());
    }
}
