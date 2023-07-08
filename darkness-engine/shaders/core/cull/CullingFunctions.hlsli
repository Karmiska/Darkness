
bool frustumCull(
    float3 bbmin, float3 bbmax, 
    float4x4 transform, 
    float3 cameraPos,
    float farPlaneDistance,
    float3 plane0,
    float3 plane1, 
    float3 plane2, 
    float3 plane3, 
    float3 plane4, 
    float3 plane5)
{
    float3 corner[8] =
    {
        mul(transform, float4(bbmin.xyz, 1.0f)).xyz,
        mul(transform, float4(bbmin.x, bbmax.y, bbmax.z, 1.0f)).xyz,
        mul(transform, float4(bbmin.x, bbmin.y, bbmax.z, 1.0f)).xyz,
        mul(transform, float4(bbmin.x, bbmax.y, bbmin.z, 1.0f)).xyz,
        mul(transform, float4(bbmax, 1.0f)).xyz,
        mul(transform, float4(bbmax.x, bbmin.y, bbmin.z, 1.0f)).xyz,
        mul(transform, float4(bbmax.x, bbmax.y, bbmin.z, 1.0f)).xyz,
        mul(transform, float4(bbmax.x, bbmin.y, bbmax.z, 1.0f)).xyz
    };

    float3 frustumPlanes[6] =
    {
        plane0.xyz,
        plane1.xyz,
        plane2.xyz,
        plane3.xyz,
        plane4.xyz,
        plane5.xyz
    };

    for (int i = 0; i < 5; ++i)
    {
        bool hit = false;
        for (int c = 0; c < 8; ++c)
        {
            if (dot(frustumPlanes[i], (corner[c] - cameraPos)) >= 0)
            {
                hit = true;
                break;
            }
        }
        if (!hit)
            return false;
    }

    bool onePointCloseCamera = false;
    for (int c = 0; c < 8; ++c)
    {
        if (length(corner[c] - cameraPos) < farPlaneDistance)
        {
            onePointCloseCamera = true;
            break;
        }
    }

    return onePointCloseCamera;
}

bool frustumCullNoTransform(
    float3 bbmin, float3 bbmax,
    float3 cameraPos,
    float farPlaneDistance,
    float3 plane0,
    float3 plane1,
    float3 plane2,
    float3 plane3,
    float3 plane4,
    float3 plane5)
{
    float3 corner[8] =
    {
        bbmin.xyz,
        float3(bbmin.x, bbmax.y, bbmax.z),
        float3(bbmin.x, bbmin.y, bbmax.z),
        float3(bbmin.x, bbmax.y, bbmin.z),
        bbmax.xyz,
        float3(bbmax.x, bbmin.y, bbmin.z),
        float3(bbmax.x, bbmax.y, bbmin.z),
        float3(bbmax.x, bbmin.y, bbmax.z)
    };

    float3 frustumPlanes[6] =
    {
        plane0.xyz,
        plane1.xyz,
        plane2.xyz,
        plane3.xyz,
        plane4.xyz,
        plane5.xyz
    };

    for (int i = 0; i < 5; ++i)
    {
        bool hit = false;
        for (int c = 0; c < 8; ++c)
        {
            if (dot(frustumPlanes[i], (corner[c] - cameraPos)) >= 0)
            {
                hit = true;
                break;
            }
        }
        if (!hit)
            return false;
    }

    bool onePointCloseCamera = false;
    for (int c = 0; c < 8; ++c)
    {
        if (length(corner[c] - cameraPos) < farPlaneDistance)
        {
            onePointCloseCamera = true;
            break;
        }
    }

    return onePointCloseCamera;
}

bool coneCull(
    float4x4 transform,
    float4 cone,
    float4 camPosition,
    float3 bbmin, float3 bbmax)
{
    return true;
    float d = length(float3(transform[0][0], transform[1][0], transform[2][0]));
    float h = length(float3(transform[0][1], transform[1][1], transform[2][1]));
    float l = length(float3(transform[0][2], transform[1][2], transform[2][2]));
    if (abs(d - h) < 0.0001 &&
        abs(h - l) < 0.0001 &&
        abs(d - l) < 0.0001)
    {
        float3 corner[8] =
        {
            mul(transform, float4(bbmin.xyz, 1.0f)).xyz,
            mul(transform, float4(bbmin.x, bbmax.y, bbmax.z, 1.0f)).xyz,
            mul(transform, float4(bbmin.x, bbmin.y, bbmax.z, 1.0f)).xyz,
            mul(transform, float4(bbmin.x, bbmax.y, bbmin.z, 1.0f)).xyz,
            mul(transform, float4(bbmax, 1.0f)).xyz,
            mul(transform, float4(bbmax.x, bbmin.y, bbmin.z, 1.0f)).xyz,
            mul(transform, float4(bbmax.x, bbmax.y, bbmin.z, 1.0f)).xyz,
            mul(transform, float4(bbmax.x, bbmin.y, bbmax.z, 1.0f)).xyz
        };

        bool onePasses = false;
        for (int i = 0; i < 8; ++i)
        {
            float3 camDir = normalize(corner[i] - camPosition.xyz);

            float3 cd = mul(transform, float4(cone.xyz, 1.0f)).xyz;
            cd -= float3(transform[0][3], transform[1][3], transform[2][3]);
            onePasses = dot(normalize(cd), camDir.xyz) < cone.w;
            if (onePasses)
                break;
        }
        return onePasses;
    }
    else
        return true;
}

uint fnv1aHash(
    uint clusterPointer, 
    uint instancePointer,
    uint fnvOffsetBasis,
    uint fnvPrime)
{
    uint hash = fnvOffsetBasis;
    hash ^= (clusterPointer & 0xff);
    hash *= fnvPrime;
    hash ^= (clusterPointer & 0xff00) >> 8;
    hash *= fnvPrime;
    hash ^= (clusterPointer & 0xff0000) >> 16;
    hash *= fnvPrime;
    hash ^= (clusterPointer & 0xff000000) >> 24;
    hash *= fnvPrime;

    hash ^= (instancePointer & 0xff);
    hash *= fnvPrime;
    hash ^= (instancePointer & 0xff00) >> 8;
    hash *= fnvPrime;
    hash ^= (instancePointer & 0xff0000) >> 16;
    hash *= fnvPrime;
    hash ^= (instancePointer & 0xff000000) >> 24;
    hash *= fnvPrime;
    return hash;
}
