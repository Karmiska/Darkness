#ifdef __cplusplus
#pragma once
#endif

struct TransformHistory
{
    float4x4 transform;
    float4x4 previousTransform;
    float4x4 inverseTransform;
};
