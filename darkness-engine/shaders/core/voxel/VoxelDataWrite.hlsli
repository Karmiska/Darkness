
RWTexture3D<uint> voxels;
RWTexture3D<float3> voxelNormals;
RWBuffer<uint> voxelColorgrid;

bool voxelGridHit(float3 pos)
{
	return voxels[int3(pos.x, pos.y, pos.z)];
}

#include "VoxelDataCommon.hlsli"
