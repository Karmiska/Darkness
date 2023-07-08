#include "../Common.hlsli"

Buffer<uint> voxelList;
Buffer<uint> voxelListCount;

#include "VoxelDataWrite.hlsli"

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (dispatchThreadID.x < voxelListCount[0])
    {
        uint voxelLoc = voxelList[dispatchThreadID.x];
        uint3 voxelPosition = decodeMorton(voxelLoc / 2);
        
        voxels[voxelPosition] = 0;
        voxelNormals[voxelPosition] = float3(0, 0, 0);
        voxelColorgrid[voxelLoc] = 0;
        voxelColorgrid[voxelLoc + 1] = 0;
    }
}
