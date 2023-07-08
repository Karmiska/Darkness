
#include "../Common.hlsli"
#include "../GBuffer.hlsli"

struct PSInput
{
	float4 position         : SV_Position0;
	float4 viewRay          : TEXCOORD0;
	float2 uv               : TEXCOORD1;

};

Texture2D<float> depthTexture;
Texture2D<float4> noiseTexture;
Buffer<float4> samples;
sampler ssaoSampler;
sampler depthSampler;
sampler defaultSampler;

cbuffer Constants
{
	float4x4 cameraProjectionMatrix;
	float4x4 cameraViewMatrix;
	float4x4 invView;
	float4x4 invProjection;
	float2 frameSize;
	float2 onePerFrameSize;
	float2 nearFar;
	float2 padding;
	float3 cameraWorldSpacePosition;
	float padding2;
};

float linDepth(float depth, float2 nearFar)
{
	return 1.0 / (((nearFar.y - nearFar.x) - nearFar.x) * depth + 1.0);
}

float3 worldPosFromUVDepth(float2 uv, float depth)
{
	float3 inputpos = float3(uv.x * 2.0f - 1.0f, (uv.y * 2.0f - 1.0f) * -1.0f, depth);
	float4 ci = mul(invProjection, float4(inputpos.xyz, 1.0f));
	ci.xyz /= ci.w;
	return mul(invView, float4(ci.xyz, 0.0f)).xyz + cameraWorldSpacePosition.xyz;
}

float main(PSInput input) : SV_Target
{
	float occlusion = 0.0f;
	int kernelSize = 8;
	float radius = 0.01;
	float bias = 0.00025;

	float2 uv = input.uv;
	float depth = depthTexture.Sample(depthSampler, uv).x;
	float3 position = worldPosFromUVDepth(uv, depth);

	float3 posRight = worldPosFromUVDepth(uv + float2(onePerFrameSize.x, 0.0f), depthTexture.Sample(depthSampler, uv + float2(onePerFrameSize.x, 0.0f)).x);
	float3 posDown = worldPosFromUVDepth(uv + float2(0.0f, onePerFrameSize.y), depthTexture.Sample(depthSampler, uv + float2(0.0f, onePerFrameSize.y)).x);
	float3 normal = normalize(cross(posRight - position, position - posDown));

	// this is like the deferred ssao shader. but really should fix all of these
	float3 viewRay = float4(normalize(input.viewRay.xyz), 0).xyz;
	position = viewRay * linDepth(depth, nearFar);


	float2 noiseScale = frameSize / 4.0;
	float3 randomVec = noiseTexture.Sample(ssaoSampler, uv * noiseScale).xyz;

	normal = mul(cameraViewMatrix, float4(normal, 0)).xyz;
	float3 ranVec = randomVec - (normal * dot(randomVec, normal));
	float3 tangent = normalize(ranVec);
	float3 bitangent = cross(normal, tangent);
	float3x3 TBN = float3x3(tangent, bitangent, normal);
	TBN = transpose(TBN);

	float3 de;

	for (int i = 0; i < kernelSize; ++i)
	{
		float3 samp = mul(TBN, samples[i].xyz);
		samp = position + (normal * bias) + (samp * radius);

		float4 offset = float4(samp, 1.0);
		offset = mul(cameraProjectionMatrix, offset);
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;
		offset.y = 1.0f - offset.y;

		float3 depthSample = viewRay * linDepth(depthTexture.Sample(depthSampler, offset.xy).x, nearFar);

		float rangeCheck = abs(depthSample.z - samp.z) < radius ? 1.0 : 0.0;
		occlusion += (samp.z < depthSample.z ? 1.0 : 0.0) * rangeCheck;
	}
	occlusion = (occlusion / kernelSize) * 1.5f;

	return occlusion;
}
