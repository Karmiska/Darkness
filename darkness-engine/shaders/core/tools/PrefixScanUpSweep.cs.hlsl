
RWBuffer<uint> dst;
Buffer<uint> workBuffer;

cbuffer SortConstants
{
    uint count;
};


[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint GroupIndex : SV_GroupIndex)
{
    dst[dispatchThreadID.x] += workBuffer[dispatchThreadID.x / 64];
}
