
Texture2D<float> src;
RWTexture2D<float> dst;

cbuffer CopyConstants
{
    uint srcTop;
    uint srcLeft;
    uint srcTextureWidth;
    uint srcTextureHeight;

    uint dstTop;
    uint dstLeft;
    uint dstTextureWidth;
    uint dstTextureHeight;

    uint copyWidth;
    uint copyHeight;
};

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 srcLocation = uint2(srcLeft + dispatchThreadID.x, srcTop + dispatchThreadID.y);
    uint2 dstLocation = uint2(dstLeft + dispatchThreadID.x, dstTop + dispatchThreadID.y);

    if (
        (dispatchThreadID.x < copyWidth) &&
        (dispatchThreadID.y < copyHeight) &&
        (srcLocation.x < srcTextureWidth) &&
        (srcLocation.y < srcTextureHeight) &&
        (dstLocation.x < dstTextureWidth) &&
        (dstLocation.y < dstTextureHeight)
        )
    {
        dst[dstLocation] = src[srcLocation];
    }
}
