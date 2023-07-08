
Texture2D<float3> BloomBuf;
RWTexture2D<float3> Result1;
RWTexture2D<float3> Result2;
RWTexture2D<float3> Result3;
RWTexture2D<float3> Result4;
sampler BiLinearClamp;

cbuffer DownsampleBloom4Constants
{
    float2 g_inverseDimensions;
}

groupshared float3 g_Tile[64];    // 8x8 input pixels

[numthreads(8, 8, 1)]
void main(uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID)
{
    // You can tell if both x and y are divisible by a power of two with this value
    uint parity = DTid.x | DTid.y;

    // Downsample and store the 8x8 block
    float2 centerUV = (float2(DTid.xy) * 2.0f + 1.0f) * g_inverseDimensions;
    float3 avgPixel = BloomBuf.SampleLevel(BiLinearClamp, centerUV, 0.0f);
    g_Tile[GI] = avgPixel;
    Result1[DTid.xy] = avgPixel;

    GroupMemoryBarrierWithGroupSync();

    // Downsample and store the 4x4 block
    if ((parity & 1) == 0)
    {
        avgPixel = 0.25f * (avgPixel + g_Tile[GI + 1] + g_Tile[GI + 8] + g_Tile[GI + 9]);
        g_Tile[GI] = avgPixel;
        Result2[DTid.xy >> 1] = avgPixel;
    }

    GroupMemoryBarrierWithGroupSync();

    // Downsample and store the 2x2 block
    if ((parity & 3) == 0)
    {
        avgPixel = 0.25f * (avgPixel + g_Tile[GI + 2] + g_Tile[GI + 16] + g_Tile[GI + 18]);
        g_Tile[GI] = avgPixel;
        Result3[DTid.xy >> 2] = avgPixel;
    }

    GroupMemoryBarrierWithGroupSync();

    // Downsample and store the 1x1 block
    if ((parity & 7) == 0)
    {
        avgPixel = 0.25f * (avgPixel + g_Tile[GI + 4] + g_Tile[GI + 32] + g_Tile[GI + 36]);
        Result4[DTid.xy >> 3] = avgPixel;
    }
}
