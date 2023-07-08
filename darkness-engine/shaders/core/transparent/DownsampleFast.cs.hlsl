
Texture2D<float4> src;
RWTexture2D<float4> dst;

cbuffer CopyConstants
{
    uint2 srcCopySize;  // copy area size
    uint2 dstCopySize;  // copy area size

    uint2 srcStart;
    uint2 dstStart;

    float4 padding1;
    float4 padding2;
};

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if ((dispatchThreadID.x < dstCopySize.x) &&
        (dispatchThreadID.y < dstCopySize.y))
    {
        // dstCopySize defines the actual copy operation count
        uint2 dstLocation = uint2(
            dstStart.x + dispatchThreadID.x, 
            dstStart.y + dispatchThreadID.y);

        float phaseX = ((float)dispatchThreadID.x + 0.5) / (float)dstCopySize.x;
        float phaseY = ((float)dispatchThreadID.y + 0.5) / (float)dstCopySize.y;

        uint2 srcLocation = uint2(
            srcStart.x + (uint)((float)srcCopySize.x * phaseX),
            srcStart.y + (uint)((float)srcCopySize.y * phaseY));

        //dst[dstLocation] = src[srcLocation];

        float4 a0 = src[uint2(srcLocation.x - 1, srcLocation.y - 1)];
        float4 a1 = src[uint2(srcLocation.x    , srcLocation.y - 1)];
        float4 a2 = src[uint2(srcLocation.x + 1, srcLocation.y - 1)];

        float4 b0 = src[uint2(srcLocation.x - 1, srcLocation.y)];
        float4 b1 = src[uint2(srcLocation.x    , srcLocation.y)];
        float4 b2 = src[uint2(srcLocation.x + 1, srcLocation.y)];

        float4 c0 = src[uint2(srcLocation.x - 1, srcLocation.y + 1)];
        float4 c1 = src[uint2(srcLocation.x    , srcLocation.y + 1)];
        float4 c2 = src[uint2(srcLocation.x + 1, srcLocation.y + 1)];

        dst[dstLocation] = ((a0 + a1 + a2 + b0 + b1 + b2 + c0 + c1 + c2) / 9.0f);
    }
}
