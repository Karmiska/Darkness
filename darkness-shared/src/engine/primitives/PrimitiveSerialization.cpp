#include "Serialization.h"

#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector3.h"
#include "engine/primitives/Vector4.h"
#include "engine/primitives/Matrix3.h"
#include "engine/primitives/Matrix4.h"

using namespace engine;

namespace serialization
{
    template<>
    void serialize<Vector2f>(Stream& stream, const Vector2f& value)
    {
        stream << value.x;
        stream << value.y;
    }

    template<>
    void serialize<Vector3f>(Stream& stream, const Vector3f& value)
    {
        stream << value.x;
        stream << value.y;
        stream << value.z;
    }

    template<>
    void serialize<Vector4f>(Stream& stream, const Vector4f& value)
    {
        stream << value.x;
        stream << value.y;
        stream << value.z;
        stream << value.w;
    }

    template<>
    void serialize<Matrix3f>(Stream& stream, const Matrix3f& value)
    {
        stream << value.m00;
        stream << value.m01;
        stream << value.m02;
        
        stream << value.m10;
        stream << value.m11;
        stream << value.m12;

        stream << value.m20;
        stream << value.m21;
        stream << value.m22;
    }

    template<>
    void serialize<Matrix4f>(Stream& stream, const Matrix4f& value)
    {
        stream << value.m00;
        stream << value.m01;
        stream << value.m02;
        stream << value.m03;

        stream << value.m10;
        stream << value.m11;
        stream << value.m12;
        stream << value.m13;

        stream << value.m20;
        stream << value.m21;
        stream << value.m22;
        stream << value.m23;

        stream << value.m30;
        stream << value.m31;
        stream << value.m32;
        stream << value.m33;
    }

    template<>
    void deserialize<Vector2f>(Stream& stream, Vector2f& value)
    {
        stream >> value.x;
        stream >> value.y;
    }

    template<>
    void deserialize<Vector3f>(Stream& stream, Vector3f& value)
    {
        stream >> value.x;
        stream >> value.y;
        stream >> value.z;
    }

    template<>
    void deserialize<Vector4f>(Stream& stream, Vector4f& value)
    {
        stream >> value.x;
        stream >> value.y;
        stream >> value.z;
        stream >> value.w;
    }

    template<>
    void deserialize<Matrix3f>(Stream& stream, Matrix3f& value)
    {
        stream >> value.m00;
        stream >> value.m01;
        stream >> value.m02;

        stream >> value.m10;
        stream >> value.m11;
        stream >> value.m12;

        stream >> value.m20;
        stream >> value.m21;
        stream >> value.m22;
    }

    template<>
    void deserialize<Matrix4f>(Stream& stream, Matrix4f& value)
    {
        stream >> value.m00;
        stream >> value.m01;
        stream >> value.m02;
        stream >> value.m03;

        stream >> value.m10;
        stream >> value.m11;
        stream >> value.m12;
        stream >> value.m13;

        stream >> value.m20;
        stream >> value.m21;
        stream >> value.m22;
        stream >> value.m23;

        stream >> value.m30;
        stream >> value.m31;
        stream >> value.m32;
        stream >> value.m33;
    }
}
