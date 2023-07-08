
Buffer<uint> inputs;
RWBuffer<uint> outputs;

cbuffer DispatchConstants
{
    uint size;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < size)
    {
        outputs[dispatchThreadID.x] = inputs[dispatchThreadID.x];
    }
}
