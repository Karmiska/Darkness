
float3 applyFog(in float3  rgb,      // original color of the pixel
	in float distance, // camera to point distance
	in float3  rayOri,   // camera position
	in float3  rayDir)  // camera to point vector
{
	//float b = 0.15f;
	//float c = 0.01f;
	//float fogAmount = c * exp(-rayOri.y*b) * (1.0 - exp(-distance * rayDir.y*b)) / rayDir.y;
	//float3  fogColor = float3(0.5, 0.6, 0.7);
	//return lerp(rgb, fogColor, fogAmount);

	float3  fogColor = float3(0.5, 0.6, 0.7);
	return lerp(rgb, fogColor, distance / 140000);
}
