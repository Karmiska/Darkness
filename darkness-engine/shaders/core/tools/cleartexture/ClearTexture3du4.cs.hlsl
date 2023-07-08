
RWTexture3D<uint4> tex;

cbuffer ClearConstants
{
    uint4 size;
    uint4 value;
};

[numthreads(4, 4, 4)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if ((dispatchThreadID.x < size.x) &&
        (dispatchThreadID.y < size.y) &&
        (dispatchThreadID.z < size.z))
    {
        tex[dispatchThreadID.xyz] = value;
    }
}
