
cbuffer VoxelDataConstants
{
	float voxelDepth;
	float3 voxelGridPosition;

	float3 voxelGridSize;
	uint voxelMip;
};

uint bufferPosFromVoxel(uint3 voxelPos)
{
	//uint slice = depth * depth;
	//return ((slice * voxelPos.z) + ((depth * voxelPos.y) + voxelPos.x)) * 2;
	return encodeMorton(voxelPos) * 2;
}

uint3 encodeColor(float3 color)
{
	return uint3(
		color.x * 255,
		color.y * 255,
		color.z * 255);

}

uint2 packColor(uint3 color)
{
	return uint2(
		(color.r << 16) | (color.g),
		(color.b << 16));
}

uint4 unpackColor(uint2 color)
{
	return uint4(
		(color.x & 0xFFFF0000) >> 16,
		(color.x & 0x0000FFFF),
		(color.y & 0xFFFF0000) >> 16,
		(color.y & 0x0000FFFF));
}

float3 decodeColor(uint3 color)
{
	return float3(
		(float)color.r / 255,
		(float)color.g / 255,
		(float)color.b / 255);
}

float3 worldPositionFromThreadID(uint3 threadID)
{
	float3 dthreadID = float3(
		voxelDepth - threadID.x, 
		voxelDepth - threadID.y, 
		voxelDepth - threadID.z);

	float3 gridCellSize = voxelGridSize / voxelDepth;
	return voxelGridPosition + (dthreadID * gridCellSize);
}

float3 voxelGridCellSize()
{
	return voxelGridSize / voxelDepth;
}

bool insideVoxelGrid(float3 pos)
{
	return
		pos.x >= 0 && pos.x < voxelDepth &&
		pos.y >= 0 && pos.y < voxelDepth &&
		pos.z >= 0 && pos.z < voxelDepth;
}

bool voxelGridPositionFromWorldPosition(float3 worldPosition, out int3 result)
{
	float3 gridPos = worldPosition - voxelGridPosition;
	float3 gridCellSize = voxelGridSize / voxelDepth;
	result = int3(
		voxelDepth - (gridPos.x / gridCellSize.x), 
		voxelDepth - (gridPos.y / gridCellSize.y), 
		voxelDepth - (gridPos.z / gridCellSize.z));
	return insideVoxelGrid(float3(result.x, result.y, result.z));
}

void voxelGridPositionFromWorldPositionFloat(float3 worldPosition, out float3 result)
{
	float3 gridPos = worldPosition - voxelGridPosition;
	float3 gridCellSize = voxelGridSize / voxelDepth;
	result = float3(
		voxelDepth - (gridPos.x / gridCellSize.x),
		voxelDepth - (gridPos.y / gridCellSize.y),
		voxelDepth - (gridPos.z / gridCellSize.z));
}

float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), radicalInverse_VdC(i));
}

float3 importanceSampleGGX(float2 Xi, float3 N, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	float3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

/*bool gridMarch(int3 origin, float3 direction, out int3 hit)
{
	direction *= -1.0f;

	float3 orig = float3(origin.x, origin.y, origin.z);
	float3 pos = orig + direction;

	bool inside = insideVoxelGrid(pos);
	while (inside && !voxelGridHit(pos))
	{
		pos += direction * 0.1;
		inside = insideVoxelGrid(pos);
	}
	hit = int3(pos.x, pos.y, pos.z);
	return inside;
}*/


