
Texture2D<float> src;
RWTexture2D<float> dst0;
RWTexture2D<float> dst1;
RWTexture2D<float> dst2;
RWTexture2D<float> dst3;

sampler srcSampler;

cbuffer Constants
{
    float2 size;
    float2 invSize;
};

groupshared float dTile[64];

[numthreads(8, 8, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint parity = dispatchThreadID.x | dispatchThreadID.y;

    // 8x8 block
    uint2 usize = uint2((uint)size.x, (uint)size.y);
    if (dispatchThreadID.x < usize.x && dispatchThreadID.y < usize.y)
    {
        float2 moveToPixelCenter = float2(0.5f, 0.5f) * invSize;
        float2 uv = (invSize * float2((float)dispatchThreadID.x, (float)dispatchThreadID.y)) + moveToPixelCenter;

        float4 srcData = src.GatherRed(srcSampler, uv, 0);
        float tiledepth = 0.0f;
#ifdef ENUM_COMPARISON_MIN
        tiledepth = min(min(min(srcData.x, srcData.y), srcData.z), srcData.w);
#endif
#ifdef ENUM_COMPARISON_MAX
		tiledepth = max(max(max(srcData.x, srcData.y), srcData.z), srcData.w);
#endif
		dst0[dispatchThreadID.xy] = tiledepth;
        dTile[groupIndex] = tiledepth;
    }

    GroupMemoryBarrierWithGroupSync();

    // 4x4 block
    if ((parity & 1) == 0)
    {
        float tiledepth = 0.0f;
#ifdef ENUM_COMPARISON_MIN
        tiledepth = min(min(min(dTile[groupIndex], dTile[groupIndex + 1]), dTile[groupIndex + 8]), dTile[groupIndex + 9]);
#endif
#ifdef ENUM_COMPARISON_MAX
		tiledepth = max(max(max(dTile[groupIndex], dTile[groupIndex + 1]), dTile[groupIndex + 8]), dTile[groupIndex + 9]);
#endif
		dTile[groupIndex] = tiledepth;
        dst1[dispatchThreadID.xy >> 1] = tiledepth;
    }

    GroupMemoryBarrierWithGroupSync();

    // 2x2 block
    if ((parity & 3) == 0)
    {
        float tiledepth = 0.0f;
#ifdef ENUM_COMPARISON_MIN
        tiledepth = min(min(min(dTile[groupIndex], dTile[groupIndex + 2]), dTile[groupIndex + 16]), dTile[groupIndex + 18]);
#endif
#ifdef ENUM_COMPARISON_MAX
		tiledepth = max(max(max(dTile[groupIndex], dTile[groupIndex + 2]), dTile[groupIndex + 16]), dTile[groupIndex + 18]);
#endif
        dTile[groupIndex] = tiledepth;
        dst2[dispatchThreadID.xy >> 2] = tiledepth;
    }

    GroupMemoryBarrierWithGroupSync();

    // 1x1 block
    if ((parity & 7) == 0)
    {
        float tiledepth = 0.0f;
#ifdef ENUM_COMPARISON_MIN
        tiledepth = min(min(min(dTile[groupIndex], dTile[groupIndex + 4]), dTile[groupIndex + 32]), dTile[groupIndex + 36]);
#endif
#ifdef ENUM_COMPARISON_MAX
		tiledepth = max(max(max(dTile[groupIndex], dTile[groupIndex + 4]), dTile[groupIndex + 32]), dTile[groupIndex + 36]);
#endif
        dst3[dispatchThreadID.xy >> 3] = tiledepth;
    }
}
