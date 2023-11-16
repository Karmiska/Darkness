#pragma once

#include "tools/Debug.h"
#include "Vector2.h"

namespace engine
{
    template <typename T>
    class Vector3
    {
    public:
        T x;
        T y;
        T z;

        Vector2<T> xy() const { return { x, y }; }
        Vector2<T> xz() const { return { x, z }; }
        Vector2<T> yx() const { return { y, x }; }
        Vector2<T> yz() const { return { y, z }; }
        Vector2<T> zx() const { return { z, x }; }
        Vector2<T> zy() const { return { z, y }; }

        Vector3<T> xyz() const { return { x, y, z }; }
        Vector3<T> xzy() const { return { x, z, y }; }
        Vector3<T> yxz() const { return { y, x, z }; }
        Vector3<T> yzx() const { return { y, z, x }; }
        Vector3<T> zxy() const { return { z, x, y }; }
        Vector3<T> zyx() const { return { z, y, x }; }

        void xy(const Vector2<T>& xy) { x = xy.x; y = xy.y; }
        void xyz(const Vector3<T>& xyz) { x = xyz.x; y = xyz.y; z = xyz.z; }

        Vector3() = default;

        Vector3(T x, T y, T z)
            : x{ x }
            , y{ y }
            , z{ z }
        {}

        Vector3(Vector2<T> xy, T z)
            : x{ xy.x }
            , y{ xy.y }
            , z{ z }
        {}

        T* operator&()
        {
            return &x;
        }

        T& operator[](int index)
        {
            ASSERT(index < 3);
            return *(&x + index);
        }

        Vector3 operator+(const Vector3& vec) const
        {
            return{ x + vec.x, y + vec.y, z + vec.z };
        }
        
        Vector3 operator-(const Vector3& vec) const
        {
            return{ x - vec.x, y - vec.y, z - vec.z };
        }

        Vector3 operator*(const Vector3& vec) const
        {
            return{ x * vec.x, y * vec.y, z * vec.z };
        }

        Vector3 operator/(const Vector3& vec) const
        {
            return{ x / vec.x, y / vec.y, z / vec.z };
        }

        Vector3& operator+=(const Vector3& vec)
        {
            x += vec.x;
            y += vec.y;
            z += vec.z;
            return *this;
        }

        Vector3& operator-=(const Vector3& vec)
        {
            x -= vec.x;
            y -= vec.y;
            z -= vec.z;
            return *this;
        }

        Vector3& operator*=(const Vector3& vec)
        {
            x *= vec.x;
            y *= vec.y;
            z *= vec.z;
            return *this;
        }

        Vector3& operator/=(const Vector3& vec)
        {
            x /= vec.x;
            y /= vec.y;
            z /= vec.z;
            return *this;
        }

        bool operator<(const Vector3& vec)
        {
            return (x < vec.x && y < vec.y && z < vec.z);
        }

        Vector3 operator+(T val) const
        {
            return{ x + val, y + val, z + val };
        }

        Vector3 operator-(T val) const
        {
            return{ x - val, y - val, z - val };
        }

        Vector3 operator*(T val) const
        {
            return{ x * val, y * val, z * val };
        }

        Vector3 operator/(T val) const
        {
            return{ x / val, y / val, z / val };
        }

        Vector3& operator+=(T val)
        {
            x += val;
            y += val;
            z += val;
            return *this;
        }

        Vector3& operator-=(T val)
        {
            x -= val;
            y -= val;
            z -= val;
            return *this;
        }

        Vector3& operator*=(T val)
        {
            x *= val;
            y *= val;
            z *= val;
            return *this;
        }

        Vector3& operator/=(T val)
        {
            x /= val;
            y /= val;
            z /= val;
            return *this;
        }

        bool operator==(const Vector3& other) const
        {
            return (x == other.x) &&
                   (y == other.y) &&
                   (z == other.z);
        }

        bool operator!=(const Vector3& other) const
        {
            return !(*this == other);
        }

        T dot(const Vector3& vec)
        {
            return{ x * vec.x + 
                    y * vec.y + 
                    z * vec.z };
        }

        static T magnitude(const Vector3& vec)
        {
            double xx = (double)vec.x;
            double yy = (double)vec.y;
            double zz = (double)vec.z;
            return std::sqrt(xx * xx + yy * yy + zz * zz);
        }

        T magnitude() const
        {
            double xx = (double)x * (double)x;
            double yy = (double)y * (double)y;
            double zz = (double)z * (double)z;
            double sum = xx + yy + zz;
            if (sum == 0.0)
            {
                return static_cast<T>(0.0);
            }
            else
                return static_cast<T>(std::sqrt(sum));
        }

        static Vector3<T> normalize(const Vector3& vec)
        {
            double xx = (double)vec.x;
            double yy = (double)vec.y;
            double zz = (double)vec.z;
            double mag = std::sqrt(xx * xx + yy * yy + zz * zz);
            return{ vec.x / mag, vec.y / mag, vec.z / mag };
        }

        Vector3<T> invert() const
        {
            return { -x, -y, -z };
        }

        Vector3<T>& normalize()
        {
            double xx = (double)x;
            double yy = (double)y;
            double zz = (double)z;
            double mag = std::sqrt(xx * xx + yy * yy + zz * zz);
            if (mag != 0.0f)
            {
                x = (T)(x / mag);
                y = (T)(y / mag);
                z = (T)(z / mag);
            }
            return *this;
        }

        Vector3 cross(const Vector3& vec)
        {
            return{
                y * vec.z - z * vec.y,
                z * vec.x - x * vec.z,
                x * vec.y - y * vec.x
            };
        }

        static Vector3 lerp(const Vector3& a, const Vector3& b, T value)
        {
            return a + ((b - a) * value);
        }
    };

    using Vector3f = Vector3<float>;
}
