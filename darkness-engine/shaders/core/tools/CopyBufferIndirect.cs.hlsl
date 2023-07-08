
Buffer<uint>    src;
Buffer<uint>    srcIndex;

RWBuffer<uint>  dst;
Buffer<uint>    dstIndex;

Buffer<uint>    count;
Buffer<uint>    countIndex;

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < count[countIndex[0]])
    {
        dst[dispatchThreadID.x + dstIndex[0]] = src[dispatchThreadID.x + srcIndex[0]];
    }
}
