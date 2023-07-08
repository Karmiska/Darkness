
#include "../Common.hlsli"
#include "../GBuffer.hlsli"

struct PSInput
{
	float4 position         : SV_Position0;
	float2 uv               : TEXCOORD0;
};

Texture2D<float4> color;
Texture2D<float> depthTexture;

sampler depthSampler;
sampler colorSampler;

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

float2 worldPosToUV(float3 worldPos)
{
	float4 projected = mul(cameraProjectionMatrix, mul(cameraViewMatrix, float4(worldPos, 1.0f)));
	projected.xy /= projected.w;
	projected.xy = projected.xy * 0.5 + 0.5;
	projected.y = 1.0f - projected.y;
	return projected.xy;
}

float2 march(float3 pos, float3 direction, out bool hit)
{
	int maxSteps = 80;
	float minStepLength = 0.02f;
	float stepLength = 0.2f;
	float sign = 1.0f;
	float maxDeviation = 0.1;

	float startDistance = length(pos - cameraWorldSpacePosition);
	while (startDistance > maxSteps * stepLength)
	{
		stepLength *= 1.3f;
		maxDeviation *= 2.5f;
	}

	hit = true;
	float dlen = 0.0f;
	float clen = 0.0f;

	float3 currentPos = pos + (direction * stepLength);
	[loop]
	for (int i = 0; i < maxSteps; ++i)
	{
		float2 uv = worldPosToUV(currentPos);
		if (uv.x < 0.0f || uv.y < 0.0f || uv.x > 1.0f || uv.y > 1.0f)
		{
			hit = false;
			break;
		}

		float dsample = depthTexture.Load(uint3(uv.x * frameSize.x, uv.y * frameSize.y, 0));

		if (dsample < 0.00001f)
		{
			hit = false;
			break;
		}

		float3 depthWPos = worldPosFromUVDepth(uv, dsample);

		dlen = length(depthWPos - cameraWorldSpacePosition);
		clen = length(currentPos - cameraWorldSpacePosition);

		if (sign > 0.0f && dlen >= clen)
		{
			currentPos += direction * (stepLength * sign);
		}
		else if (sign > 0.0f && dlen < clen)
		{
			sign *= -1.0f;
			stepLength /= 2.0f;
			currentPos += direction * (stepLength * sign);
		}
		else if (sign < 0.0f && dlen >= clen)
		{
			sign *= -1.0f;
			stepLength /= 2.0f;
			currentPos += direction * (stepLength * sign);
		}
		else if (sign < 0.0f && dlen < clen)
		{
			if (sign > 0.0)
			{
				stepLength /= 2.0f;
				sign *= -1.0f;
			}
			currentPos += direction * (stepLength * sign);
		}
		if (stepLength < minStepLength)
			break;
	}
	if (abs(dlen - clen) > maxDeviation)
		hit = false;

	return worldPosToUV(currentPos);
}

float3 normalFromUVDepth(float2 uv, float3 position)
{
	float3 posRight = worldPosFromUVDepth(uv + float2(onePerFrameSize.x, 0.0f), depthTexture.Sample(depthSampler, uv + float2(onePerFrameSize.x, 0.0f)).x);
	float3 posDown = worldPosFromUVDepth(uv + float2(0.0f, onePerFrameSize.y), depthTexture.Sample(depthSampler, uv + float2(0.0f, onePerFrameSize.y)).x);
	return normalize(cross(posRight - position, position - posDown));
}

float4 main(PSInput input) : SV_Target
{
	float2 uv = input.uv;
	float depth = depthTexture.Sample(depthSampler, uv).x;
	float3 position = worldPosFromUVDepth(uv, depth);

	float3 normal = normalFromUVDepth(uv, position);
	float3 viewRay = normalize(position - cameraWorldSpacePosition);

	float3 r = reflect(viewRay, normal);
	float3 resCol = float3(0, 0, 0);

	bool hit;
	float2 resColUV = march(position, r, hit);

	if (!hit)
	{
		discard;
	}
	else
	{
		resCol = color.SampleLevel(colorSampler, resColUV, 0).xyz;
	}

	return float4(resCol, 1.0);
}
