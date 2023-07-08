
Texture2D<uint> LumaBuf;
RWBuffer<uint> Histogram;

groupshared uint g_TileHistogram[256];

cbuffer GenerateHistogramConstants
{
    uint height;
    uint width;
}

[numthreads(16, 16, 1)]
void main(uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID)
{
    g_TileHistogram[GI] = 0;

    GroupMemoryBarrierWithGroupSync();

    // Loop 24 times until the entire column has been processed
    for (uint TopY = 0; TopY < height; TopY += 16)
    {
        uint QuantizedLogLuma = LumaBuf[DTid.xy + uint2(0, TopY)];
        InterlockedAdd(g_TileHistogram[QuantizedLogLuma], 1);
    }

    GroupMemoryBarrierWithGroupSync();

    uint newValue;
    InterlockedAdd(Histogram[GI], g_TileHistogram[GI], newValue);
}
