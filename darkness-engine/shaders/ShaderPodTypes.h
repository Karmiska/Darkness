#pragma once

#include <cstdint>
#include "engine/primitives/Matrix4.h"
#include "engine/primitives/Vector4.h"
#include "engine/primitives/Vector2.h"
#include "engine/primitives/Vector3.h"

namespace engine
{
	struct Int1
	{
		int32_t x;
	};
	struct Int2
	{
		int32_t x;
		int32_t y;
	};
	struct Int3
	{
		int32_t x;
		int32_t y;
		int32_t z;
	};
	struct Int4
	{
		int32_t x;
		int32_t y;
		int32_t z;
		int32_t w;
	};

	struct Uint1
	{
		uint32_t x;
	};
	struct Uint2
	{
		uint32_t x;
		uint32_t y;
	};
	struct Uint3
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;
	};
	struct Uint4
	{
		uint32_t x;
		uint32_t y;
		uint32_t z;
		uint32_t w;

		Uint4& operator=(const Vector4<size_t>& vec)
		{
			x = static_cast<uint32_t>(vec.x);
			y = static_cast<uint32_t>(vec.y);
			z = static_cast<uint32_t>(vec.z);
			w = static_cast<uint32_t>(vec.w);
			return *this;
		};
	};

	struct Float1
	{
		float x;
	};
	struct Float2
	{
		float x;
		float y;

		Float2() = default;

		constexpr Float2(float _x, float _y) noexcept
			: x{ _x }
			, y{ _y }
		{}

		constexpr Float2(const Vector2f& vec) noexcept
			: x{ vec.x }
			, y{ vec.y }
		{}
	};
	struct Float3
	{
		float x;
		float y;
		float z;

		Float3() = default;

		Float3(float _x, float _y, float _z)
			: x{ _x }
			, y{ _y }
			, z{ _z }
		{}

		Float3(const Vector3f& vec)
			: x{ vec.x }
			, y{ vec.y }
			, z{ vec.z }
		{}

		Float3(const Vector2f& vec, float _z)
			: x{ vec.x }
			, y{ vec.y }
			, z{ _z }
		{}

		Float3(const Vector2f& vec)
			: x{ vec.x }
			, y{ vec.y }
			, z{ 0.0f }
		{}
	};
	struct Float4
	{
		float x;
		float y;
		float z;
		float w;

		Float4() = default;

		Float4(float _x, float _y, float _z, float _w)
			: x{ _x }
			, y{ _y }
			, z{ _z }
			, w{ _w }
		{}

		Float4(const Vector4f& vec)
			: x{ vec.x }
			, y{ vec.y }
			, z{ vec.z }
			, w{ vec.w }
		{}

		Float4(const Vector3f& vec)
			: x{ vec.x }
			, y{ vec.y }
			, z{ vec.z }
			, w{ 1.0f }
		{}

		Float4(const Vector3f& vec, float _w)
			: x{ vec.x }
			, y{ vec.y }
			, z{ vec.z }
			, w{ _w }
		{}

		Float4(const Vector2f& vec)
			: x{ vec.x }
			, y{ vec.y }
			, z{ 0.0f }
			, w{ 1.0f }
		{}

		Float4(const Vector2f& vec, float _z, float _w)
			: x{ vec.x }
			, y{ vec.y }
			, z{ _z }
			, w{ _w }
		{}

		Float4(const Vector2f& vec, float _z)
			: x{ vec.x }
			, y{ vec.y }
			, z{ _z }
			, w{ 1.0f }
		{}
	};

	struct Double1
	{
		double x;
	};
	struct Double2
	{
		double x;
		double y;
	};
	struct Double3
	{
		double x;
		double y;
		double z;
	};
	struct Double4
	{
		double x;
		double y;
		double z;
		double w;
	};

	using Int = Int1;
	using Uint = Uint1;
	using Dword = uint32_t;
	using Half = int16_t;
	using Float = float;
	using Bool = bool;

	template<typename T, unsigned int R, unsigned int C>
	struct TemplateType
	{
		T data[R * C];

		static TemplateType<T, R, C> zero()
		{
			TemplateType<T, R, C> res;
			memset(&res.data[0], 0, sizeof(T) * (R * C));
			return res;
		}

		TemplateType<T, R, C> operator+(const TemplateType<T, R, C>& a)
		{
			TemplateType<T, R, C> res;
			for (int i = 0; i < R * C; ++i)
			{
				res.data[i] = data[i] + a.data[i];
			}
			return res;
		};

		TemplateType<T, R, C> operator-(const TemplateType<T, R, C>& a)
		{
			TemplateType<T, R, C> res;
			for (int i = 0; i < R * C; ++i)
			{
				res.data[i] = data[i] - a.data[i];
			}
			return res;
		};

		TemplateType<T, R, C> operator*(const TemplateType<T, R, C>& a)
		{
			TemplateType<T, R, C> res;
			for (int i = 0; i < R * C; ++i)
			{
				res.data[i] = data[i] * a.data[i];
			}
			return res;
		};

		TemplateType<T, R, C> operator/(const TemplateType<T, R, C>& a)
		{
			TemplateType<T, R, C> res;
			for (int i = 0; i < R * C; ++i)
			{
				res.data[i] = data[i] / a.data[i];
			}
			return res;
		};

		template<class T2>
		TemplateType<T, R, C> operator+(const T2& a)
		{
			TemplateType<T, R, C> res;
			for (int i = 0; i < R * C; ++i)
			{
				res.data[i] = data[i] + static_cast<T>(a);
			}
			return res;
		};

		template<class T2>
		TemplateType<T, R, C> operator-(const T2& a)
		{
			TemplateType<T, R, C> res;
			for (int i = 0; i < R * C; ++i)
			{
				res.data[i] = data[i] - static_cast<T>(a);
			}
			return res;
		};

		template<class T2>
		TemplateType<T, R, C> operator*(const T2& a)
		{
			TemplateType<T, R, C> res;
			for (int i = 0; i < R * C; ++i)
			{
				res.data[i] = data[i] * static_cast<T>(a);
			}
			return res;
		};

		template<class T2>
		TemplateType<T, R, C> operator/(const T2& a)
		{
			TemplateType<T, R, C> res;
			for (int i = 0; i < R * C; ++i)
			{
				res.data[i] = data[i] / static_cast<T>(a);
			}
			return res;
		};


		TemplateType<T, R, C>& operator+=(const TemplateType<T, R, C>& a)
		{
			for (int i = 0; i < R * C; ++i)
			{
				data[i] += a.data[i];
			}
			return *this;
		};

		TemplateType<T, R, C>& operator-=(const TemplateType<T, R, C>& a)
		{
			for (int i = 0; i < R * C; ++i)
			{
				data[i] -= a.data[i];
			}
			return *this;
		};

		TemplateType<T, R, C>& operator*=(const TemplateType<T, R, C>& a)
		{
			for (int i = 0; i < R * C; ++i)
			{
				data[i] *= a.data[i];
			}
			return *this;
		};

		TemplateType<T, R, C>& operator/=(const TemplateType<T, R, C>& a)
		{
			for (int i = 0; i < R * C; ++i)
			{
				data[i] /= a.data[i];
			}
			return *this;
		};

		template<class T2>
		TemplateType<T, R, C>& operator+=(const T2& a)
		{
			for (int i = 0; i < R * C; ++i)
			{
				data[i] += static_cast<T>(a);
			}
			return *this;
		};

		template<class T2>
		TemplateType<T, R, C>& operator-=(const T2& a)
		{
			for (int i = 0; i < R * C; ++i)
			{
				data[i] -= static_cast<T>(a);
			}
			return *this;
		};

		template<class T2>
		TemplateType<T, R, C>& operator*=(const T2& a)
		{
			for (int i = 0; i < R * C; ++i)
			{
				data[i] *= static_cast<T>(a);
			}
			return *this;
		};

		template<class T2>
		TemplateType<T, R, C>& operator/=(const T2& a)
		{
			for (int i = 0; i < R * C; ++i)
			{
				data[i] /= static_cast<T>(a);
			}
			return *this;
		};
	};

	template<typename T, unsigned int R, unsigned int C>
	TemplateType<T, R, C> operator+(TemplateType<T, R, C> a, const TemplateType<T, R, C>& b)
	{
		a += b;
		return a;
	}

	template<typename T, unsigned int R, unsigned int C>
	TemplateType<T, R, C> operator-(TemplateType<T, R, C> a, const TemplateType<T, R, C>& b)
	{
		a += b;
		return a;
	}

	template<typename T, unsigned int R, unsigned int C>
	TemplateType<T, R, C> operator*(TemplateType<T, R, C> a, const TemplateType<T, R, C>& b)
	{
		a *= b;
		return a;
	}

	template<typename T, unsigned int R, unsigned int C>
	TemplateType<T, R, C> operator/(TemplateType<T, R, C> a, const TemplateType<T, R, C>& b)
	{
		a /= b;
		return a;
	}

	template<typename T, unsigned int R, unsigned int C, class T2>
	TemplateType<T, R, C> operator+(TemplateType<T, R, C> a, const T2& b)
	{
		a += b;
		return a;
	}

	template<typename T, unsigned int R, unsigned int C, class T2>
	TemplateType<T, R, C> operator-(TemplateType<T, R, C> a, const T2& b)
	{
		a -= b;
		return a;
	}

	template<typename T, unsigned int R, unsigned int C, class T2>
	TemplateType<T, R, C> operator*(TemplateType<T, R, C> a, const T2& b)
	{
		a *= b;
		return a;
	}

	template<typename T, unsigned int R, unsigned int C, class T2>
	TemplateType<T, R, C> operator/(TemplateType<T, R, C> a, const T2& b)
	{
		a /= b;
		return a;
	}


	using Int1x1 = TemplateType<int32_t, 1, 1>;
	using Int2x1 = TemplateType<int32_t, 2, 1>;
	using Int3x1 = TemplateType<int32_t, 3, 1>;
	using Int4x1 = TemplateType<int32_t, 4, 1>;
	using Int1x2 = TemplateType<int32_t, 1, 2>;
	using Int2x2 = TemplateType<int32_t, 2, 2>;
	using Int3x2 = TemplateType<int32_t, 3, 2>;
	using Int4x2 = TemplateType<int32_t, 4, 2>;
	using Int1x3 = TemplateType<int32_t, 1, 3>;
	using Int2x3 = TemplateType<int32_t, 2, 3>;
	using Int3x3 = TemplateType<int32_t, 3, 3>;
	using Int4x3 = TemplateType<int32_t, 4, 3>;
	using Int1x4 = TemplateType<int32_t, 1, 4>;
	using Int2x4 = TemplateType<int32_t, 2, 4>;
	using Int3x4 = TemplateType<int32_t, 3, 4>;
	using Int4x4 = TemplateType<int32_t, 4, 4>;

	using Uint1x1 = TemplateType<uint32_t, 1, 1>;
	using Uint2x1 = TemplateType<uint32_t, 2, 1>;
	using Uint3x1 = TemplateType<uint32_t, 3, 1>;
	using Uint4x1 = TemplateType<uint32_t, 4, 1>;
	using Uint1x2 = TemplateType<uint32_t, 1, 2>;
	using Uint2x2 = TemplateType<uint32_t, 2, 2>;
	using Uint3x2 = TemplateType<uint32_t, 3, 2>;
	using Uint4x2 = TemplateType<uint32_t, 4, 2>;
	using Uint1x3 = TemplateType<uint32_t, 1, 3>;
	using Uint2x3 = TemplateType<uint32_t, 2, 3>;
	using Uint3x3 = TemplateType<uint32_t, 3, 3>;
	using Uint4x3 = TemplateType<uint32_t, 4, 3>;
	using Uint1x4 = TemplateType<uint32_t, 1, 4>;
	using Uint2x4 = TemplateType<uint32_t, 2, 4>;
	using Uint3x4 = TemplateType<uint32_t, 3, 4>;
	using Uint4x4 = TemplateType<uint32_t, 4, 4>;

	using Float1x1 = TemplateType<float, 1, 1>;
	using Float2x1 = TemplateType<float, 2, 1>;
	using Float3x1 = TemplateType<float, 3, 1>;
	using Float4x1 = TemplateType<float, 4, 1>;
	using Float1x2 = TemplateType<float, 1, 2>;
	using Float2x2 = TemplateType<float, 2, 2>;
	using Float3x2 = TemplateType<float, 3, 2>;
	using Float4x2 = TemplateType<float, 4, 2>;
	using Float1x3 = TemplateType<float, 1, 3>;
	using Float2x3 = TemplateType<float, 2, 3>;
	using Float3x3 = TemplateType<float, 3, 3>;
	using Float4x3 = TemplateType<float, 4, 3>;
	using Float1x4 = TemplateType<float, 1, 4>;
	using Float2x4 = TemplateType<float, 2, 4>;
	using Float3x4 = TemplateType<float, 3, 4>;
	using Float4x4 = TemplateType<float, 4, 4>;

	using Double1x1 = TemplateType<double, 1, 1>;
	using Double2x1 = TemplateType<double, 2, 1>;
	using Double3x1 = TemplateType<double, 3, 1>;
	using Double4x1 = TemplateType<double, 4, 1>;
	using Double1x2 = TemplateType<double, 1, 2>;
	using Double2x2 = TemplateType<double, 2, 2>;
	using Double3x2 = TemplateType<double, 3, 2>;
	using Double4x2 = TemplateType<double, 4, 2>;
	using Double1x3 = TemplateType<double, 1, 3>;
	using Double2x3 = TemplateType<double, 2, 3>;
	using Double3x3 = TemplateType<double, 3, 3>;
	using Double4x3 = TemplateType<double, 4, 3>;
	using Double1x4 = TemplateType<double, 1, 4>;
	using Double2x4 = TemplateType<double, 2, 4>;
	using Double3x4 = TemplateType<double, 3, 4>;
	using Double4x4 = TemplateType<double, 4, 4>;
}
