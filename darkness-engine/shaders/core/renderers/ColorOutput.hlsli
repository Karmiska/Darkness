

outputColor = float4(color.xyz, 1.0f);
#ifdef OPTION_DEBUG

static float3 mipDebugColors[12] =
{
    float3(1.0f, 0.0f, 0.0f),
    float3(0.5f, 0.0f, 0.0f),
    float3(0.0f, 1.0f, 0.0f),
    float3(0.0f, 0.5f, 0.0f),
    float3(0.0f, 0.0f, 1.0f),
    float3(0.0f, 0.0f, 0.5f),
    float3(1.0f, 1.0f, 0.0f),
    float3(0.5f, 0.5f, 0.0f),
    float3(0.0f, 1.0f, 1.0f),
    float3(0.0f, 0.5f, 0.5f),
    float3(1.0f, 0.0f, 1.0f),
    float3(0.5f, 0.0f, 0.5f)
};

if (debugMode == DEBUG_MODE_CLUSTERS)
{
	// Debug clusters
	outputColor = float4(color.xyz, 1.0f);
}
else if (debugMode == DEBUG_MODE_MIP_ALBEDO)
{
	// MipAlbedo
    float scaledMip = saturate(albedoMip / max((albedoMaxMip + FLT_EPS), FLT_EPS));
    float fr = max((scaledMip * 11.0f) - 0.0001f, 0.0f);
    int lowIndex = (int)fr;
    int highIndex = lowIndex + 1;
	outputColor = float4(lerp(mipDebugColors[lowIndex], mipDebugColors[highIndex], fr - (float)lowIndex), 1.0f);
}
else if (debugMode == DEBUG_MODE_MIP_ROUGHNESS)
{
	// MipRoughness
    float scaledMip = saturate(roughnessMip / roughnessMaxMip);
    float fr = max((scaledMip * 11.0f) - 0.0001f, 0.0f);
    int lowIndex = (int)fr;
    int highIndex = lowIndex + 1;
    outputColor = float4(lerp(mipDebugColors[lowIndex], mipDebugColors[highIndex], fr - (float)lowIndex), 1.0f);
}
else if (debugMode == DEBUG_MODE_MIP_METALNESS)
{
	// MipMetalness
    float scaledMip = saturate(metalnessMip / metalnessMaxMip);
    float fr = max((scaledMip * 11.0f) - 0.0001f, 0.0f);
    int lowIndex = (int)fr;
    int highIndex = lowIndex + 1;
    outputColor = float4(lerp(mipDebugColors[lowIndex], mipDebugColors[highIndex], fr - (float)lowIndex), 1.0f);
}
else if (debugMode == DEBUG_MODE_MIP_AO)
{
	// MipAo
    float scaledMip = saturate(aoMip / aoMaxMip);
    float fr = max((scaledMip * 11.0f) - 0.0001f, 0.0f);
    int lowIndex = (int)fr;
    int highIndex = lowIndex + 1;
    outputColor = float4(lerp(mipDebugColors[lowIndex], mipDebugColors[highIndex], fr - (float)lowIndex), 1.0f);
}
else if (debugMode == DEBUG_MODE_ALBEDO)
{
	// Albedo
	outputColor = float4(albedo.xyz, 1.0f);
}
else if (debugMode == DEBUG_MODE_ROUGHNESS)
{
	// Roughness
	outputColor = float4(roughness, roughness, roughness, 1.0f);
}
else if (debugMode == DEBUG_MODE_METALNESS)
{
	// Metalness
	outputColor = float4(metalness, metalness, metalness, 1.0f);
}
else if (debugMode == DEBUG_MODE_OCCLUSION)
{
	// Occlusion
	outputColor = float4(occlusion, occlusion, occlusion, 1.0f);
}
else if (debugMode == DEBUG_MODE_UV)
{
	// UV
	outputColor = float4(scaledObjectUV.xy, 0.0f, 1.0f);
}
else if (debugMode == DEBUG_MODE_NORMAL)
{
	// Normal
	outputColor = float4(normal.xyz, 1.0f);
}
else if (debugMode == DEBUG_MODE_MOTION)
{
	// motion vector is actually shown from TemporalResolve.ps.hlsl
	outputColor = float4(color.xyz, 1.0f);
}
else if (debugMode == DEBUG_MODE_LIGHT_BINS)
{
	// light bins
	/*for (uint i = 0; i < lightCount2; ++i)
	{
	uint lightIndex = lightBins[countLocation + 1 + i];
	color += lightColor[lightIndex].xyz * 0.2;
	}*/

	/*float4 temp = float4(
	rand_1_05(float2(drawnLights, drawnLights)),
	rand_1_05(float2(drawnLights + 1, drawnLights)),
	rand_1_05(float2(drawnLights + 2, drawnLights)),
	1.0f);

	outputColor = float4(color.xyz, 1.0f) * temp;*/
	if (drawnLights == 0)
		outputColor = float4(color.xyz, 1.0f) * float4(0.4f, 2.0f, 0.4f, 1.0f);
	else if (drawnLights == 1)
		outputColor = float4(color.xyz, 1.0f) * float4(0.1f, 0.7f, 0.1f, 1.0f);
	else if (drawnLights == 2)
		outputColor = float4(color.xyz, 1.0f) * float4(0.2f, 0.4f, 2.0f, 1.0f);
	else if (drawnLights == 3)
		outputColor = float4(color.xyz, 1.0f) * float4(0.4f, 2.0f, 2.0f, 1.0f);
	else if (drawnLights == 4)
		outputColor = float4(color.xyz, 1.0f) * float4(1.3f, 1.4f, 0.4f, 1.0f);
	else if (drawnLights == 5)
		outputColor = float4(color.xyz, 1.0f) * float4(1.0f, 0.4f, 0.4f, 1.0f);
	else if (drawnLights == 6)
		outputColor = float4(color.xyz, 1.0f) * float4(0.5f, 0.05f, 0.05f, 1.0f);
	else if (drawnLights == 7)
		outputColor = float4(color.xyz, 1.0f) * float4(0.3f, 0.02f, 0.02f, 1.0f);
	else if (drawnLights == 8)
		outputColor = float4(color.xyz, 1.0f) * float4(0.1f, 0.01f, 0.01f, 1.0f);
	else
		outputColor = float4(color.xyz, 1.0f) * float4(0.05f, 0.005f, 0.005f, 1.0f);
	//outputColor = float4(color.xyz, 1.0f) * float4(drawnLights / 6.0f, drawnLights / 6.0f, drawnLights / 6.0f, 1.0f);
}
else if (debugMode == DEBUG_MODE_TRIANGLES)
{
	outputColor = float4(albedo.xyz, 1.0f);
}
else
{
	outputColor = float4(color, 1.0f);
}
#endif
