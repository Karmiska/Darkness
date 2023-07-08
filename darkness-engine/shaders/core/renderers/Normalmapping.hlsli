
float3 createNormalmapNormal(float3x3 TBN, float3 normalMapSample)
{
	float3 remappedNormal = (normalMapSample * 2.0f) - 1.0f;
	float3 normalMapNormal = normalize(float3(remappedNormal.x, remappedNormal.y, remappedNormal.z));
	return mul(TBN, normalMapNormal);
}

float3 normalMap(
	bool isFrontFace, 
	bool mirrorUV, 
	float3 normalMapNormal, 
	float3 surfaceNormal, 
	float3 surfaceTangent)
{
	float3 normal = surfaceNormal;
	float3 tangent = surfaceTangent;
	if (!isFrontFace)
		normal *= -1.0f;

	float mirrorUVMul = -1.0f;
	if (mirrorUV)
		mirrorUVMul = 1.0f;

	// create normal from normal map sample
	float3x3 TBN = float3x3(
		normalize(tangent),
		// TODO: this normalize is propably not needed
		normalize(cross(tangent, normal) * mirrorUVMul),
		normalize(normal));
	TBN = transpose(TBN);
	
	return createNormalmapNormal(TBN, normalMapNormal);
}
