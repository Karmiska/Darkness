
RWBuffer<uint> operationBuffer;

cbuffer MathOperationConstants
{
    uint count;
    uint offset;
    float value;
    float padding;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < count)
    {
#ifdef ENUM_OPERATION_ADDITION
        operationBuffer[dispatchThreadID.x + offset] = operationBuffer[dispatchThreadID.x + offset] + value;
#endif
#ifdef ENUM_OPERATION_SUBTRACTION
        operationBuffer[dispatchThreadID.x + offset] = operationBuffer[dispatchThreadID.x + offset] - value;
#endif
#ifdef ENUM_OPERATION_DIVISION
        operationBuffer[dispatchThreadID.x + offset] = operationBuffer[dispatchThreadID.x + offset] / value;
#endif
#ifdef ENUM_OPERATION_MULTIPLICATION
        operationBuffer[dispatchThreadID.x + offset] = operationBuffer[dispatchThreadID.x + offset] * value;
#endif
#ifdef ENUM_OPERATION_SET
		operationBuffer[dispatchThreadID.x + offset] = value;
#endif
    }
}
