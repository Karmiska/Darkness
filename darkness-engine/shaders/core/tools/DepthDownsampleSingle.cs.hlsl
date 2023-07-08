
Texture2D<float> src;
RWTexture2D<float> dst;

sampler srcSampler;

cbuffer Constants
{
    float2 size;
    float2 invSize;
};

[numthreads(8, 8, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 usize = uint2((uint)size.x, (uint)size.y);
    if (dispatchThreadID.x < usize.x && dispatchThreadID.y < usize.y)
    {
        float2 moveToPixelCenter = float2(0.5f, 0.5f) * invSize;
        float2 uv = (invSize * float2((float)dispatchThreadID.x, (float)dispatchThreadID.y)) + moveToPixelCenter;

        float4 srcData = src.GatherRed(srcSampler, uv, 0);
#ifdef ENUM_COMPARISON_MIN
        dst[dispatchThreadID.xy] = min(min(min(srcData.x, srcData.y), srcData.z), srcData.w);
#endif
#ifdef ENUM_COMPARISON_MAX
		dst[dispatchThreadID.xy] = max(max(max(srcData.x, srcData.y), srcData.z), srcData.w);
#endif
    }
}

