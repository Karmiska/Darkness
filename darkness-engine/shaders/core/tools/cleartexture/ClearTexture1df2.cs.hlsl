
RWTexture1D<float2> tex;

cbuffer ClearConstants
{
    uint4 size;
    float2 value;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < size.x)
    {
        tex[dispatchThreadID.x] = value;
    }
}
