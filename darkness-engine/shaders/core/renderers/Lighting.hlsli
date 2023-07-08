
#include "Shadows.hlsli"

Buffer<float4> lightWorldPosition;
Buffer<float4> lightDirection;
Buffer<float4> lightColor;
Buffer<float4> lightParameters;
Buffer<uint> lightType;
Buffer<float> lightIntensity;
Buffer<float> lightRange;
Buffer<uint> lightBins;
Buffer<uint> lightIndexToShadowIndex;

cbuffer LightingConstantData
{
	uint2 binSize;
};

uint performLighting(
	float3 worldPosition, 
	float3 cameraPosition, 
	float3 normal,
	float3 toCamera,
	float3 albedoColor,
	float metalness,
	float roughness,
	uint2 screenCoordinate,
	float3 F0,
	inout float3 Lo)
{
	float3 N = normal;
	float3 V = toCamera; // maybe switch?

	uint2 binLocation = screenCoordinate / 8;
	uint countLocation = (binLocation.y * binSize.x) + binLocation.x;
	uint lightCount2 = lightBins[countLocation];

#ifdef OPTION_DEBUG
	uint drawnLights = 0;
#endif

	//for (uint i = 0; i < lightCount2; ++i)
	uint lightBits = lightCount2;
	uint bucket = 0;
	while (lightBits != 0)
	{
		const uint bucket_bit_index = firstbitlow(lightBits);
		const uint entity_index = bucket * 32 + bucket_bit_index;
		lightBits ^= (uint)1 << bucket_bit_index;
		uint lightIndex = entity_index;

		//uint lightLocation = (countLocation + ((i + 1) * (binSize.x * binSize.y)));
		//uint lightIndex = lightBins[lightLocation];


#ifdef OPTION_DEBUG
		++drawnLights;
#endif

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
			float3 L = normalize(lightDirection[lightIndex].xyz);
			float3 H = normalize(V + L);

			float3 radiance = (lightColor[lightIndex].xyz * lightIntensity[lightIndex]);

			float NDF = distributionGGX(N, H, roughness);
			float G = geometryFunctionSmith(N, V, L, roughness);
			float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

			float3 kS = F;
			float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
			kD *= 1.0f - metalness;

			float3 nominator = NDF * G * F;
			float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
			float3 specular = nominator / denominator;

			float NdotL = max(dot(N, L), 0.0);
			float3 directContrib = (kD * albedoColor / PI + specular) * radiance * NdotL;
			Lo += directContrib;
		}
		else if (lightType[lightIndex] == 1)
		{
			// spot light
			float3 L = normalize(lightWorldPosition[lightIndex].xyz - worldPosition);
			float3 H = normalize(V + L);

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

				float NDF = distributionGGX(N, H, roughness);
				float G = geometryFunctionSmith(N, V, L, roughness);
				float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

				float3 kS = F;
				float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
				kD *= 1.0f - metalness;

				float3 nominator = NDF * G * F;
				float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
				float3 specular = nominator / denominator;

				float NdotL = max(dot(N, L), 0.0);
				float3 directContrib = (kD * albedoColor / PI + specular) * radiance * NdotL * spot;

                // TODO: WTF ?????
                if (isnan(directContrib.x)) directContrib.x = 0.0f;
                if (isnan(directContrib.y)) directContrib.y = 0.0f;
                if (isnan(directContrib.z)) directContrib.z = 0.0f;

				// spot shadow
				if (shadowCasting)
				{
					// shadow bias
					/*float hi = 0.00005;
					float lo = 0.000045;
					float bias = lerp(hi, lo, pow(NdotL, 0.2));*/

					float bias = 0.0f;// 0.00001f;
					float hi = 0.00005;
					float lo = 0.000045;
					float di = distance / 10.0;
					//if(di < 3.0f)
					//	bias = hi;
					bias = lerp(hi, lo, saturate(di));
					
					// distance bias
					/*float noFlicker = 0.0006;
					float flicker = 0.0000;
					float camDistance = length(worldPosition - cameraPosition);
					camDistance /= 55.0f;
					camDistance = min(camDistance, 1.0f);
					camDistance = 1.0f - camDistance;
					float distanceBias = lerp(noFlicker, flicker, camDistance);

					float anoFlicker = 0.04;
					float aflicker = 0.0000;
					float acamDistance = length(worldPosition - lightWorldPosition[lightIndex].xyz);
					float capDistance = max(acamDistance - 1.0f, 0.0f);
					acamDistance /= 5.0f;
					acamDistance = min(acamDistance, 1.0f);
					float adistanceBias = lerp(anoFlicker, aflicker, acamDistance);*/

					//directContrib *= 1.0 - ((shadowSample | (1 << i)) >> i);
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
			float3 H = normalize(V + L);

			float3 radiance = (lightColor[lightIndex].xyz * lightIntensity[lightIndex]) * attenuation;

			float NDF = distributionGGX(N, H, roughness);
			//float NDF = GGX_D(dot(N, H), roughnessValue);
			float G = geometryFunctionSmith(N, V, L, roughness);
			//float G = GGX_OPT3(N, V, L, roughnessValue, F0);
			float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

			float3 kS = F;
			float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
			kD *= 1.0f - metalness;

			float3 nominator = NDF * G * F;
			float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
			float3 specular = nominator / denominator;

			float NdotL = max(dot(N, L), 0.0);
			float3 directContrib = (kD * albedoColor / PI + specular) * radiance * NdotL;
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
				float hi = 0.00000030;// * distanceSqrdS;
				float lo = 0.00000025;// * distanceSqrdS;
				float bias = lerp(hi, lo, pow(NdotL, 0.2));

				// distance bias
				float noFlicker = 0.0004;
				float flicker = 0.0000;
				float camDistance = length(worldPosition - cameraPosition);
				camDistance /= 55.0f;
				camDistance = min(camDistance, 1.0f);
				camDistance = 1.0f - camDistance;
				float distanceBias = lerp(noFlicker, flicker, camDistance);

				float anoFlicker = 0.04;
				float aflicker = 0.0000;
				float acamDistance = length(worldPosition - lightWorldPosition[lightIndex].xyz);
				float capDistance = max(acamDistance - 1.0f, 0.0f);
				acamDistance /= 5.0f;
				acamDistance = min(acamDistance, 1.0f);
				float adistanceBias = lerp(anoFlicker, aflicker, acamDistance);

				for (int caster = 0; caster < 6; ++caster)
				{
					float4 shadowPosition = mul(shadowVP[lightIndexToShadowIndex[lightIndex]+caster], float4(worldPosition, 1.0));
					float2 shadowUV = (0.5 * (shadowPosition.xy / shadowPosition.w)) + float2(0.5, 0.5);

					shadowUV.y = 1.0f - shadowUV.y;
					float2 satShadowUV = saturate(shadowUV);

					if (satShadowUV.x == shadowUV.x && satShadowUV.y == shadowUV.y && (shadowPosition.z / shadowPosition.w > 0))
					{
						//return float4(1.0f, 0.0f, 0.0f, 1.0f);

						//PCF sampling for shadow map
						float sum = 0;
						float z = (shadowPosition.z / shadowPosition.w) + bias + distanceBias + adistanceBias;
						float x, y;

						for (y = -1.5f; y <= 1.5f; y += 1.0f)
						{
							for (x = -1.5f; x <= 1.5f; x += 1.0f)
							{
								sum += shadowMap.SampleCmpLevelZero(
									shadow_sampler,
									float3(shadowUV + (float2(x, y) * shadowSize), lightIndexToShadowIndex[lightIndex]+caster),
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
#ifdef OPTION_DEBUG
	return drawnLights;
#else
	return 0;
#endif
}
