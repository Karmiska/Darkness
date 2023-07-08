#include "ShaderTypes.h"

namespace engine
{
    namespace shaders
    {
        uint64_t fnv1aHash(uint64_t value, uint64_t hash)
        {
            hash ^= (value & 0xff);
            hash *= FnvPrime;
            hash ^= (value & 0xff00) >> 8;
            hash *= FnvPrime;
            hash ^= (value & 0xff0000) >> 16;
            hash *= FnvPrime;
            hash ^= (value & 0xff000000) >> 24;
            hash *= FnvPrime;

            hash ^= (value & 0xff00000000) >> 32;
            hash *= FnvPrime;
            hash ^= (value & 0xff0000000000) >> 40;
            hash *= FnvPrime;
            hash ^= (value & 0xff000000000000) >> 48;
            hash *= FnvPrime;
            hash ^= (value & 0xff00000000000000) >> 56;
            hash *= FnvPrime;
            return hash;
        }
    }

    Float4x4 fromMatrix(Matrix4f mat)
    {
        return {
            mat.m00, mat.m01, mat.m02, mat.m03,
            mat.m10, mat.m11, mat.m12, mat.m13,
            mat.m20, mat.m21, mat.m22, mat.m23,
            mat.m30, mat.m31, mat.m32, mat.m33
        };

        //return {
        //    mat.m00, mat.m10, mat.m20, mat.m30,
        //    mat.m01, mat.m11, mat.m21, mat.m31,
        //    mat.m02, mat.m12, mat.m22, mat.m32,
        //    mat.m03, mat.m13, mat.m23, mat.m33
        //};
    };
}
