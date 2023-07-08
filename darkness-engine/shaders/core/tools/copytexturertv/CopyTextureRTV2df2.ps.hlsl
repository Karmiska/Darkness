
struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

Texture2D<float2> src;

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

float2 main(PSInput input) : SV_Target
{
    uint2 currentPos = uint2(
        (uint)((float)dstTextureWidth * input.uv.x),
        (uint)((float)dstTextureHeight * input.uv.y));

    uint2 dstLeftTopCorner = uint2(dstLeft, dstTop);
    uint2 dstRightBottomCorner = dstLeftTopCorner + uint2(copyWidth, copyHeight);
    if (currentPos.x >= dstLeftTopCorner.x && currentPos.y >= dstLeftTopCorner.y &&
        currentPos.x < dstRightBottomCorner.x && currentPos.y < dstRightBottomCorner.y)
    {
        uint2 fromCorner = currentPos - uint2(dstLeft, dstTop);
        uint2 srcCorner = uint2(srcLeft, srcTop);
        return src.Load(uint3(srcCorner + fromCorner, 0));
    }
    else
    {
        discard;
        return float2(0.0f, 0.0f);
    }
}
