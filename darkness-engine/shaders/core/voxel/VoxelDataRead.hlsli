
Texture3D<uint> voxels;
Texture3D<float3> voxelNormals;
Buffer<uint> voxelColorgrid;

bool voxelGridHit(float3 pos)
{
	return voxels[int3(pos.x, pos.y, pos.z)];
}

#include "VoxelDataCommon.hlsli"
