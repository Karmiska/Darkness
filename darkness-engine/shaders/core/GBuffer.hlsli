
// For each component of v, returns -1 if the component is < 0, else 1
/*float2 sign_not_zero(float2 v)
{
    return float2(
        v.x >= 0 ? 1.0 : -1.0,
        v.y >= 0 ? 1.0 : -1.0
    );
};

// Packs a 3-component normal to 2 channels using octahedron normals
float2 packNormalOctahedron(float3 v)
{
    float2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
    // Reflect the folds of the lower hemisphere over the diagonals
    return (v.z <= 0.0) ? ((1.0 - abs(p.yx))  * sign_not_zero(p)) : p;
};

// Unpacking from octahedron normals, input is the output from pack_normal_octahedron
float3 unpackNormalOctahedron(float2 packed_nrm)
{
    float3 v = float3(packed_nrm.xy, 1.0 - abs(packed_nrm.x) - abs(packed_nrm.y));
    if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);

    return normalize(v);
};*/

float2 octWrap(float2 v)
{
    return (1.0 - abs(v.yx)) * (v.xy >= 0.0 ? 1.0 : -1.0);
}

float2 packNormalOctahedron(float3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : octWrap(n.xy);
    n.xy = n.xy * 0.5 + 0.5;
    return n.xy;
};

float3 unpackNormalOctahedron(float2 f)
{
    f = f * 2.0 - 1.0;
    float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = saturate(-n.z);
    n.xy += n.xy >= 0.0 ? -t : t;
    return normalize(n);
};

// motion data
// has 10 bits accuracy
// original range: -512  <->  + 511

uint2 packUV(float2 uv)
{
    return uint2(uv.x * 65535.0, uv.y * 65535.0);
};

float2 unpackUV(uint2 uv)
{
    return float2((float)uv.x / 65535.0, (float)uv.y / 65535.0);
};

/*uint packMotion(float motion)
{
    float b = 0.5f * (motion + 1.f);
    uint c = min(((uint)round(b * 0x400)), 0x3ff);
    return c;
};

float unpackMotion(uint motion)
{
    float b = saturate((float)motion / (float)0x400);
    float a = b * 2.f - 1.f;
    return a;
};

uint4 packUvMotionX(float2 uv, float motionX)
{
    // we want 11 bits for uv channels
    // so we're using alpha 2 bits as extra bits for uv
    // 11 bits has 2048 values
    uint uInt = uv.x * 2047.0f;
    uint vInt = uv.y * 2047.0f;

    uint u10lowbits = uInt & 0x3ff;
    uint u1highbit = uInt >> 10;
    uint v10lowbits = vInt & 0x3ff;
    uint v1highbit = vInt >> 10;

    uint4 res;
    res.x = u10lowbits;
    res.y = v10lowbits;
    res.z = packMotion(motionX);
    res.w = (u1highbit << 1) | v1highbit;
    return res;
};

uint4 packObjectIdMotionY(uint objectId, float motionY)
{
    uint high10bits = (objectId & 0x3ff000) >> 12;
    uint middle10bits = (objectId & 0xffc) >> 2;
    uint low2bits = (objectId & 0x3);
    uint motion = packMotion(motionY);
    return uint4(high10bits, middle10bits, motion, low2bits);
};

float unpackMotionX(uint4 data)
{
    return unpackMotion(data.z);
};

float unpackMotionY(uint4 data)
{
    return unpackMotion(data.z);
};

float2 unpackUV(uint4 data)
{
    return float2(
        (float)(data.x | ((data.w & 0x2) << 9)) / 2047.0f,
        (float)(data.y | ((data.w & 0x1) << 10)) / 2047.0f);
};

uint unpackObjectId(uint4 data)
{
    return (data.x << 12) | (data.y << 2) | data.w;
};*/

