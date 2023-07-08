#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "Matrix3.h"
#include "Math.h"

#include "containers/memory.h"

namespace engine
{
    template<class T>
    class Matrix4
    {
    public:
        T m00; T m01; T m02; T m03;
        T m10; T m11; T m12; T m13;
        T m20; T m21; T m22; T m23;
        T m30; T m31; T m32; T m33;

        Matrix4()
            : m00{ (T)0 }, m01{ (T)0 }, m02{ (T)0 }, m03{ (T)0 }
            , m10{ (T)0 }, m11{ (T)0 }, m12{ (T)0 }, m13{ (T)0 }
            , m20{ (T)0 }, m21{ (T)0 }, m22{ (T)0 }, m23{ (T)0 }
            , m30{ (T)0 }, m31{ (T)0 }, m32{ (T)0 }, m33{ (T)0 }
        {
        };

        Matrix4(const T& a, const T& b, const T& c, const T& d,
                const T& e, const T& f, const T& g, const T& h, 
                const T& i, const T& j, const T& k, const T& l, 
                const T& m, const T& n, const T& o, const T& p)
            : m00{ a }, m01{ b }, m02{ c }, m03{ d }
            , m10{ e }, m11{ f }, m12{ g }, m13{ h }
            , m20{ i }, m21{ j }, m22{ k }, m23{ l }
            , m30{ m }, m31{ n }, m32{ o }, m33{ p }
        {
        };

        Vector4<T> operator*(const Vector4<T> vec) const
        {
            return {
                m00*vec.x + m01*vec.y + m02*vec.z + m03*vec.w,
                m10*vec.x + m11*vec.y + m12*vec.z + m13*vec.w,
                m20*vec.x + m21*vec.y + m22*vec.z + m23*vec.w,
                m30*vec.x + m31*vec.y + m32*vec.z + m33*vec.w
            };

            /*return {
                m00*vec.x + m10*vec.y + m20*vec.z + m30*vec.w,
                m01*vec.x + m11*vec.y + m21*vec.z + m31*vec.w,
                m02*vec.x + m12*vec.y + m22*vec.z + m32*vec.w,
                m03*vec.x + m13*vec.y + m23*vec.z + m33*vec.w
            };*/
        }

        Vector3<T> operator*(const Vector3<T> vec) const
        {
            /*return {
                m00*vec.x + m01*vec.y + m02*vec.z,
                m10*vec.x + m11*vec.y + m12*vec.z,
                m20*vec.x + m21*vec.y + m22*vec.z
            };*/
            return (*this * Vector4f(vec, 1.0f)).xyz();
            /*return {
                m00*vec.x + m10*vec.y + m20*vec.z,
                m01*vec.x + m11*vec.y + m21*vec.z,
                m02*vec.x + m12*vec.y + m22*vec.z
            };*/
        }

        /*Vector4<T> toAxisAngles() const
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
        }*/

		// this function has been tested and verified to work correctly!
		// TRUST IT!
        Vector3<T> toEulerAngles() const
        {
            Vector3<T> result;

			// Assuming the angles are in radians.
            if (m10 > (T)0.998) { // singularity at north pole
                result.y = (T)std::atan2((double)m02, (double)m22);
				result.z = (T)M_PIl / (T)2;
				result.x = (T)0;
                return result;
            }
            if (m10 < (T)-0.998) { // singularity at south pole
				result.y = (T)std::atan2((double)m02, (double)m22);
				result.z = -(T)M_PIl / (T)2;
				result.x = (T)0;
                return result;
            }
			result.y = (T)std::atan2((double)-m20, (double)m00);
			result.x = (T)std::atan2((double)-m12, (double)m11);
			result.z = (T)std::asin((double)m10);
			return { RAD_TO_DEG * result.x, RAD_TO_DEG * result.y, RAD_TO_DEG * result.z };
        }

        Vector4<T> toQuaternion() const
        {
            /*w = (T)(std::sqrt((double)1.0 + (double)m00 + (double)m11 + (double)m22) / 2.0);
            double w4 = (4.0 * (double)w);
            x = (T)(((double)m21 - (double)m12) / w4);
            y = (T)(((double)m02 - (double)m20) / w4);
            z = (T)(((double)m10 - (double)m01) / w4);
            return{ x, y, z, w };*/

			Vector4<T> res;
			T trace = m00 + m11 + m22; // I removed + 1.0f; see discussion with Ethan
			if (trace > 0) {// I changed M_EPSILON to 0
				T s = 0.5f / std::sqrtf(trace + 1.0f);
				res.w = 0.25f / s;
				res.x = (m21 - m12) * s;
				res.y = (m02 - m20) * s;
				res.z = (m10 - m01) * s;
			}
			else {
				if (m00 > m11 && m00 > m22) {
					T s = 2.0f * sqrtf(1.0f + m00 - m11 - m22);
					res.w = (m21 - m12) / s;
					res.x = 0.25f * s;
					res.y = (m01 + m10) / s;
					res.z = (m02 + m20) / s;
				}
				else if (m11 > m22) {
					T s = 2.0f * sqrtf(1.0f + m11 - m00 - m22);
					res.w = (m02 - m20) / s;
					res.x = (m01 + m10) / s;
					res.y = 0.25f * s;
					res.z = (m12 + m21) / s;
				}
				else {
					T s = 2.0f * sqrtf(1.0f + m22 - m00 - m11);
					res.w = (m10 - m01) / s;
					res.x = (m02 + m20) / s;
					res.y = (m12 + m21) / s;
					res.z = 0.25f * s;
				}
			}
			return res;
        }

        /*static Matrix4 fromAxisAngles(T x, T y, T z, T w)
        {
            Matrix4<T> mat;
            T c = (T)std::cos((double)w);
            T s = (T)std::sin((double)w);
            T t = (T)1.0 - c;
            //  normalize
            auto normalized = Vector3<T>(x, y, z).normalize();

            mat.m00 = c + normalized.x * normalized.x * t;
            mat.m11 = c + normalized.y * normalized.y * t;
            mat.m22 = c + normalized.z * normalized.z * t;

            T tmp1 = normalized.x * normalized.y * t;
            T tmp2 = normalized.z * s;
            mat.m10 = tmp1 + tmp2;
            mat.m01 = tmp1 - tmp2;
            tmp1 = normalized.x * normalized.z * t;
            tmp2 = normalized.y * s;
            mat.m20 = tmp1 - tmp2;
            mat.m02 = tmp1 + tmp2;
            tmp1 = normalized.y * normalized.z * t;
            tmp2 = normalized.x * s;
            mat.m21 = tmp1 + tmp2;
            mat.m12 = tmp1 - tmp2;

            return mat;
        }*/

        static Matrix4<T> fromEulerAngles(T x, T y, T z)
        {
			return Matrix4<T>::rotation(x, y, z);
        }

        static Matrix4<T> fromQuaternion(const Vector4<T>& quaternion)
        {
			Matrix4<T> res;

			T sqw = quaternion.w*quaternion.w;
			T sqx = quaternion.x*quaternion.x;
			T sqy = quaternion.y*quaternion.y;
			T sqz = quaternion.z*quaternion.z;

			// invs (inverse square length) is only required if quaternion is not already normalised
			T invs = static_cast<T>(1) / (sqx + sqy + sqz + sqw);
			res.m00 = (sqx - sqy - sqz + sqw)*invs; // since sqw + sqx + sqy + sqz =1/invs*invs
			res.m11 = (-sqx + sqy - sqz + sqw)*invs;
			res.m22 = (-sqx - sqy + sqz + sqw)*invs;

			T tmp1 = quaternion.x*quaternion.y;
			T tmp2 = quaternion.z*quaternion.w;
			res.m10 = 2.0 * (tmp1 + tmp2)*invs;
			res.m01 = 2.0 * (tmp1 - tmp2)*invs;

			tmp1 = quaternion.x*quaternion.z;
			tmp2 = quaternion.y*quaternion.w;
			res.m20 = 2.0 * (tmp1 - tmp2)*invs;
			res.m02 = 2.0 * (tmp1 + tmp2)*invs;
			tmp1 = quaternion.y*quaternion.z;
			tmp2 = quaternion.x*quaternion.w;
			res.m21 = 2.0 * (tmp1 + tmp2)*invs;
			res.m12 = 2.0 * (tmp1 - tmp2)*invs;

			res.m03 = static_cast<T>(0);
			res.m13 = static_cast<T>(0);
			res.m23 = static_cast<T>(0);
			res.m30 = static_cast<T>(0);
			res.m31 = static_cast<T>(0);
			res.m32 = static_cast<T>(0);
			res.m33 = static_cast<T>(1);

			return res;

            /*T sqx = quaternion.x * quaternion.x;
            T sqy = quaternion.y * quaternion.y;
            T sqz = quaternion.z * quaternion.z;
            T sqw = quaternion.w * quaternion.w;

            Matrix4<T> mat;

            // invs (inverse square length) is only required if quaternion is not already normalised
            T invs = (T)1 / (sqx + sqy + sqz + sqw);
            mat.m00 = (sqx - sqy - sqz + sqw) * invs; // since sqw + sqx + sqy + sqz =1/invs*invs
            mat.m11 = (-sqx + sqy - sqz + sqw) * invs;
            mat.m22 = (-sqx - sqy + sqz + sqw) * invs;

            T tmp1 = quaternion.x * quaternion.y;
            T tmp2 = quaternion.z * quaternion.w;
            mat.m10 = (T)2.0 * (tmp1 + tmp2) * invs;
            mat.m01 = (T)2.0 * (tmp1 - tmp2) * invs;

            tmp1 = quaternion.x * quaternion.z;
            tmp2 = quaternion.y * quaternion.w;
            mat.m20 = (T)2.0 * (tmp1 - tmp2) * invs;
            mat.m02 = (T)2.0 * (tmp1 + tmp2) * invs;
            tmp1 = quaternion.y * quaternion.z;
            tmp2 = quaternion.x * quaternion.w;
            mat.m21 = (T)2.0 * (tmp1 + tmp2) * invs;
            mat.m12 = (T)2.0 * (tmp1 - tmp2) * invs;

            return mat;*/
        }

        Matrix4(const Matrix4<T>& mat)
            : m00{ mat.m00 }, m01{ mat.m01 }, m02{ mat.m02 }, m03{ mat.m03 }
            , m10{ mat.m10 }, m11{ mat.m11 }, m12{ mat.m12 }, m13{ mat.m13 }
            , m20{ mat.m20 }, m21{ mat.m21 }, m22{ mat.m22 }, m23{ mat.m23 }
            , m30{ mat.m30 }, m31{ mat.m31 }, m32{ mat.m32 }, m33{ mat.m33 }
        {};

		Matrix4& operator=(const Matrix4<T>& mat)
		{
            m00 = mat.m00; m01 = mat.m01; m02 = mat.m02; m03 = mat.m03;
            m10 = mat.m10; m11 = mat.m11; m12 = mat.m12; m13 = mat.m13;
            m20 = mat.m20; m21 = mat.m21; m22 = mat.m22; m23 = mat.m23;
            m30 = mat.m30; m31 = mat.m31; m32 = mat.m32; m33 = mat.m33;
            return *this;
		}

        Matrix4(const Matrix3<T>& mat)
            : m00{ mat.m00 }, m01{ mat.m01 }, m02{ mat.m02 }, m03{ 0 }
            , m10{ mat.m10 }, m11{ mat.m11 }, m12{ mat.m12 }, m13{ 0 }
            , m20{ mat.m20 }, m21{ mat.m21 }, m22{ mat.m22 }, m23{ 0 }
            , m30{ 0 },       m31{ 0 },       m32{ 0 },       m33{ 1 }
        {
        }

        Matrix4<T> transpose()
        {
            return {
                m00, m10, m20, m30,
                m01, m11, m21, m31,
                m02, m12, m22, m32,
                m03, m13, m23, m33
            };
        }

        static Matrix4<T> identity()
        {
            return{(T)1, (T)0, (T)0, (T)0,
                   (T)0, (T)1, (T)0, (T)0,
                   (T)0, (T)0, (T)1, (T)0,
                   (T)0, (T)0, (T)0, (T)1};
        }

		static Matrix4<T> translate(const Vector3f& vec)
		{
			return translate(vec.x, vec.y, vec.z);
		}

        static Matrix4<T> translate(const T& x, const T& y, const T& z)
        {
            return {(T)1, (T)0, (T)0, x,
                    (T)0, (T)1, (T)0, y,
                    (T)0, (T)0, (T)1, z,
                    (T)0, (T)0, (T)0, (T)1};
        }

		static Matrix4<T> scale(const Vector3f& vec)
		{
			return scale(vec.x, vec.y, vec.z);
		}

        static Matrix4<T> scale(const T& x, const T& y, const T& z)
        {
            return {   x, (T)0, (T)0, (T)0,
                    (T)0,    y, (T)0, (T)0,
                    (T)0, (T)0,    z, (T)0,
                    (T)0, (T)0, (T)0, (T)1};
        }

		static Matrix4<T> rotation(const Vector3f& vec)
		{
			return rotation(vec.x, vec.y, vec.z);
		}

		// this function has been tested and verified to work correctly!
		// TRUST IT!
        static Matrix4<T> rotation(const T& x, const T& y, const T& z)
        {
			T ch = std::cos(y * DEG_TO_RAD);
			T sh = std::sin(y * DEG_TO_RAD);
			T ca = std::cos(z * DEG_TO_RAD);
			T sa = std::sin(z * DEG_TO_RAD);
			T cb = std::cos(x * DEG_TO_RAD);
			T sb = std::sin(x * DEG_TO_RAD);

			T m00 = ch * ca;
			T m01 = sh*sb - ch*sa*cb;
			T m02 = ch*sa*sb + sh*cb;
			T m03 = static_cast<T>(0);
			
			T m10 = sa;
			T m11 = ca*cb;
			T m12 = -ca*sb;
			T m13 = static_cast<T>(0);
			
			T m20 = -sh*ca;
			T m21 = sh*sa*cb + ch*sb;
			T m22 = -sh*sa*sb + ch*cb;
			T m23 = static_cast<T>(0);

			T m30 = static_cast<T>(0);
			T m31 = static_cast<T>(0);
			T m32 = static_cast<T>(0);
			T m33 = static_cast<T>(1);

			return { 
				m00, m01, m02, m03,
				m10, m11, m12, m13, 
				m20, m21, m22, m23, 
				m30, m31, m32, m33};
        }

        Matrix4<T> inverse() const
        {
            Matrix4<T> result;

            result.m00 = m11 * m22 * m33 -
                         m11 * m23 * m32 -
                         m21 * m12 * m33 +
                         m21 * m13 * m32 +
                         m31 * m12 * m23 -
                         m31 * m13 * m22;

            result.m10 = -m10 * m22 * m33 +
                          m10 * m23 * m32 +
                          m20 * m12 * m33 -
                          m20 * m13 * m32 -
                          m30 * m12 * m23 +
                          m30 * m13 * m22;

            result.m20 = m10 * m21 * m33 -
                         m10 * m23 * m31 -
                         m20 * m11 * m33 +
                         m20 * m13 * m31 +
                         m30 * m11 * m23 -
                         m30 * m13 * m21;

            result.m30 = -m10 * m21 * m32 +
                          m10 * m22 * m31 +
                          m20 * m11 * m32 -
                          m20 * m12 * m31 -
                          m30 * m11 * m22 +
                          m30 * m12 * m21;

            result.m01 = -m01 * m22 * m33 +
                          m01 * m23 * m32 +
                          m21 * m02 * m33 -
                          m21 * m03 * m32 -
                          m31 * m02 * m23 +
                          m31 * m03 * m22;

            result.m11 = m00 * m22 * m33 -
                         m00 * m23 * m32 -
                         m20 * m02 * m33 +
                         m20 * m03 * m32 +
                         m30 * m02 * m23 -
                         m30 * m03 * m22;

            result.m21 = -m00 * m21 * m33 +
                          m00 * m23 * m31 +
                          m20 * m01 * m33 -
                          m20 * m03 * m31 -
                          m30 * m01 * m23 +
                          m30 * m03 * m21;

            result.m31 = m00 * m21 * m32 -
                         m00 * m22 * m31 -
                         m20 * m01 * m32 +
                         m20 * m02 * m31 +
                         m30 * m01 * m22 -
                         m30 * m02 * m21;

            result.m02 = m01 * m12 * m33 -
                         m01 * m13 * m32 -
                         m11 * m02 * m33 +
                         m11 * m03 * m32 +
                         m31 * m02 * m13 -
                         m31 * m03 * m12;

            result.m12 = -m00 * m12 * m33 +
                          m00 * m13 * m32 +
                          m10 * m02 * m33 -
                          m10 * m03 * m32 -
                          m30 * m02 * m13 +
                          m30 * m03 * m12;

            result.m22 = m00 * m11 * m33 -
                         m00 * m13 * m31 -
                         m10 * m01 * m33 +
                         m10 * m03 * m31 +
                         m30 * m01 * m13 -
                         m30 * m03 * m11;

            result.m32 = -m00 * m11 * m32 +
                          m00 * m12 * m31 +
                          m10 * m01 * m32 -
                          m10 * m02 * m31 -
                          m30 * m01 * m12 +
                          m30 * m02 * m11;

            result.m03 = -m01 * m12 * m23 +
                          m01 * m13 * m22 +
                          m11 * m02 * m23 -
                          m11 * m03 * m22 -
                          m21 * m02 * m13 +
                          m21 * m03 * m12;

            result.m13 = m00 * m12 * m23 -
                         m00 * m13 * m22 -
                         m10 * m02 * m23 +
                         m10 * m03 * m22 +
                         m20 * m02 * m13 -
                         m20 * m03 * m12;

            result.m23 = -m00 * m11 * m23 +
                          m00 * m13 * m21 +
                          m10 * m01 * m23 -
                          m10 * m03 * m21 -
                          m20 * m01 * m13 +
                          m20 * m03 * m11;

            result.m33 = m00 * m11 * m22 -
                         m00 * m12 * m21 -
                         m10 * m01 * m22 +
                         m10 * m02 * m21 +
                         m20 * m01 * m12 -
                         m20 * m02 * m11;

            double det = m00 * result.m00 + m01 * result.m10 + m02 * result.m20 + m03 * result.m30;

            if (det == 0)
                return Matrix4<T>();
            det = 1.0 / det;
            result.m00 *= (T)det; result.m01 *= (T)det; result.m02 *= (T)det; result.m03 *= (T)det;
            result.m10 *= (T)det; result.m11 *= (T)det; result.m12 *= (T)det; result.m13 *= (T)det;
            result.m20 *= (T)det; result.m21 *= (T)det; result.m22 *= (T)det; result.m23 *= (T)det;
            result.m30 *= (T)det; result.m31 *= (T)det; result.m32 *= (T)det; result.m33 *= (T)det;
            return result;
        }

        const T& operator[](int index) const
        {
            return *(&m00 + index);
        }

		T determinant() const
		{
			return
				m03 * m12 * m21 * m30 - m02 * m13 * m21 * m30 - m03 * m11 * m22 * m30 + m01 * m13 * m22 * m30 +
				m02 * m11 * m23 * m30 - m01 * m12 * m23 * m30 - m03 * m12 * m20 * m31 + m02 * m13 * m20 * m31 +
				m03 * m10 * m22 * m31 - m00 * m13 * m22 * m31 - m02 * m10 * m23 * m31 + m00 * m12 * m23 * m31 +
				m03 * m11 * m20 * m32 - m01 * m13 * m20 * m32 - m03 * m10 * m21 * m32 + m00 * m13 * m21 * m32 +
				m01 * m10 * m23 * m32 - m00 * m11 * m23 * m32 - m02 * m11 * m20 * m33 + m01 * m12 * m20 * m33 +
				m02 * m10 * m21 * m33 - m00 * m12 * m21 * m33 - m01 * m10 * m22 * m33 + m00 * m11 * m22 * m33;
		}

		Matrix3<T> toMatrix3() const
		{
			return { m00, m01, m02,
				m10, m11, m12,
				m20, m21, m22 };
		}

		bool orthonormal(T limit = static_cast<T>(0.00001)) const
		{
			auto invtr = inverse().transpose();
			return
				(fabs(m00 - invtr.m00) < limit) &&
				(fabs(m01 - invtr.m01) < limit) &&
				(fabs(m02 - invtr.m02) < limit) &&
				(fabs(m03 - invtr.m03) < limit) &&

				(fabs(m10 - invtr.m10) < limit) &&
				(fabs(m11 - invtr.m11) < limit) &&
				(fabs(m12 - invtr.m12) < limit) &&
				(fabs(m13 - invtr.m13) < limit) &&

				(fabs(m20 - invtr.m20) < limit) &&
				(fabs(m21 - invtr.m21) < limit) &&
				(fabs(m22 - invtr.m22) < limit) &&
				(fabs(m23 - invtr.m23) < limit) &&

				(fabs(m30 - invtr.m30) < limit) &&
				(fabs(m31 - invtr.m31) < limit) &&
				(fabs(m32 - invtr.m32) < limit) &&
				(fabs(m33 - invtr.m33) < limit);
		}

        Matrix4<T> operator* (const Matrix4<T>& mat) const
        {
            return {
                (m00 * mat.m00) + (m01 * mat.m10) + (m02 * mat.m20) + (m03 * mat.m30),
                (m00 * mat.m01) + (m01 * mat.m11) + (m02 * mat.m21) + (m03 * mat.m31),
                (m00 * mat.m02) + (m01 * mat.m12) + (m02 * mat.m22) + (m03 * mat.m32),
                (m00 * mat.m03) + (m01 * mat.m13) + (m02 * mat.m23) + (m03 * mat.m33),

                (m10 * mat.m00) + (m11 * mat.m10) + (m12 * mat.m20) + (m13 * mat.m30),
                (m10 * mat.m01) + (m11 * mat.m11) + (m12 * mat.m21) + (m13 * mat.m31),
                (m10 * mat.m02) + (m11 * mat.m12) + (m12 * mat.m22) + (m13 * mat.m32),
                (m10 * mat.m03) + (m11 * mat.m13) + (m12 * mat.m23) + (m13 * mat.m33),

                (m20 * mat.m00) + (m21 * mat.m10) + (m22 * mat.m20) + (m23 * mat.m30),
                (m20 * mat.m01) + (m21 * mat.m11) + (m22 * mat.m21) + (m23 * mat.m31),
                (m20 * mat.m02) + (m21 * mat.m12) + (m22 * mat.m22) + (m23 * mat.m32),
                (m20 * mat.m03) + (m21 * mat.m13) + (m22 * mat.m23) + (m23 * mat.m33),

                (m30 * mat.m00) + (m31 * mat.m10) + (m32 * mat.m20) + (m33 * mat.m30),
                (m30 * mat.m01) + (m31 * mat.m11) + (m32 * mat.m21) + (m33 * mat.m31),
                (m30 * mat.m02) + (m31 * mat.m12) + (m32 * mat.m22) + (m33 * mat.m32),
                (m30 * mat.m03) + (m31 * mat.m13) + (m32 * mat.m23) + (m33 * mat.m33)
            };
        }
    };

    using Matrix4f = Matrix4<float>;
    using Matrix4d = Matrix4<double>;
}
