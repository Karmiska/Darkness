
Texture2DArray<float> shadowMap;
StructuredBuffer<float4x4> shadowVP;

cbuffer ShadowConstantData
{
	float2 shadowSize;
};

SamplerComparisonState shadow_sampler;
