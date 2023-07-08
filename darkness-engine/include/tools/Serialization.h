#pragma once

#include "containers/string.h"

#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Vector4.h"
#include "engine/primitives/Matrix3.h"
#include "engine/primitives/Matrix4.h"
#include "engine/primitives/Quaternion.h"

namespace engine
{
    namespace serialization
    {
        enum KeyTypes
        {
            InvalidKey      = 0,
            NodeName        = 1,
            NodeList        = 2,
            ComponentName   = 3,
            ComponentList   = 4,
            PropertyName    = 5,
            PropertyValue   = 6,
            PropertyList    = 7
        };

        enum ObjectTypes
        {
            InvalidObject   = 0,
            Node            = 1,
            Component       = 2,
            Property        = 3
        };

        enum class TypeId
        {
            InvalidId       = 0,
            Bool            = 1,
            Char            = 2,
            Short           = 3,
            Int             = 4,
            UnsignedChar    = 5,
            UnsignedShort   = 6,
            UnsignedInt     = 7,
            Float           = 8,
            Double          = 9,
            Vector2f        = 10,
            Vector3f        = 11,
            Vector4f        = 12,
            Matrix3f        = 13,
            Matrix4f        = 14,
            Quaternionf     = 15,
            String          = 16,
            Projection      = 17,
            ButtonPush      = 18,
            ButtonToggle    = 19,
            LightType       = 20,
            CollisionShape  = 21,
        };

        template <typename K, typename T>
        engine::string keyPrefix(K key, T id = TypeId::InvalidId)
        {
            return engine::string(std::to_string((int)key).c_str()) + ":" + engine::string(std::to_string((int)id).c_str());
        }
    }

    template <typename T>
    void writeJsonValue(
        rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer,
        const engine::string& name,
        T& value);

    /*template <typename T>
    void readJsonValue(rapidjson::Reader<rapidjson::StringBuffer>& writer, T& value);*/

    char readJsonValue_char(int value);
    short readJsonValue_short(int value);
    int readJsonValue_int(int value);
    unsigned char readJsonValue_unsignedchar(unsigned int value);
    unsigned short readJsonValue_unsignedshort(unsigned int value);
    unsigned int readJsonValue_unsignedint(unsigned int value);

    bool readJsonValue_bool(bool value);
    float readJsonValue_float(double value);
    double readJsonValue_double(double value);

    Vector2f readJsonValue_vector2f(const engine::string& value);
    Vector3f readJsonValue_vector3f(const engine::string& value);
    Vector4f readJsonValue_vector4f(const engine::string& value);
    Matrix3f readJsonValue_matrix3f(const engine::string& value);
    Matrix4f readJsonValue_matrix4f(const engine::string& value);
    Quaternionf readJsonValue_quaternionf(const engine::string& value);

    enum class Projection;
    Projection readJsonValue_projection(const engine::string& value);

    enum class CollisionShape;
    CollisionShape readJsonValue_collisionShape(const engine::string& value);

    enum class ButtonPush;
    ButtonPush readJsonValue_buttonPush(bool value);

    enum class ButtonToggle;
    ButtonToggle readJsonValue_buttonToggle(bool value);

    engine::string readJsonValue_string(const engine::string& value);

    enum class LightType;
    LightType readJsonValue_lightType(const engine::string& value);
}

