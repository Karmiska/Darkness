
Buffer<uint> src;
RWBuffer<uint> dst;
RWBuffer<uint> workBuffer;

cbuffer SortConstants
{
    uint count;
    uint inclusive;
};


groupshared uint warpscan[2][64];

uint scan(RWBuffer<uint> o, Buffer<uint> i, int n, int tid, int gid)
{
    if(inclusive)
        warpscan[0][gid] = i[tid];
    else
        warpscan[0][gid] = (tid > 0) ? i[tid - 1] : 0;
    GroupMemoryBarrierWithGroupSync();
    
    int pout = 0;
    int pin = 1;

    for (int offset = 1; offset < n; offset <<= 1)
    {
        pout = 1 - pout;
        pin = 1 - pin;
        if (gid >= offset)
            warpscan[pout][gid] = warpscan[pin][gid - offset] + warpscan[pin][gid];
        else
            warpscan[pout][gid] = warpscan[pin][gid];

        GroupMemoryBarrierWithGroupSync();
    }

    o[tid] = warpscan[pout][gid];
    return warpscan[pout][63];
}

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint GroupIndex : SV_GroupIndex)
{
    uint pre = scan(dst, src, 64, dispatchThreadID.x, GroupIndex);

#ifdef OPTION_OUTPUTSUMS
    if (GroupIndex == 0)
    {
        workBuffer[dispatchThreadID.x / 64] = pre;
    }
#endif
}
