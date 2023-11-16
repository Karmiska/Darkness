#pragma once

#include "tools/Debug.h"

namespace engine
{
    template <typename T>
    class Vector2
    {
    public:
        T x;
        T y;

        Vector2<T> xy() const { return { x, y }; }
        Vector2<T> yx() const { return { y, x }; }

        void xy(const Vector2<T>& xy) { x = xy.x; y = xy.y; }

        Vector2() = default;

        Vector2(T x, T y)
            : x{ x }
            , y{ y }
        {}

        Vector2(T xy)
            : x{ xy }
            , y{ xy }
        {}

        T* operator&()
        {
            return &x;
        }

        T& operator[](int index)
        {
            return *(&x + index);
        }

        Vector2 operator+(const Vector2& vec) const
        {
            return{ x + vec.x, y + vec.y };
        }

        Vector2 operator-(const Vector2& vec) const
        {
            return{ x - vec.x, y - vec.y };
        }

        Vector2 operator*(const Vector2& vec) const
        {
            return{ x * vec.x, y * vec.y };
        }

        Vector2 operator/(const Vector2& vec) const
        {
            return{ x / vec.x, y / vec.y };
        }

        Vector2 operator+(T val) const
        {
            return{ x + val, y + val };
        }

        Vector2 operator-(T val) const
        {
            return{ x - val, y - val };
        }

        Vector2 operator*(T val) const
        {
            return{ x * val, y * val };
        }

        Vector2 operator/(T val) const
        {
            return{ x / val, y / val };
        }

        Vector2& operator+=(const Vector2& vec)
        {
            x += vec.x;
            y += vec.y;
            return *this;
        }

        Vector2& operator-=(const Vector2& vec)
        {
            x -= vec.x;
            y -= vec.y;
            return *this;
        }

        Vector2& operator*=(const Vector2& vec)
        {
            x *= vec.x;
            y *= vec.y;
            return *this;
        }

        Vector2& operator/=(const Vector2& vec)
        {
            x /= vec.x;
            y /= vec.y;
            return *this;
        }

        Vector2& operator+=(T val)
        {
            x += val;
            y += val;
            return *this;
        }

        Vector2& operator-=(T val)
        {
            x -= val;
            y -= val;
            return *this;
        }

        Vector2& operator*=(T val)
        {
            x *= val;
            y *= val;
            return *this;
        }

        Vector2& operator/=(T val)
        {
            x /= val;
            y /= val;
            return *this;
        }

		static T magnitude(const Vector2& vec)
		{
			double xx = (double)vec.x;
			double yy = (double)vec.y;
			return std::sqrt(xx * xx + yy * yy);
		}

		T magnitude() const
		{
			double xx = (double)x * (double)x;
			double yy = (double)y * (double)y;
			double sum = xx + yy;
			if (sum == 0.0)
			{
				return static_cast<T>(0.0);
			}
			else
				return static_cast<T>(std::sqrt(sum));
		}

		static Vector2<T> normalize(const Vector2& vec)
		{
			double xx = (double)vec.x;
			double yy = (double)vec.y;
			double mag = std::sqrt(xx * xx + yy * yy);
			return{ vec.x / mag, vec.y / mag };
		}

		Vector2<T> invert() const
		{
			return { -x, -y };
		}

		Vector2<T>& normalize()
		{
			double xx = (double)x;
			double yy = (double)y;
			double mag = std::sqrt(xx * xx + yy * yy);
			if (mag != 0.0f)
			{
				x = (T)(x / mag);
				y = (T)(y / mag);
			}
			else
			{
				LOG("Tried normalizing zero length vector");
			}
			return *this;
		}

        bool operator==(const Vector2& other) const
        {
            return (x == other.x) &&
                   (y == other.y);
        }

        bool operator!=(const Vector2& other) const
        {
            return !(*this == other);
        }

        bool operator>=(const Vector2& other) const
        {
            return 
                (x >= other.x) &&
                (y >= other.y);
        }

        bool operator<=(const Vector2& other) const
        {
            return
                (x <= other.x) &&
                (y <= other.y);
        }

        bool operator>(const Vector2& other) const
        {
            return
                (x > other.x) &&
                (y > other.y);
        }

        bool operator<(const Vector2& other) const
        {
            return
                (x < other.x) &&
                (y < other.y);
        }

        T dot(const Vector2& vec)
        {
            return{ x * vec.x +
                    y * vec.y };
        }

        Vector2<T> abs() const
        {
            return { std::abs(x), std::abs(y) };
        }

        bool operator>=(float val) { return (x >= val) && (y >= val); }
        bool operator<=(float val) { return (x <= val) && (y <= val); }
        bool operator>(float val) { return (x > val) && (y > val); }
        bool operator<(float val) { return (x < val) && (y < val); }

    };

    using Vector2f = Vector2<float>;
}
