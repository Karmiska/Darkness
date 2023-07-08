#include "../Common.hlsli"

struct Vertex
{
	float3 vert;
};

#include "VoxelDataRead.hlsli"

RWStructuredBuffer<Vertex> vertex;
RWStructuredBuffer<Vertex> color;
RWBuffer<uint> index;
RWBuffer<uint> allocations;

void createVertices(uint location, float3 center, float3 h)
{
	vertex[location].vert = float3(center.x - h.x, center.y + h.y, center.z - h.z);	// top		0
	vertex[location + 1].vert = float3(center.x - h.x, center.y + h.y, center.z + h.z);
	vertex[location + 2].vert = float3(center.x + h.x, center.y + h.y, center.z + h.z);
	vertex[location + 3].vert = float3(center.x + h.x, center.y + h.y, center.z - h.z);			//	3
	vertex[location + 4].vert = float3(center.x - h.x, center.y - h.y, center.z + h.z);	// bottom	4
	vertex[location + 5].vert = float3(center.x - h.x, center.y - h.y, center.z - h.z);
	vertex[location + 6].vert = float3(center.x + h.x, center.y - h.y, center.z - h.z);
	vertex[location + 7].vert = float3(center.x + h.x, center.y - h.y, center.z + h.z);			//	7
	vertex[location + 8].vert = float3(center.x - h.x, center.y + h.y, center.z + h.z);	// front	8
	vertex[location + 9].vert = float3(center.x - h.x, center.y - h.y, center.z + h.z);
	vertex[location + 10].vert = float3(center.x + h.x, center.y - h.y, center.z + h.z);
	vertex[location + 11].vert = float3(center.x + h.x, center.y + h.y, center.z + h.z);			//	11
	vertex[location + 12].vert = float3(center.x + h.x, center.y + h.y, center.z - h.z);	// back		12
	vertex[location + 13].vert = float3(center.x + h.x, center.y - h.y, center.z - h.z);
	vertex[location + 14].vert = float3(center.x - h.x, center.y - h.y, center.z - h.z);
	vertex[location + 15].vert = float3(center.x - h.x, center.y + h.y, center.z - h.z);			//	15
	vertex[location + 16].vert = float3(center.x + h.x, center.y + h.y, center.z + h.z);	// right	16
	vertex[location + 17].vert = float3(center.x + h.x, center.y - h.y, center.z + h.z);
	vertex[location + 18].vert = float3(center.x + h.x, center.y - h.y, center.z - h.z);
	vertex[location + 19].vert = float3(center.x + h.x, center.y + h.y, center.z - h.z);			//	19
	vertex[location + 20].vert = float3(center.x - h.x, center.y + h.y, center.z - h.z);	// left		20
	vertex[location + 21].vert = float3(center.x - h.x, center.y - h.y, center.z - h.z);
	vertex[location + 22].vert = float3(center.x - h.x, center.y - h.y, center.z + h.z);
	vertex[location + 23].vert = float3(center.x - h.x, center.y + h.y, center.z + h.z);		//	23
}

void createIndexes(uint indexLocation, uint vertexLocation)
{
	index[indexLocation] = vertexLocation + 0;
	index[indexLocation + 1] = vertexLocation + 1;
	index[indexLocation + 2] = vertexLocation + 2;
	index[indexLocation + 3] = vertexLocation + 2;
	index[indexLocation + 4] = vertexLocation + 3;
	index[indexLocation + 5] = vertexLocation + 0;		// top
	index[indexLocation + 6] = vertexLocation + 4;
	index[indexLocation + 7] = vertexLocation + 5;
	index[indexLocation + 8] = vertexLocation + 6;
	index[indexLocation + 9] = vertexLocation + 6;
	index[indexLocation + 10] = vertexLocation + 7;
	index[indexLocation + 11] = vertexLocation + 4;		// bottom
	index[indexLocation + 12] = vertexLocation + 8;
	index[indexLocation + 13] = vertexLocation + 9;
	index[indexLocation + 14] = vertexLocation + 10;
	index[indexLocation + 15] = vertexLocation + 10;
	index[indexLocation + 16] = vertexLocation + 11;
	index[indexLocation + 17] = vertexLocation + 8;		// front
	index[indexLocation + 18] = vertexLocation + 12;
	index[indexLocation + 19] = vertexLocation + 13;
	index[indexLocation + 20] = vertexLocation + 14;
	index[indexLocation + 21] = vertexLocation + 14;
	index[indexLocation + 22] = vertexLocation + 15;
	index[indexLocation + 23] = vertexLocation + 12;		// back
	index[indexLocation + 24] = vertexLocation + 16;
	index[indexLocation + 25] = vertexLocation + 17;
	index[indexLocation + 26] = vertexLocation + 18;
	index[indexLocation + 27] = vertexLocation + 18;
	index[indexLocation + 28] = vertexLocation + 19;
	index[indexLocation + 29] = vertexLocation + 16;		// right
	index[indexLocation + 30] = vertexLocation + 20;
	index[indexLocation + 31] = vertexLocation + 21;
	index[indexLocation + 32] = vertexLocation + 22;
	index[indexLocation + 33] = vertexLocation + 22;
	index[indexLocation + 34] = vertexLocation + 23;
	index[indexLocation + 35] = vertexLocation + 20;
}

[numthreads(8, 8, 8)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint bufferIndex = bufferPosFromVoxel(dispatchThreadID * ((uint)1 << voxelMip));
	uint2 gridColor = uint2(voxelColorgrid[bufferIndex], voxelColorgrid[bufferIndex + 1]);

	//if (gridColor.y)
	if (voxels[dispatchThreadID.xyz])
	{
		uint cubeLocation = 0;
		InterlockedAdd(allocations[0], 1, cubeLocation);

		float3 worldPosition = worldPositionFromThreadID(dispatchThreadID);
		float3 gridCellSize = voxelGridCellSize();

		createVertices(
			cubeLocation * 24,
			worldPosition - (gridCellSize / 2),
			gridCellSize / 2);

		createIndexes(
			cubeLocation * 36,
			cubeLocation * 24);

		uint4 unpackedcolor = unpackColor(gridColor);
		float3 col = decodeColor(unpackedcolor.xyz);
		float count = (float)unpackedcolor.w;

		color[cubeLocation].vert = col / count;

	}
}
