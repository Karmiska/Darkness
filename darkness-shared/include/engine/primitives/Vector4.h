#pragma once

#include "Vector3.h"
#include "Vector2.h"

namespace engine
{
    template <typename T>
    class Vector4
    {
    public:
        T x;
        T y;
        T z;
        T w;

        Vector2<T> xy() const { return { x, y }; }
        Vector2<T> xz() const { return { x, z }; }
        Vector2<T> yx() const { return { y, x }; }
        Vector2<T> yz() const { return { y, z }; }
        Vector2<T> zx() const { return { z, x }; }
        Vector2<T> zy() const { return { z, y }; }
		Vector2<T> xx() const { return { x, x }; }
		Vector2<T> yy() const { return { y, y }; }
		Vector2<T> zz() const { return { z, z }; }
		Vector2<T> xw() const { return { x, w }; }
		Vector2<T> yw() const { return { y, w }; }
		Vector2<T> zw() const { return { z, w }; }

		Vector3<T> xxx() const { return { x, x, x }; }
		Vector3<T> yyy() const { return { y, y, y }; }
		Vector3<T> zzz() const { return { z, z, z }; }
		
		Vector3<T> xyz() const { return { x, y, z }; }
        Vector3<T> xzy() const { return { x, z, y }; }
        Vector3<T> yxz() const { return { y, x, z }; }
        Vector3<T> yzx() const { return { y, z, x }; }
        Vector3<T> zxy() const { return { z, x, y }; }
        Vector3<T> zyx() const { return { z, y, x }; }

		Vector3<T> xxy() const { return { x, x, y }; }
		Vector3<T> xxz() const { return { x, x, z }; }
		Vector3<T> yyx() const { return { y, y, x }; }
		Vector3<T> yyz() const { return { y, y, z }; }
		Vector3<T> zzx() const { return { z, z, x }; }
		Vector3<T> zzy() const { return { z, z, y }; }

		Vector3<T> xyy() const { return { x, y, y }; }
		Vector3<T> xzz() const { return { x, z, z }; }
		Vector3<T> yxx() const { return { y, x, x }; }
		Vector3<T> yzz() const { return { y, z, z }; }
		Vector3<T> zxx() const { return { z, x, x }; }
		Vector3<T> zyy() const { return { z, y, y }; }

		Vector4<T> xxxx() const { return { x, x, x, x }; }
		Vector4<T> yyyy() const { return { y, y, y, y }; }
		Vector4<T> zzzz() const { return { z, z, z, z }; }
		Vector4<T> wwww() const { return { w, w, w, w }; }

		Vector4<T> xxxy() const { return { x, x, x, y }; }
		Vector4<T> xxxz() const { return { x, x, x, z }; }
		Vector4<T> xxxw() const { return { x, x, x, w }; }

		Vector4<T> yyyx() const { return { y, y, y, x }; }
		Vector4<T> yyyz() const { return { y, y, y, z }; }
		Vector4<T> yyyw() const { return { y, y, y, w }; }

		Vector4<T> zzzx() const { return { z, z, z, x }; }
		Vector4<T> zzzy() const { return { z, z, z, y }; }
		Vector4<T> zzzw() const { return { z, z, z, w }; }

		Vector4<T> wwwx() const { return { w, w, w, x }; }
		Vector4<T> wwwy() const { return { w, w, w, y }; }
		Vector4<T> wwwz() const { return { w, w, w, z }; }


		Vector4<T> xxyy() const { return { x, x, y, y }; }
		Vector4<T> xxzz() const { return { x, x, z, z }; }
		Vector4<T> xxww() const { return { x, x, w, w }; }

		Vector4<T> yyxx() const { return { y, y, x, x }; }
		Vector4<T> yyzz() const { return { y, y, z, z }; }
		Vector4<T> yyww() const { return { y, y, w, w }; }

		Vector4<T> zzxx() const { return { z, z, x, x }; }
		Vector4<T> zzyy() const { return { z, z, y, y }; }
		Vector4<T> zzww() const { return { z, z, w, w }; }

		Vector4<T> wwxx() const { return { w, w, x, x }; }
		Vector4<T> wwyy() const { return { w, w, y, y }; }
		Vector4<T> wwzz() const { return { w, w, z, z }; }

		Vector4<T> xxyz() const { return { x, x, y, z }; }
		Vector4<T> xxzy() const { return { x, x, z, y }; }
		Vector4<T> xxyw() const { return { x, x, y, w }; }
		Vector4<T> xxwy() const { return { x, x, w, y }; }
		Vector4<T> xxwz() const { return { x, x, w, z }; }
		Vector4<T> xxzw() const { return { x, x, z, w }; }

		Vector4<T> yyxz() const { return { y, y, x, z }; }
		Vector4<T> yyzx() const { return { y, y, z, x }; }
		Vector4<T> yyxw() const { return { y, y, x, w }; }
		Vector4<T> yywx() const { return { y, y, w, x }; }
		Vector4<T> yywz() const { return { y, y, w, z }; }
		Vector4<T> yyzw() const { return { y, y, z, w }; }

		Vector4<T> zzxy() const { return { z, z, x, y }; }
		Vector4<T> zzyx() const { return { z, z, y, x }; }
		Vector4<T> zzxw() const { return { z, z, x, w }; }
		Vector4<T> zzwx() const { return { z, z, w, x }; }
		Vector4<T> zzwy() const { return { z, z, w, y }; }
		Vector4<T> zzyw() const { return { z, z, y, w }; }

		Vector4<T> wwxy() const { return { w, w, x, y }; }
		Vector4<T> wwyx() const { return { w, w, y, x }; }
		Vector4<T> wwxz() const { return { w, w, x, z }; }
		Vector4<T> wwzx() const { return { w, w, z, x }; }
		Vector4<T> wwzy() const { return { w, w, z, y }; }
		Vector4<T> wwyz() const { return { w, w, y, z }; }

        Vector4<T> xyzw() const { return { x, y, z, w }; }
        Vector4<T> xzyw() const { return { x, z, y, w }; }
        Vector4<T> yxzw() const { return { y, x, z, w }; }
        Vector4<T> yzxw() const { return { y, z, x, w }; }
        Vector4<T> zxyw() const { return { z, x, y, w }; }
        Vector4<T> zyxw() const { return { z, y, x, w }; }

        Vector4<T> xywz() const { return { x, y, w, z }; }
        Vector4<T> xzwy() const { return { x, z, w, y }; }
        Vector4<T> yxwz() const { return { y, x, w, z }; }
        Vector4<T> yzwx() const { return { y, z, w, x }; }
        Vector4<T> zxwy() const { return { z, x, w, y }; }
        Vector4<T> zywx() const { return { z, y, w, x }; }

        Vector4<T> xwyz() const { return { x, w, y, z }; }
        Vector4<T> xwzy() const { return { x, w, z, y }; }
        Vector4<T> ywxz() const { return { y, w, x, z }; }
        Vector4<T> ywzx() const { return { y, w, z, x }; }
        Vector4<T> zwxy() const { return { z, w, x, y }; }
        Vector4<T> zwyx() const { return { z, w, y, x }; }

        Vector4<T> wxyz() const { return { w, x, y, z }; }
        Vector4<T> wxzy() const { return { w, x, z, y }; }
        Vector4<T> wyxz() const { return { w, y, x, z }; }
        Vector4<T> wyzx() const { return { w, y, z, x }; }
        Vector4<T> wzxy() const { return { w, z, x, y }; }
        Vector4<T> wzyx() const { return { w, z, y, x }; }

		Vector4<T> xyxy() const { return { x, y, x, y }; }
		Vector4<T> xzxz() const { return { x, z, x, z }; }
		Vector4<T> xwxw() const { return { x, w, x, w }; }
		Vector4<T> xzxy() const { return { x, z, x, y }; }
		Vector4<T> xyxz() const { return { x, y, x, z }; }
		Vector4<T> xwxy() const { return { x, w, x, y }; }
		Vector4<T> xyxw() const { return { x, y, x, w }; }
		Vector4<T> xzxw() const { return { x, z, x, w }; }
		Vector4<T> xwxz() const { return { x, w, x, z }; }

		Vector4<T> yxyx() const { return { y, x, y, x }; }
		Vector4<T> yzyz() const { return { y, z, y, z }; }
		Vector4<T> ywyw() const { return { y, w, y, w }; }
		Vector4<T> yzyx() const { return { y, z, y, x }; }
		Vector4<T> yxyz() const { return { y, x, y, z }; }
		Vector4<T> ywyx() const { return { y, w, y, x }; }
		Vector4<T> yxyw() const { return { y, x, y, w }; }
		Vector4<T> yzyw() const { return { y, z, y, w }; }
		Vector4<T> ywyz() const { return { y, w, y, z }; }

		Vector4<T> zxzx() const { return { z, x, z, x }; }
		Vector4<T> zyzy() const { return { z, y, z, y }; }
		Vector4<T> zwzw() const { return { z, w, z, w }; }
		Vector4<T> zyzx() const { return { z, y, z, x }; }
		Vector4<T> zxzy() const { return { z, x, z, y }; }
		Vector4<T> zwzx() const { return { z, w, z, x }; }
		Vector4<T> zxzw() const { return { z, x, z, w }; }
		Vector4<T> zyzw() const { return { z, y, z, w }; }
		Vector4<T> zwzy() const { return { z, w, z, y }; }

		Vector4<T> wxwx() const { return { w, x, w, x }; }
		Vector4<T> wywy() const { return { w, y, w, y }; }
		Vector4<T> wzwz() const { return { w, z, w, z }; }
		Vector4<T> wywx() const { return { w, y, w, x }; }
		Vector4<T> wxwy() const { return { w, x, w, y }; }
		Vector4<T> wzwx() const { return { w, z, w, x }; }
		Vector4<T> wxwz() const { return { w, x, w, z }; }
		Vector4<T> wywz() const { return { w, y, w, z }; }
		Vector4<T> wzwy() const { return { w, z, w, y }; }

        void xy(const Vector2<T>& xy) { x = xy.x; y = xy.y; }
        void xyz(const Vector3<T>& xyz) { x = xyz.x; y = xyz.y; z = xyz.z; }
        void xyzw(const Vector4<T>& xyzw) { x = xyzw.x; y = xyzw.y; z = xyzw.z; w = xyzw.w; }

        Vector4() = default;

		Vector4(T x)
			: x{ x }
			, y{ static_cast<T>(0) }
			, z{ static_cast<T>(0) }
			, w{ static_cast<T>(0) }
		{}

		Vector4(T x, T y)
			: x{ x }
			, y{ y }
			, z{ static_cast<T>(0) }
			, w{ static_cast<T>(0) }
		{}

		Vector4(T x, T y, T z)
			: x{ x }
			, y{ y }
			, z{ z }
			, w{ static_cast<T>(0) }
		{}

        Vector4(T x, T y, T z, T w)
            : x{ x }
            , y{ y }
            , z{ z }
            , w{ w }
        {}

		Vector4(Vector3<T> vec)
			: x{ vec.x }
			, y{ vec.y }
			, z{ vec.z }
			, w{ static_cast<T>(0) }
		{}

        Vector4(Vector3<T> vec, T w)
            : x{ vec.x }
            , y{ vec.y }
            , z{ vec.z }
            , w{ w }
        {}
        
		Vector4(Vector2<T> vec)
			: x{ vec.x }
			, y{ vec.y }
			, z{ static_cast<T>(0) }
			, w{ static_cast<T>(0) }
		{}

		Vector4(Vector2<T> vecA, Vector2<T> vecB)
			: x{ vecA.x }
			, y{ vecA.y }
			, z{ vecB.x }
			, w{ vecB.y }
		{}

        Vector4(Vector2<T> vec, T z, T w)
            : x{ vec.x }
            , y{ vec.y }
            , z{ z }
            , w{ w }
        {}

        T* operator&()
        {
            return &x;
        }

        T& operator[](int index)
        {
            return *(&x + index);
        }

        Vector4 operator+(const Vector4& vec) const
        {
            return{ x + vec.x, y + vec.y, z + vec.z, w + vec.w };
        }
        
        Vector4 operator-(const Vector4& vec) const
        {
            return{ x - vec.x, y - vec.y, z - vec.z, w - vec.w };
        }

        Vector4 operator*(const Vector4& vec) const
        {
            return{ x * vec.x, y * vec.y, z * vec.z, w * vec.w };
        }

        Vector4 operator/(const Vector4& vec) const
        {
            return{ x / vec.x, y / vec.y, z / vec.z, w / vec.w };
        }

        Vector4 operator+(T val) const
        {
            return{ x + val, y + val, z + val, w + val };
        }

        Vector4 operator-(T val) const
        {
            return{ x - val, y - val, z - val, w - val };
        }

        Vector4 operator*(T val) const
        {
            return{ x * val, y * val, z * val, w * val };
        }

        Vector4 operator/(T val) const
        {
            return{ x / val, y / val, z / val, w / val };
        }

        Vector4& operator+=(const Vector4& vec)
        {
            x += vec.x;
            y += vec.y;
            z += vec.z;
            w += vec.w;
            return *this;
        }

        Vector4& operator-=(const Vector4& vec)
        {
            x -= vec.x;
            y -= vec.y;
            z -= vec.z;
            w -= vec.w;
            return *this;
        }

        Vector4& operator*=(const Vector4& vec)
        {
            x *= vec.x;
            y *= vec.y;
            z *= vec.z;
            w *= vec.w;
            return *this;
        }

        Vector4& operator/=(const Vector4& vec)
        {
            x /= vec.x;
            y /= vec.y;
            z /= vec.z;
            w /= vec.w;
            return *this;
        }

        Vector4& operator+=(T val)
        {
            x += val;
            y += val;
            z += val;
            w += val;
            return *this;
        }

        Vector4& operator-=(T val)
        {
            x -= val;
            y -= val;
            z -= val;
            w -= val;
            return *this;
        }

        Vector4& operator*=(T val)
        {
            x *= val;
            y *= val;
            z *= val;
            w *= val;
            return *this;
        }

        Vector4& operator/=(T val)
        {
            x /= val;
            y /= val;
            z /= val;
            w /= val;
            return *this;
        }

        bool operator==(const Vector4& other) const
        {
            return (x == other.x) &&
                   (y == other.y) &&
                   (z == other.z) &&
                   (w == other.w);
        }

        bool operator!=(const Vector4& other) const
        {
            return !(*this == other);
        }

        T dot(const Vector4& vec)
        {
            return{ x * vec.x + 
                    y * vec.y + 
                    z * vec.z +
                    w * vec.w };
        }
    };

    using Vector4f = Vector4<float>;
}
