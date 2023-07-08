
Buffer<uint> srcA;
Buffer<uint> srcB;
RWBuffer<uint> output;

cbuffer MathOperationConstants
{
    uint count;
    uint srcAOffset;
    uint srcBOffset;
    uint outputOffset;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < count)
    {
#ifdef ENUM_OPERATION_ADDITION
        output[dispatchThreadID.x + outputOffset] = srcA[dispatchThreadID.x + srcAOffset] + srcB[dispatchThreadID.x + srcBOffset];
#endif
#ifdef ENUM_OPERATION_SUBTRACTION
        output[dispatchThreadID.x + outputOffset] = srcA[dispatchThreadID.x + srcAOffset] - srcB[dispatchThreadID.x + srcBOffset];
#endif
#ifdef ENUM_OPERATION_DIVISION
        output[dispatchThreadID.x + outputOffset] = srcA[dispatchThreadID.x + srcAOffset] / srcB[dispatchThreadID.x + srcBOffset];
#endif
#ifdef ENUM_OPERATION_MULTIPLICATION
        output[dispatchThreadID.x + outputOffset] = srcA[dispatchThreadID.x + srcAOffset] * srcB[dispatchThreadID.x + srcBOffset];
#endif
    }
}
