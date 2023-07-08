
Texture2D<float> input;
RWBuffer<float> output;

groupshared float buffer[64];

[numthreads(8, 8, 1)]
void main(uint3 Gid : SV_GroupID, uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID)
{
    float sumThisThread = input[DTid.xy];
    buffer[GI] = sumThisThread;
    GroupMemoryBarrierWithGroupSync();

    sumThisThread += buffer[GI + 32];
    buffer[GI] = sumThisThread;
    GroupMemoryBarrierWithGroupSync();

    sumThisThread += buffer[GI + 16];
    buffer[GI] = sumThisThread;
    GroupMemoryBarrierWithGroupSync();

    sumThisThread += buffer[GI + 8];
    buffer[GI] = sumThisThread;
    GroupMemoryBarrierWithGroupSync();

    sumThisThread += buffer[GI + 4];
    buffer[GI] = sumThisThread;
    GroupMemoryBarrierWithGroupSync();

    sumThisThread += buffer[GI + 2];
    buffer[GI] = sumThisThread;
    GroupMemoryBarrierWithGroupSync();

    sumThisThread += buffer[GI + 1];

    if (GI == 0)
        output[Gid.x + Gid.y * 5] = sumThisThread / 64.0f;
}
