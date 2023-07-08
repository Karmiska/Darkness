Texture2D<uint> instanceId;
RWBuffer<uint> output;

cbuffer PickConstants
{
    uint2 mousePosition;
};

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    output[0] = instanceId[mousePosition];
}
