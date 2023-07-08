#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix3.h"
#include "Matrix4.h"
#include "Math.h"

namespace engine
{
    template <typename T>
    class Quaternion
    {
    public:
        T x;
        T y;
        T z;
        T w;

        Quaternion()
            : x{}
            , y{}
            , z{}
            , w{}
        {}

        Quaternion(T x, T y, T z, T w)
            : x{ x }
            , y{ y }
            , z{ z }
            , w{ w }
        {}

        Quaternion(Vector3<T> vec, T w)
            : x{ vec.x }
            , y{ vec.y }
            , z{ vec.z }
            , w{ w }
        {}

        Vector3<T> complex() const
        {
            return{ x, y, z };
        }

        void complex(Vector3<T> val)
        {
            x = val.x;
            y = val.y;
            z = val.z;
        }

        T real() const
        {
            return w;
        }

        Quaternion conjugate() const
        {
            return Quaternion(-complex(), real());
        }

        Quaternion inverse() const
        {
            return conjugate() / normalize();
        }

        Quaternion normalize() const
        {
            Quaternion res = *this;
            double n = std::sqrt((double)res.x * (double)res.x + (double)res.y * (double)res.y + (double)res.z * (double)res.z + (double)res.w * (double)res.w);
            res.x /= (T)n;
            res.y /= (T)n;
            res.z /= (T)n;
            res.w /= (T)n;
            return res;
        }

        Quaternion product(const Quaternion<T>& val) const
        {
            return{
                y * val.z - z * val.y + x * val.w + w * val.x,
                z * val.x - x * val.z + y * val.w + w * val.y,
                x * val.y - y * val.x + z * val.w + w * val.z,
                w * val.w - x * val.x - y * val.y - z * val.z
            };
        }

        static Quaternion lerp(const Quaternion& a, const Quaternion& b, const float t)
        {
            Quaternion r;
            float t_ = 1 - t;
            r.x = t_*a.x + t*b.x;
            r.y = t_*a.y + t*b.y;
            r.z = t_*a.z + t*b.z;
            r.w = t_*a.w + t*b.w;
            return r.normalize();
        }

        static Quaternion slerp(const Quaternion& a, const Quaternion& b, const float t)
        {
            Quaternion r;
            float t_ = 1 - t;
            float Wa, Wb;
            float theta = acos(a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w);
            float sn = sin(theta);
            Wa = sin(t_*theta) / sn;
            Wb = sin(t*theta) / sn;
            r.x = Wa*a.x + Wb*b.x;
            r.y = Wa*a.y + Wb*b.y;
            r.z = Wa*a.z + Wb*b.z;
            r.w = Wa*a.w + Wb*b.w;
            return r.normalize();
        }

        Quaternion operator*(const Quaternion<T>& val) const
        {
            return product(val);
        }

        Quaternion operator*(T val) const
        {
            return{ complex() * val, real() * val };
        }

        Quaternion operator+(const Quaternion<T>& val) const
        {
            return{ x + val.x, 
                    y + val.y, 
                    z + val.z, 
                    w + val.w };
        }

        Quaternion operator-(const Quaternion<T>& val) const
        {
            return{ x - val.x,
                    y - val.y,
                    z - val.z,
                    w - val.w };
        }

        Quaternion operator-() const
        {
            return{ -x, -y, -z, -w };
        }

        Quaternion operator/(T val) const
        {
            return{ complex() / val, real() / val };
        }

        T magnitude() const
        {
            return{ std::sqrtf(x * x + y * y + z * z + w * w) };
        }

        /*Vector4<T> toAxisAngles() const
        {
            Vector4<T> result;
            Quaternion<T> thisQuaternion = *this;

            if ((double)thisQuaternion.w > 1.0) thisQuaternion.normalise(); // if w>1 acos and sqrt will produce errors, this cant happen if quaternion is normalised
            result.w = (T)(2.0 * std::acos((double)thisQuaternion.w));
            double s = std::sqrt(1.0 - (double)thisQuaternion.w * (double)thisQuaternion.w); // assuming quaternion normalised then w is less than 1, so term always positive.
            if (s < 0.001) { // test to avoid divide by zero, s is always positive due to sqrt
                             // if s close to zero then direction of axis not important
                result.x = (T)thisQuaternion.x; // if it is important that axis is normalised then replace with x=1; y=z=0;
                result.y = (T)thisQuaternion.y;
                result.z = (T)thisQuaternion.z;
            }
            else {
                result.x = (T)thisQuaternion.x / s; // normalise axis
                result.y = (T)thisQuaternion.y / s;
                result.z = (T)thisQuaternion.z / s;
            }
            return result;
        }*/

        Matrix4<T> toMatrix() const
        {
			Matrix4<T> res;

			T sqw = w*w;
			T sqx = x*x;
			T sqy = y*y;
			T sqz = z*z;

			// invs (inverse square length) is only required if quaternion is not already normalised
			T invs = static_cast<T>(1) / (sqx + sqy + sqz + sqw);
			res.m00 = (sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
			res.m11 = (-sqx + sqy - sqz + sqw)*invs;
			res.m22 = (-sqx - sqy + sqz + sqw)*invs;

			T tmp1 = x*y;
			T tmp2 = z*w;
			res.m10 = static_cast<T>(2.0) * (tmp1 + tmp2)*invs;
			res.m01 = static_cast<T>(2.0) * (tmp1 - tmp2)*invs;

			tmp1 = x*z;
			tmp2 = y*w;
			res.m20 = static_cast<T>(2.0) * (tmp1 - tmp2)*invs;
			res.m02 = static_cast<T>(2.0) * (tmp1 + tmp2)*invs;
			tmp1 = y*z;
			tmp2 = x*w;
			res.m21 = static_cast<T>(2.0) * (tmp1 + tmp2)*invs;
			res.m12 = static_cast<T>(2.0) * (tmp1 - tmp2)*invs;

			res.m03 = static_cast<T>(0);
			res.m13 = static_cast<T>(0);
			res.m23 = static_cast<T>(0);
			res.m30 = static_cast<T>(0);
			res.m31 = static_cast<T>(0);
			res.m32 = static_cast<T>(0);
			res.m33 = static_cast<T>(1);

			return res;
        }

        Vector3<T> toEulerAngles() const
        {
            Vector3<T> result;
			
			T sqw = w*w;
			T sqx = x*x;
			T sqy = y*y;
			T sqz = z*z;
			T unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
			T test = x*y + z*w;

			if (test > 0.499*unit) { // singularity at north pole
				result.y = static_cast<T>(2) * atan2(x, w);
				result.z = static_cast<T>(M_PIl) / static_cast<T>(2);
				result.x = 0;
                return { RAD_TO_DEG * result.x, RAD_TO_DEG * result.y, RAD_TO_DEG * result.z };
			}
			if (test < -0.499*unit) { // singularity at south pole
				result.y = -2 * atan2(x, w);
				result.z = static_cast<T>(-M_PIl) / static_cast<T>(2);
				result.x = 0;
                return { RAD_TO_DEG * result.x, RAD_TO_DEG * result.y, RAD_TO_DEG * result.z };
			}
			result.y = atan2(static_cast<T>(2) * y*w - static_cast<T>(2) * x*z, sqx - sqy - sqz + sqw);
			result.z = asin(static_cast<T>(2) * test / unit);
			result.x = atan2(static_cast<T>(2) * x*w - static_cast<T>(2) * y*z, -sqx + sqy - sqz + sqw);

            return { RAD_TO_DEG * result.x, RAD_TO_DEG * result.y, RAD_TO_DEG * result.z };
        }

		static Quaternion<T> fromEulerAngles(Vector3<T> angles)
		{
			return Quaternion<T>::fromEulerAngles(angles.x, angles.y, angles.z);
		}

		static Quaternion<T> fromEulerAngles(T _x, T _y, T _z)
		{
			T c1 = std::cos((DEG_TO_RAD * _y) / static_cast<T>(2));
			T s1 = std::sin((DEG_TO_RAD * _y) / static_cast<T>(2));
			T c2 = std::cos((DEG_TO_RAD * _z) / static_cast<T>(2));
			T s2 = std::sin((DEG_TO_RAD * _z) / static_cast<T>(2));
			T c3 = std::cos((DEG_TO_RAD * _x) / static_cast<T>(2));
			T s3 = std::sin((DEG_TO_RAD * _x) / static_cast<T>(2));
			T c1c2 = c1*c2;
			T s1s2 = s1*s2;
			
			Quaternion<T> res;
			res.w = c1c2*c3 - s1s2*s3;
			res.x = c1c2*s3 + s1s2*c3;
			res.y = s1*c2*c3 + c1*s2*s3;
			res.z = c1*s2*c3 - s1*c2*s3;
			
			return res;
		}

        //public static Vector3 operator *(Quaternion quat, Vector3 vec) {
        Vector3<T> operator*(const Vector3<T>& vec)
        {
            float num =   x * 2.0f;
            float num2 =  y * 2.0f;
            float num3 =  z * 2.0f;
            float num4 =  x * num;
            float num5 =  y * num2;
            float num6 =  z * num3;
            float num7 =  x * num2;
            float num8 =  x * num3;
            float num9 =  y * num3;
            float num10 = w * num;
            float num11 = w * num2;
            float num12 = w * num3;
            Vector3<T> result;
            result.x = (1.0f - (num5 + num6)) * vec.x + (num7 - num12) * vec.y + (num8 + num11) * vec.z;
            result.y = (num7 + num12) * vec.x + (1.0f - (num4 + num6)) * vec.y + (num9 - num10) * vec.z;
            result.z = (num8 - num11) * vec.x + (num9 + num10) * vec.y + (1.0f - (num4 + num5)) * vec.z;
            return result;
        }

        Vector3<T> toAxisAngle() const
        {
            Quaternion<T> temp = *this;
            Vector3<T> res;
            
            if (temp.w > 1) temp = temp.normalize(); // if w>1 acos and sqrt will produce errors, this cant happen if quaternion is normalised

            //T angle = 2 * std::acos(temp.w);
            T s = std::sqrt(1 - temp.w*temp.w); // assuming quaternion normalised then w is less than 1, so term always positive.
            if (s < static_cast<T>(0.001)) { // test to avoid divide by zero, s is always positive due to sqrt
                                // if s close to zero then direction of axis not important
                res.x = temp.x; // if it is important that axis is normalised then replace with x=1; y=z=0;
                res.y = temp.y;
                res.z = temp.z;
            }
            else {
                res.x = temp.x / s; // normalise axis
                res.y = temp.y / s;
                res.z = temp.z / s;
            }
            return res;
        }

        static Quaternion<T> fromMatrix(const Matrix4<T> mat)
        {
			Quaternion<T> res;
			T trace = mat.m00 + mat.m11 + mat.m22; // I removed + 1.0f; see discussion with Ethan
			if (trace > 0) {// I changed M_EPSILON to 0
				T s = 0.5f / std::sqrtf(trace + 1.0f);
				res.w = 0.25f / s;
				res.x = (mat.m21 - mat.m12) * s;
				res.y = (mat.m02 - mat.m20) * s;
				res.z = (mat.m10 - mat.m01) * s;
			}
			else {
				if (mat.m00 > mat.m11 && mat.m00 > mat.m22) {
					T s = 2.0f * sqrtf(1.0f + mat.m00 - mat.m11 - mat.m22);
					res.w = (mat.m21 - mat.m12) / s;
					res.x = 0.25f * s;
					res.y = (mat.m01 + mat.m10) / s;
					res.z = (mat.m02 + mat.m20) / s;
				}
				else if (mat.m11 > mat.m22) {
					T s = 2.0f * sqrtf(1.0f + mat.m11 - mat.m00 - mat.m22);
					res.w = (mat.m02 - mat.m20) / s;
					res.x = (mat.m01 + mat.m10) / s;
					res.y = 0.25f * s;
					res.z = (mat.m12 + mat.m21) / s;
				}
				else {
					T s = 2.0f * sqrtf(1.0f + mat.m22 - mat.m00 - mat.m11);
					res.w = (mat.m10 - mat.m01) / s;
					res.x = (mat.m02 + mat.m20) / s;
					res.y = (mat.m12 + mat.m21) / s;
					res.z = 0.25f * s;
				}
			}
			return res;
        }
    };

    template<typename T>
    Vector3<T> operator*(const Quaternion<T> quat, const Vector3<T>& vec)
    {
        float num =   quat.x * 2.0f;
        float num2 =  quat.y * 2.0f;
        float num3 =  quat.z * 2.0f;
        float num4 =  quat.x * num;
        float num5 =  quat.y * num2;
        float num6 =  quat.z * num3;
        float num7 =  quat.x * num2;
        float num8 =  quat.x * num3;
        float num9 =  quat.y * num3;
        float num10 = quat.w * num;
        float num11 = quat.w * num2;
        float num12 = quat.w * num3;
        Vector3<T> result;
        result.x = (1.0f - (num5 + num6)) * vec.x + (num7 - num12) * vec.y + (num8 + num11) * vec.z;
        result.y = (num7 + num12) * vec.x + (1.0f - (num4 + num6)) * vec.y + (num9 - num10) * vec.z;
        result.z = (num8 - num11) * vec.x + (num9 + num10) * vec.y + (1.0f - (num4 + num5)) * vec.z;
        return result;
    }

    using Quaternionf = Quaternion<float>;
    using Quaterniond = Quaternion<double>;
}
