
RWTexture2D<uint> tex;

cbuffer ClearConstants
{
    uint4 size;
    uint value;
};

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if ((dispatchThreadID.x < size.x) &&
        (dispatchThreadID.y < size.y))
    {
        tex[dispatchThreadID.xy] = value;
    }
}
