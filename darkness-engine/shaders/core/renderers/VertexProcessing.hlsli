const float4 invalid = float4(
	asfloat(0x7f800001),
	asfloat(0x7f800001),
	asfloat(0x7f800001),
	asfloat(0x7f800001));

VSOutput output;
if (id != 0xffffffff)
{
	uint clusterId = (id & 0xffff0000) >> 16;
	uint clusterIndex = id & 0x0000ffff;
	ClusterInstanceData cInstanceData = clusters[rootConstants.clusterStart + clusterId];
	ClusterData cdata = clusterData[cInstanceData.clusterPointer];

	uint vertexDataIndex = cdata.vertexPointer + clusterIndex;

	// uint uvDataIndex = cInstanceData.uvPointer + clusterIndex;
	uint uvDataIndex = cdata.vertexPointer + clusterIndex;

	TransformHistory transforms = transformHistory[cInstanceData.instancePointer];
	float4x4 normalTransform = transpose(transforms.inverseTransform);

	float3 vertex = unpackVertex(vertices[vertexDataIndex], scales[cInstanceData.instancePointer]);

	output.position = mul(mul(jitterViewProjectionMatrix, transforms.transform), float4(vertex, 1));
	output.mvPosCurrent = mul(mul(viewProjectionMatrix, transforms.transform), float4(vertex, 1));
	output.mvPosPrevious = mul(mul(previousViewProjectionMatrix, transforms.previousTransform), float4(vertex, 1));
	output.normal = normalize(mul(normalTransform, float4(unpackNormalOctahedron(normals[vertexDataIndex]), 0.0f)));
	output.tangent = normalize(mul(normalTransform, float4(unpackNormalOctahedron(tangents[vertexDataIndex]), 0.0f)));
	output.instancePointer = cInstanceData.instancePointer;
#ifdef OPTION_DEBUG
	if (debugMode == DEBUG_MODE_TRIANGLES)
		output.clusterPointer = (cdata.indexPointer + id) / 3;
	else if (debugMode == DEBUG_MODE_CLUSTERS)
		output.clusterPointer = cInstanceData.clusterPointer;
	else
		output.clusterPointer = 0;
#endif

	float2 uu = flipVerticalUV(uv[uvDataIndex]);
	output.normal.w = uu.x;
	output.tangent.w = uu.y;

	return output;
}

output.position = invalid;
output.mvPosCurrent = invalid;
output.mvPosPrevious = invalid;
output.normal = invalid;
output.tangent = invalid;
output.instancePointer = 0;
#ifdef OPTION_DEBUG
output.clusterPointer = 0;
#endif
return output;
