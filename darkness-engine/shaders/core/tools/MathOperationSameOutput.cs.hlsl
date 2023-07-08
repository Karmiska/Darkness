
Buffer<uint> src;
RWBuffer<uint> inputOutput;

cbuffer MathOperationConstants
{
    uint count;
    uint srcOffset;
    uint inputOutputOffset;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < count)
    {
#ifdef ENUM_OPERATION_ADDITION
        inputOutput[dispatchThreadID.x + inputOutputOffset] = src[dispatchThreadID.x + srcOffset] + inputOutput[dispatchThreadID.x + inputOutputOffset];
#endif
#ifdef ENUM_OPERATION_SUBTRACTION
        inputOutput[dispatchThreadID.x + inputOutputOffset] = src[dispatchThreadID.x + srcOffset] - inputOutput[dispatchThreadID.x + inputOutputOffset];
#endif
#ifdef ENUM_OPERATION_DIVISION
        inputOutput[dispatchThreadID.x + inputOutputOffset] = src[dispatchThreadID.x + srcOffset] / inputOutput[dispatchThreadID.x + inputOutputOffset];
#endif
#ifdef ENUM_OPERATION_MULTIPLICATION
        inputOutput[dispatchThreadID.x + inputOutputOffset] = src[dispatchThreadID.x + srcOffset] * inputOutput[dispatchThreadID.x + inputOutputOffset];
#endif
    }
}
