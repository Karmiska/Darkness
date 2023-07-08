
Texture3D<float3> src;
RWTexture3D<float3> dst;

cbuffer CopyConstants
{
    uint srcTop;
    uint srcLeft;
    uint srcFront;
    uint srcTextureWidth;

    uint srcTextureHeight;
    uint srcTextureDepth;
    uint dstTop;
    uint dstLeft;

    uint dstFront;
    uint dstTextureWidth;
    uint dstTextureHeight;
    uint dstTextureDepth;

    uint copyWidth;
    uint copyHeight;
    uint copyDepth;
};

[numthreads(4, 4, 4)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint3 srcLocation = uint3(srcLeft + dispatchThreadID.x, srcTop + dispatchThreadID.y, srcFront + dispatchThreadID.z);
    uint3 dstLocation = uint3(dstLeft + dispatchThreadID.x, dstTop + dispatchThreadID.y, dstFront + dispatchThreadID.z);

    if (
        (dispatchThreadID.x < copyWidth) &&
        (dispatchThreadID.y < copyHeight) &&
        (dispatchThreadID.z < copyDepth) &&
        (srcLocation.x < srcTextureWidth) &&
        (srcLocation.y < srcTextureHeight) &&
        (srcLocation.z < srcTextureDepth) &&
        (dstLocation.x < dstTextureWidth) &&
        (dstLocation.y < dstTextureHeight) &&
        (dstLocation.z < dstTextureDepth)
        )
    {
        dst[dstLocation] = src[srcLocation];
    }
}
