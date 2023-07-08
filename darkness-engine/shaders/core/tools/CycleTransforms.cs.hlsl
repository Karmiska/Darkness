#include "../shared_types/TransformHistory.hlsli"

RWStructuredBuffer<TransformHistory> historyBuffer;

cbuffer CycleTransformsConstants
{
    uint transformCount;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < transformCount)
    {
        //historyBuffer[dispatchThreadID.x].previousTransform = historyBuffer[dispatchThreadID.x].transform;
        historyBuffer[dispatchThreadID.x].previousTransform._m00 = historyBuffer[dispatchThreadID.x].transform._m00;
        historyBuffer[dispatchThreadID.x].previousTransform._m01 = historyBuffer[dispatchThreadID.x].transform._m01;
        historyBuffer[dispatchThreadID.x].previousTransform._m02 = historyBuffer[dispatchThreadID.x].transform._m02;
        historyBuffer[dispatchThreadID.x].previousTransform._m03 = historyBuffer[dispatchThreadID.x].transform._m03;

        historyBuffer[dispatchThreadID.x].previousTransform._m10 = historyBuffer[dispatchThreadID.x].transform._m10;
        historyBuffer[dispatchThreadID.x].previousTransform._m11 = historyBuffer[dispatchThreadID.x].transform._m11;
        historyBuffer[dispatchThreadID.x].previousTransform._m12 = historyBuffer[dispatchThreadID.x].transform._m12;
        historyBuffer[dispatchThreadID.x].previousTransform._m13 = historyBuffer[dispatchThreadID.x].transform._m13;

        historyBuffer[dispatchThreadID.x].previousTransform._m20 = historyBuffer[dispatchThreadID.x].transform._m20;
        historyBuffer[dispatchThreadID.x].previousTransform._m21 = historyBuffer[dispatchThreadID.x].transform._m21;
        historyBuffer[dispatchThreadID.x].previousTransform._m22 = historyBuffer[dispatchThreadID.x].transform._m22;
        historyBuffer[dispatchThreadID.x].previousTransform._m23 = historyBuffer[dispatchThreadID.x].transform._m23;

        historyBuffer[dispatchThreadID.x].previousTransform._m30 = historyBuffer[dispatchThreadID.x].transform._m30;
        historyBuffer[dispatchThreadID.x].previousTransform._m31 = historyBuffer[dispatchThreadID.x].transform._m31;
        historyBuffer[dispatchThreadID.x].previousTransform._m32 = historyBuffer[dispatchThreadID.x].transform._m32;
        historyBuffer[dispatchThreadID.x].previousTransform._m33 = historyBuffer[dispatchThreadID.x].transform._m33;
    }
}
