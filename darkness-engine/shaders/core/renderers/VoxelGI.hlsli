
#ifdef OPTION_VOXELREFLECTIONS
if (roughness < 0.95)
{
	int3 voxelGridPosition;
	if (voxelGridPositionFromWorldPosition(worldPosition, voxelGridPosition))
	{
		float3 voxelContribution = float3(0.0f, 0.0f, 0.0f);

		int3 hit;

		float3 refDir = reflect((worldPosition - cameraWorldSpacePosition), normal);
		refDir = normalize(refDir);

		/*if (gridMarch(voxelGridPosition, refDir, hit))
		{
			uint bufferIndex = bufferPosFromVoxel(hit * (1 << voxelMip));
			uint2 gridColor = uint2(voxelColorgrid[bufferIndex], voxelColorgrid[bufferIndex + 1]);
			uint4 unpackedcolor = unpackColor(gridColor);
			float3 col = decodeColor(unpackedcolor.xyz);
			float count = (float)unpackedcolor.w;

			color += col / count;
		}*/

		float3 gridPos;
		voxelGridPositionFromWorldPositionFloat(worldPosition, gridPos);
		if (gridMarch(gridPos, refDir, normal, hit))
		{
			uint bufferIndex = bufferPosFromVoxel(hit * ((uint)1 << voxelMip));
			uint2 gridColor = uint2(voxelColorgrid[bufferIndex], voxelColorgrid[bufferIndex + 1]);
			uint4 unpackedcolor = unpackColor(gridColor);
			float3 col = decodeColor(unpackedcolor.xyz);
			float count = (float)unpackedcolor.w;

			float3 voxelCol = col / count;
			
			color += (col / count);
			//specular += voxelCol * (F * envBRDF.x + envBRDF.y);

		}
		//else
		//	color = float3(1, 0, 0);

		/*const uint sampleCount = 4u;
		float3 gridPos;
		voxelGridPositionFromWorldPositionFloat(worldPosition, gridPos);
		for (uint i = 0u; i < sampleCount; ++i)
		{
			float2 Xi = hammersley(i, sampleCount);
			float3 H = importanceSampleGGX(Xi, refDir, roughness);

			if (gridMarch2(gridPos, H, normal, hit))
			{
				uint bufferIndex = bufferPosFromVoxel(hit * (1 << voxelMip));
				uint2 gridColor = uint2(voxelColorgrid[bufferIndex], voxelColorgrid[bufferIndex + 1]);
				uint4 unpackedcolor = unpackColor(gridColor);
				float3 col = decodeColor(unpackedcolor.xyz);
				float count = (float)unpackedcolor.w;

				voxelContribution += col / count;
			}
		}
		color += voxelContribution;*/


		/*const uint sampleCount = 1u;
		float3 voxelContribution = float3(0.0f, 0.0f, 0.0f);
		for (uint i = 0u; i < sampleCount; ++i)
		{
			float2 Xi = hammersley(i, sampleCount);
			float3 H = importanceSampleGGX(Xi, normal, roughness);

			int3 hit;

			if (gridMarch(voxelGridPosition, H, hit))
			{
				uint bufferIndex = bufferPosFromVoxel(hit * (1 << voxelMip));
				uint2 gridColor = uint2(voxelColorgrid[bufferIndex], voxelColorgrid[bufferIndex + 1]);
				uint4 unpackedcolor = unpackColor(gridColor);
				float3 col = decodeColor(unpackedcolor.xyz);
				float count = (float)unpackedcolor.w;

				voxelContribution += col / count;
			}
		}
		color += voxelContribution;*/

	}
}

/*float3 environIrradianceSample = sampleEnvironment(normal, 0.0f);
float3 kS = F;
float3 kD = 1.0 - kS;
kD *= 1.0f - metalness;
float3 irradiance = environIrradianceSample;
float3 diffuse = irradiance * albedo.xyz;

float3 ambient = float3(0.0f, 0.0f, 0.0f);
if (ssrEnabled)
{
	ambient = max((kD * diffuse + specular) * occlusion * environmentStrength, float3(0.0, 0.0, 0.0));
}
else
{
	ambient = max((kD * diffuse + specular) * occlusion * environmentStrength, float3(0.0, 0.0, 0.0));
}


float3 color = (ambient + Lo);

color *= exposure;
*/
#endif
