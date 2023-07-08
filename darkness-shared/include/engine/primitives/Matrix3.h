#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "Math.h"

#include "containers/memory.h"

namespace engine
{
    template<class T>
    class Matrix3
    {
    public:
        T m00; T m01; T m02;
        T m10; T m11; T m12;
        T m20; T m21; T m22;

        Matrix3()
            : m00{ (T)1 }, m01{ (T)0 }, m02{ (T)0 }
            , m10{ (T)0 }, m11{ (T)1 }, m12{ (T)0 }
            , m20{ (T)0 }, m21{ (T)0 }, m22{ (T)1 }
        {
        };

        Matrix3(const T& a, const T& b, const T& c,
            const T& d, const T& e, const T& f,
            const T& g, const T& h, const T& i)
            : m00{ a }, m01{ b }, m02{ c }
            , m10{ d }, m11{ e }, m12{ f }
            , m20{ g }, m21{ h }, m22{ i }
        {
        };

        Vector4<T> toAxisAngles() const
        {
            Vector4<T> result;

            T epsilon = (T)0.01; // margin to allow for rounding errors
            T epsilon2 = (T)0.1; // margin to distinguish between 0 and 180 degrees
                                 // optional check that input is pure rotation, 'isRotationMatrix' is defined at:
                                 // http://www.euclideanspace.com/maths/algebra/matrix/orthogonal/rotation/

                                 //assert isRotationMatrix(m) : "not valid rotation matrix";// for debugging

            if (((T)std::fabs((double)m00 - (double)m10)< epsilon)
                && ((T)std::fabs((double)m02 - (double)m20)< epsilon)
                && ((T)std::fabs((double)m12 - (double)m21)< epsilon))
            {
                // singularity found
                // first check for identity matrix which must have +1 for all terms
                //  in leading diagonaland zero in other terms
                if (((T)std::fabs((double)m00 + (double)m10) < epsilon2)
                    && ((T)std::fabs((double)m02 + (double)m20) < epsilon2)
                    && ((T)std::fabs((double)m12 + (double)m21) < epsilon2)
                    && ((T)std::fabs((double)m00 + (double)m11 + (double)m22 - (double)3) < epsilon2)) {
                    // this singularity is identity matrix so angle = 0
                    return{ (T)1, (T)0, (T)0, (T)0 }; // zero angle, arbitrary axis
                }
                // otherwise this singularity is angle = 180
                result.w = (T)M_PIl;
                T xx = (m00 + (T)1) / (T)2;
                T yy = (m11 + (T)1) / (T)2;
                T zz = (m22 + (T)1) / (T)2;
                T xy = (m00 + m10) / (T)4;
                T xz = (m02 + m20) / (T)4;
                T yz = (m12 + m21) / (T)4;
                if ((xx > yy) && (xx > zz)) { // m00 is the largest diagonal term
                    if (xx< epsilon) {
                        result.x = (T)0;
                        result.y = (T)0.7071;
                        result.z = (T)0.7071;
                    }
                    else {
                        result.x = (T)std::sqrt((double)xx);
                        result.y = xy / result.x;
                        result.z = xz / result.x;
                    }
                }
                else if (yy > zz) { // m11 is the largest diagonal term
                    if (yy< epsilon) {
                        result.x = (T)0.7071;
                        result.y = (T)0;
                        result.z = (T)0.7071;
                    }
                    else {
                        result.y = (T)std::sqrt((double)yy);
                        result.x = xy / result.y;
                        result.z = yz / result.y;
                    }
                }
                else { // m22 is the largest diagonal term so base result on this
                    if (zz< epsilon) {
                        result.x = (T)0.7071;
                        result.y = (T)0.7071;
                        result.z = (T)0;
                    }
                    else {
                        result.z = (T)std::sqrt((double)zz);
                        result.x = xz / result.z;
                        result.y = yz / result.z;
                    }
                }
                return result; // return 180 deg rotation
            }
            // as we have reached here there are no singularities so we can handle normally
            T s = (T)std::sqrt(((double)m21 - (double)m12)*((double)m21 - (double)m12)
                + ((double)m02 - (double)m20)*((double)m02 - (double)m20)
                + ((double)m10 - (double)m00)*((double)m10 - (double)m00)); // used to normalise
            if ((T)std::fabs(s) < (T)0.001) s = (T)1;
            // prevent divide by zero, should not happen if matrix is orthogonal and should be
            // caught by singularity test above, but I've left it in just in case
            result.w = (T)std::acos(((double)m00 + (double)m11 + (double)m22 - (double)1) / (double)2);
            result.x = (T)(m21 - m12) / s;
            result.y = (T)(m02 - m20) / s;
            result.z = (T)(m10 - m00) / s;
            return result;
        }

        Vector3<T> toEulerAngles() const
        {
            // x = heading
            // y = attitude
            // z = bank

            Vector3<T> result;
            // Assuming the angles are in radians.
            if (m10 > (T)0.998) { // singularity at north pole
                result.x = (T)std::atan2((double)m02, (double)m22);
                result.y = (T)M_PIl / (T)2;
                result.z = (T)0;
                return result;
            }
            if (m10 < (T)-0.998) { // singularity at south pole
                result.x = (T)std::atan2((double)m02, (double)m22);
                result.y = -(T)M_PIl / (T)2;
                result.z = (T)0;
                return result;
            }
            result.x = (T)std::atan2((double)-m20, (double)m00);
            result.z = (T)std::atan2((double)-m12, (double)m11);
            result.y = (T)std::asin((double)m10);
            return result;
        }

        Vector4<T> toQuaternion() const
        {
            T w = (T)(std::sqrt((double)1.0 + (double)m00 + (double)m11 + (double)m22) / 2.0);
            double w4 = (4.0 * (double)w);
            T x = (T)(((double)m21 - (double)m12) / w4);
            T y = (T)(((double)m02 - (double)m20) / w4);
            T z = (T)(((double)m10 - (double)m01) / w4);
            return{ x, y, z, w };
        }

        Matrix3& fromAxisAngles(T x, T y, T z, T w)
        {
            T c = (T)std::cos((double)w);
            T s = (T)std::sin((double)w);
            T t = (T)1.0 - c;
            //  normalize
            T magnitude = (T)std::sqrt((double)x * (double)x + (double)y * (double)y + (double)z * (double)z);
            x /= magnitude;
            y /= magnitude;
            z /= magnitude;

            m00 = c + x * x * t;
            m11 = c + y * y * t;
            m22 = c + z * z * t;

            T tmp1 = x * y * t;
            T tmp2 = z * s;
            m10 = tmp1 + tmp2;
            m01 = tmp1 - tmp2;
            tmp1 = x * z * t;
            tmp2 = y * s;
            m20 = tmp1 - tmp2;
            m02 = tmp1 + tmp2;
            tmp1 = y * z * t;
            tmp2 = x * s;
            m21 = tmp1 + tmp2;
            m12 = tmp1 - tmp2;

            return *this;
        }

        Matrix3& fromEulerAngles(T x, T y, T z)
        {
            // Assuming the angles are in radians.
            double ch = std::cos((double)x);
            double sh = std::sin((double)x);
            double ca = std::cos((double)y);
            double sa = std::sin((double)y);
            double cb = std::cos((double)z);
            double sb = std::sin((double)z);

            m00 = (T)(ch * ca);
            m01 = (T)(sh*sb - ch*sa*cb);
            m02 = (T)(ch*sa*sb + sh*cb);
            m10 = (T)(sa);
            m11 = (T)(ca*cb);
            m12 = (T)(-ca*sb);
            m20 = (T)(-sh*ca);
            m21 = (T)(sh*sa*cb + ch*sb);
            m22 = (T)(-sh*sa*sb + ch*cb);
            return *this;
        }

        Matrix3& fromQuaternion(const Vector4<T>& quaternion)
        {
            T sqx = quaternion.x * quaternion.x;
            T sqy = quaternion.y * quaternion.y;
            T sqz = quaternion.z * quaternion.z;
            T sqw = quaternion.w * quaternion.w;

            // invs (inverse square length) is only required if quaternion is not already normalised
            T invs = (T)1 / (sqx + sqy + sqz + sqw);
            m00 = (sqx - sqy - sqz + sqw) * invs; // since sqw + sqx + sqy + sqz =1/invs*invs
            m11 = (-sqx + sqy - sqz + sqw) * invs;
            m22 = (-sqx - sqy + sqz + sqw) * invs;

            T tmp1 = quaternion.x * quaternion.y;
            T tmp2 = quaternion.z * quaternion.w;
            m10 = (T)2.0 * (tmp1 + tmp2) * invs;
            m01 = (T)2.0 * (tmp1 - tmp2) * invs;

            tmp1 = quaternion.x * quaternion.z;
            tmp2 = quaternion.y * quaternion.w;
            m20 = (T)2.0 * (tmp1 - tmp2) * invs;
            m02 = (T)2.0 * (tmp1 + tmp2) * invs;
            tmp1 = quaternion.y * quaternion.z;
            tmp2 = quaternion.x * quaternion.w;
            m21 = (T)2.0 * (tmp1 + tmp2) * invs;
            m12 = (T)2.0 * (tmp1 - tmp2) * invs;

            return *this;
        }

        Matrix3(const Matrix3<T>& mat)
            : m00{ mat.m00 }, m01{ mat.m01 }, m02{ mat.m02 }
            , m10{ mat.m10 }, m11{ mat.m11 }, m12{ mat.m12 }
            , m20{ mat.m20 }, m21{ mat.m21 }, m22{ mat.m22 }
        {};

		Matrix3& operator=(const Matrix3<T>& mat)
		{
            m00 = mat.m00; m01 = mat.m01; m02 = mat.m02;
            m10 = mat.m10; m11 = mat.m11; m12 = mat.m12;
            m20 = mat.m20; m21 = mat.m21; m22 = mat.m22;
            return *this;
        }

        static Matrix3<T> identity()
        {
            return{ (T)1, (T)0, (T)0,
                (T)0, (T)1, (T)0,
                (T)0, (T)0, (T)1 };
        }

        static Matrix3<T> rotate(const T& x, const T& y, const T& z)
        {
            T a = (T)cos((T)x * DEG_TO_RAD);
            T b = (T)sin((T)x * DEG_TO_RAD);

            T c = (T)cos((T)y * DEG_TO_RAD);
            T d = (T)sin((T)y * DEG_TO_RAD);

            T e = (T)cos((T)z * DEG_TO_RAD);
            T f = (T)sin((T)z * DEG_TO_RAD);

            T ad = a * d;
            T bd = b * d;

            return Matrix3<T>(
                c * e,          // 0
                -c * f,         // 1
                d,              // 2
                (T)0,           // 3

                bd * e + a * f, // 4
                -bd * f + a * e,// 5
                -b * c,         // 6
                (T)0,           // 7

                -ad * e + b * f,// 8
                ad * f + b * e, // 9
                a * c,          // 10
                (T)0,           // 11

                (T)0,           // 12
                (T)0,           // 13
                (T)0,           // 14
                (T)1            // 15
                );
        }

        const T& operator[](int index) const
        {
            return *(&m00 + index);
        }

        Matrix3<T> operator* (const Matrix3<T> mat)
        {
            return{
                (m00 * mat.m00) + (m01 * mat.m10) + (m02 * mat.m20),
                (m00 * mat.m01) + (m01 * mat.m11) + (m02 * mat.m21),
                (m00 * mat.m02) + (m01 * mat.m12) + (m02 * mat.m22),

                (m10 * mat.m00) + (m11 * mat.m10) + (m12 * mat.m20),
                (m10 * mat.m01) + (m11 * mat.m11) + (m12 * mat.m21),
                (m10 * mat.m02) + (m11 * mat.m12) + (m12 * mat.m22),

                (m20 * mat.m00) + (m21 * mat.m10) + (m22 * mat.m20),
                (m20 * mat.m01) + (m21 * mat.m11) + (m22 * mat.m21),
                (m20 * mat.m02) + (m21 * mat.m12) + (m22 * mat.m22)
            };
        }

		T determinant() const
		{
			return m00 * m11*m22 + m01 * m12*m20 + m02 * m10*m21 - m00 * m12*m21 - m01 * m10*m22 - m02 * m11*m20;
		}

		Matrix3<T> inverse() const
		{
			Matrix3<T> result;

			T det =
				m00 * m11 * m22 +
				m01 * m12 * m20 +
				m02 * m10 * m21 -
				m00 * m12 * m21 -
				m01 * m10 * m22 -
				m02 * m11 * m20;

			result.m00 = (m11 * m22 - m12 * m21) / det;
			result.m01 = (m02 * m21 - m01 * m22) / det;
			result.m02 = (m01 * m12 - m02 * m11) / det;
			result.m10 = (m12 * m20 - m10 * m22) / det;
			result.m11 = (m00 * m22 - m02 * m20) / det;
			result.m12 = (m02 * m10 - m00 * m12) / det;
			result.m20 = (m10 * m21 - m11 * m20) / det;
			result.m21 = (m01 * m20 - m00 * m21) / det;
			result.m22 = (m00 * m11 - m01 * m10) / det;

			return result;
		}

		Matrix3<T> transpose() const
		{
			return {
				m00, m10, m20,
				m01, m11, m21,
				m02, m12, m22
			};
		}

		bool orthonormal(T limit = static_cast<T>(0.00001)) const
		{
			auto invtr = inverse().transpose();
			return
				(fabs(m00 - invtr.m00) < limit) &&
				(fabs(m01 - invtr.m01) < limit) &&
				(fabs(m02 - invtr.m02) < limit) &&

				(fabs(m10 - invtr.m10) < limit) &&
				(fabs(m11 - invtr.m11) < limit) &&
				(fabs(m12 - invtr.m12) < limit) &&

				(fabs(m20 - invtr.m20) < limit) &&
				(fabs(m21 - invtr.m21) < limit) &&
				(fabs(m22 - invtr.m22) < limit);
		}
    };

    using Matrix3f = Matrix3<float>;
    using Matrix3d = Matrix3<double>;
}
