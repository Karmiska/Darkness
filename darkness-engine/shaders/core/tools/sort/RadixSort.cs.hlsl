
Buffer<uint> src;
RWBuffer<uint> dst;

cbuffer SortConstants
{
    uint count;
};

groupshared uint val[64];
groupshared uint ones[64];
groupshared uint sh[64];
groupshared uint dstIndexes[64];
groupshared uint totalFalses;

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint GroupIndex : SV_GroupIndex)
{
    //if (dispatchThreadID.x >= count)
    //    return;

    val[GroupIndex] = src[dispatchThreadID.x];

    [unroll(32)]
    for (uint i = 0; i < 32; ++i)
    {
        // count
        ones[GroupIndex] = (uint)((val[GroupIndex] >> i) & 1) == 0;

        GroupMemoryBarrierWithGroupSync();

        if (GroupIndex != 0)
        {
            sh[GroupIndex] = ones[GroupIndex - 1];
        }
        else
        {
            sh[GroupIndex] = 0;
        }

        GroupMemoryBarrierWithGroupSync();

        // scan
        //[unroll(int(log2(64)))]
        [unroll(6)]
        for (uint n = 1; n < 64; n <<= 1)
        {
            uint temp;
            if (GroupIndex > n)
            {
                temp = sh[GroupIndex] + sh[GroupIndex - n];
            }
            else
            {
                temp = sh[GroupIndex];
            }
                
            GroupMemoryBarrierWithGroupSync();

            sh[GroupIndex] = temp;

            GroupMemoryBarrierWithGroupSync();
        }

        // sum
        if (GroupIndex == 0)
        {
            totalFalses = ones[64 - 1] + sh[64 - 1];
        }

        GroupMemoryBarrierWithGroupSync();

        dstIndexes[GroupIndex] = ones[GroupIndex] ? sh[GroupIndex] : GroupIndex - sh[GroupIndex] + totalFalses;

        uint temp = val[GroupIndex];

        GroupMemoryBarrierWithGroupSync();

        val[dstIndexes[GroupIndex]] = temp;

        GroupMemoryBarrierWithGroupSync();
    }

    if(dispatchThreadID.x < count)
        dst[dispatchThreadID.x] = val[GroupIndex];
}
