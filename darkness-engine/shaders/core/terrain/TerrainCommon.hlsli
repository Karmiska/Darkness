
int3 getCameraSnapPosition(float3 cameraPosition, float3 cellSize)
{
	return int3(floor(cameraPosition / cellSize)) * cellSize;
}

float3 getVertex(int vertexId)
{
	int z = vertexId / VertexCount.x;
	int x = vertexId - (VertexCount.x * z);
	return float3(x, 0.0f, z);
}

float3 getVertexWorldPosition(
	float3 vertex,
	float3 nodeCount,
	float3 sectorSize,
	float3 cellSize,
	int3 cameraSnapPosition,
	uint2 cellLocation,
	float3 offset)
{
	float3 sectorWorldPosition = float3(cameraSnapPosition) - float3(sectorSize.x / 2.0f, 0.0f, sectorSize.z / 2.0f);
	float3 cellWorldPosition = sectorWorldPosition + float3(cellLocation.x * cellSize.x, 0.0f, cellLocation.y * cellSize.z);
	float3 nodePosition = float3(
		(vertex.x + offset.x) / nodeCount.x * cellSize.x,
		(vertex.y + offset.y),
		(vertex.z + offset.z) / nodeCount.z * cellSize.z);
	return cellWorldPosition + nodePosition;
}

float getHeightData(float3 worldSize, float3 vertexWorldPosition, out float2 posUV, out float4 height)
{
	float2 heightMapLocation = (vertexWorldPosition).xz;
	float2 heightUV = heightMapLocation / worldSize.xz;
	float2 heightTexelPos = heightUV * float2(heightMapSize);
	float2 topLeft = floor(heightTexelPos);
	float2 gatherCenter = topLeft + float2(0.5f, 0.5f);
	posUV = gatherCenter / float2(heightMapSize);

	// interpolated height
	// this really is needed only if triangle size is smaller than
	// the actual resolution of the height map.
	//float2 posFrac = heightTexelPos - topLeft;
	//height = heightmap.Gather(heightMapSampler, float2(posUV.x, posUV.y));
	//float t = lerp(height.w, height.z, posFrac.x);
	//float b = lerp(height.x, height.y, posFrac.x);
	//float interpolatedHeight = lerp(t, b, posFrac.y);

	// height without interpolation
	float interpolatedHeight = heightmap.SampleLevel(heightMapSampler, posUV, 0);

	return interpolatedHeight;
}
