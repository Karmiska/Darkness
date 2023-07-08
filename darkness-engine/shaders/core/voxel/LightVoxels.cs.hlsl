
#include "../Common.hlsli"
#include "../renderers/Shadows.hlsli"
#include "../PhysicallyBasedRendering.hlsli"

Buffer<float4> lightWorldPosition;
Buffer<float4> lightDirection;
Buffer<float4> lightColor;
Buffer<float4> lightParameters;
Buffer<uint> lightType;
Buffer<float> lightIntensity;
Buffer<float> lightRange;
Buffer<uint> lightIndexToShadowIndex;

Texture3D<uint> voxels;
Texture3D<float3> voxelNormals;
RWBuffer<uint> voxelColorgrid;

#include "VoxelDataCommon.hlsli"

cbuffer VoxelLightingConstantData
{
    uint lightCount;
    uint3 padding3;
};

[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (voxels[dispatchThreadID])
	{
		float3 worldPosition = worldPositionFromThreadID(dispatchThreadID);

		uint bufferIndex = bufferPosFromVoxel(dispatchThreadID);
		uint2 gridColor = uint2(voxelColorgrid[bufferIndex], voxelColorgrid[bufferIndex + 1]);
		uint4 unpackedcolor = unpackColor(gridColor);
		float3 col = decodeColor(unpackedcolor.xyz);
		float count = (float)unpackedcolor.w;
		float3 albedoColor = col / count;

		float3 normal = normalize(float3(0, 1, 0)); // normals[dispatchThreadID]; //
		float3 N = normal;
		float3 Lo = float3(0.0f, 0.0f, 0.0f);

		for (int lightIndex = 0; lightIndex < lightCount; ++lightIndex)
		{
			float distance = length(lightWorldPosition[lightIndex].xyz - worldPosition);
			float distanceSqrd = distance * distance;
			float attenuation = 1.0 / distanceSqrd;
			float distanceSqrdS = distanceSqrd + 10.0;

			float rangeSqrd = lightRange[lightIndex] * lightRange[lightIndex];
			float overDistance = distanceSqrd - rangeSqrd;
			const float dimArea = lightRange[lightIndex] * 0.1;
			float inDim = 1.0f - saturate((dimArea - overDistance) / dimArea);
			attenuation *= 1.0f - inDim;

			if (lightType[lightIndex] == 0)
			{
				// directional light
			}
			else if (lightType[lightIndex] == 1)
			{
				// spot light
				float3 L = normalize(lightWorldPosition[lightIndex].xyz - worldPosition);

				float3 radiance = (lightColor[lightIndex].xyz * lightIntensity[lightIndex]) * attenuation;

				float3 spotDirection = normalize(lightDirection[lightIndex].xyz);
				float cos_cur_angle = dot(L, spotDirection);

				float outer = lightParameters[lightIndex].y;
				float inner = lightParameters[lightIndex].x;
				bool shadowCasting = lightParameters[lightIndex].z > 0.0f;

				float angleDegrees = degrees(acos(cos_cur_angle));
				float angleDistance = outer - inner;

				if (angleDegrees < lightParameters[lightIndex].y)
				{
					float spot = clamp((outer - angleDegrees) / angleDistance, 0.0f, 1.0f);

					float kD = 1.0f;
					float NdotL = max(dot(N, L), 0.0);
					float3 directContrib = (kD * albedoColor / PI) * radiance * spot * NdotL;

					// TODO: WTF ?????
					if (isnan(directContrib.x)) directContrib.x = 0.0f;
					if (isnan(directContrib.y)) directContrib.y = 0.0f;
					if (isnan(directContrib.z)) directContrib.z = 0.0f;

					// spot shadow
					if (shadowCasting)
					{
						// shadow bias
						float hi = 0.00005;
						float lo = 0.000045;
						float bias = lerp(hi, lo, pow(NdotL, 0.2));
						float4 shadowPosition = mul(shadowVP[lightIndexToShadowIndex[lightIndex]], float4(worldPosition, 1.0));
						float2 shadowUV = (0.5 * (shadowPosition.xy / shadowPosition.w)) + float2(0.5, 0.5);

						shadowUV.y = 1.0f - shadowUV.y;
						float2 satShadowUV = saturate(shadowUV);

						if (satShadowUV.x == shadowUV.x && satShadowUV.y == shadowUV.y && (shadowPosition.z / shadowPosition.w > 0))
						{
							//PCF sampling for shadow map
							float sum = 0;
							float z = (shadowPosition.z / shadowPosition.w) + bias;
							float x, y;

							for (y = -1.5f; y <= 1.5f; y += 1.0f)
							{
								for (x = -1.5f; x <= 1.5f; x += 1.0f)
								{
									sum += shadowMap.SampleCmpLevelZero(
										shadow_sampler,
										float3(shadowUV + (float2(x, y) * shadowSize), lightIndexToShadowIndex[lightIndex]),
										z);
								}
							}
							float shadowFactor = sum / 16.0f;

							directContrib *= 1.0f - shadowFactor;
						}
					}

					Lo += directContrib;
				}
			}
			else if (lightType[lightIndex] == 2)
			{
				// point light
				float3 L = normalize(lightWorldPosition[lightIndex].xyz - worldPosition);

				float3 radiance = (lightColor[lightIndex].xyz * lightIntensity[lightIndex]) * attenuation;

				float NdotL = max(dot(N, L), 0.0);
				float3 directContrib = (albedoColor / PI) * radiance * NdotL;
				//float3 directContrib = albedoColor* NdotL;

				// TODO: WTF ?????
				if (isnan(directContrib.x)) directContrib.x = 0.0f;
				if (isnan(directContrib.y)) directContrib.y = 0.0f;
				if (isnan(directContrib.z)) directContrib.z = 0.0f;

				// point shadows
				bool shadowCasting = lightParameters[lightIndex].z > 0.0f;
				if (shadowCasting)
				{
					// shadow bias
					float hi = 0.00005;
					float lo = 0.000045;
					float bias = lerp(hi, lo, pow(NdotL, 0.2));

					for (int caster = 0; caster < 6; ++caster)
					{
						float4 shadowPosition = mul(shadowVP[lightIndexToShadowIndex[lightIndex] + caster], float4(worldPosition, 1.0));
						float2 shadowUV = (0.5 * (shadowPosition.xy / shadowPosition.w)) + float2(0.5, 0.5);

						shadowUV.y = 1.0f - shadowUV.y;
						float2 satShadowUV = saturate(shadowUV);

						if (satShadowUV.x == shadowUV.x && satShadowUV.y == shadowUV.y && (shadowPosition.z / shadowPosition.w > 0))
						{
							//return float4(1.0f, 0.0f, 0.0f, 1.0f);

							//PCF sampling for shadow map
							float sum = 0;
							float z = (shadowPosition.z / shadowPosition.w) + bias;
							float x, y;

							for (y = -1.5f; y <= 1.5f; y += 1.0f)
							{
								for (x = -1.5f; x <= 1.5f; x += 1.0f)
								{
									sum += shadowMap.SampleCmpLevelZero(
										shadow_sampler,
										float3(shadowUV + (float2(x, y) * shadowSize), lightIndexToShadowIndex[lightIndex] + caster),
										z);
								}
							}
							float shadowFactor = sum / 16.0f;

							directContrib *= 1.0f - shadowFactor;
						}
					}
				}
				
				Lo += directContrib;
			}
		}

		uint3 encodedColor = encodeColor(Lo);
		uint2 packedColor = packColor(encodedColor);
		packedColor.y |= 0x1;

		voxelColorgrid[bufferIndex] = packedColor.x;
		voxelColorgrid[bufferIndex + 1] = packedColor.y;
	}
}
