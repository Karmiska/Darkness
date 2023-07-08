
Texture3D<uint> voxels;
RWTexture3D<uint> voxelMip1;
RWTexture3D<uint> voxelMip2;
RWTexture3D<uint> voxelMip3;
RWTexture3D<uint> voxelMip4;

cbuffer DownsampleBloom4Constants
{
    float2 g_inverseDimensions;
}

groupshared uint g_Tile[512];    // 8x8 input pixels

[numthreads(8, 8, 8)]
void main(uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID)
{
    uint parity = DTid.x | DTid.y | DTid.z;

    uint voxel = 
        voxels[DTid.xyz * 2 + uint3(0, 0, 0)] | voxels[DTid.xyz * 2 + uint3(1, 0, 0)] | voxels[DTid.xyz * 2 + uint3(0, 1, 0)] | voxels[DTid.xyz * 2 + uint3(1, 1, 0)] |
        voxels[DTid.xyz * 2 + uint3(0, 0, 1)] | voxels[DTid.xyz * 2 + uint3(1, 0, 1)] | voxels[DTid.xyz * 2 + uint3(0, 1, 1)] | voxels[DTid.xyz * 2 + uint3(1, 1, 1)];

    g_Tile[GI] = voxel;
    voxelMip1[DTid.xyz] = voxel;

    GroupMemoryBarrierWithGroupSync();

    if ((parity & 1) == 0)
    {
        voxel = 
            g_Tile[GI + 0] | g_Tile[GI + 1] | g_Tile[GI + 8] | g_Tile[GI + 9] |
            g_Tile[GI + 64] | g_Tile[GI + 64 + 1] | g_Tile[GI + 64 + 8] | g_Tile[GI + 64 + 9];

        g_Tile[GI] = voxel;
        voxelMip2[DTid.xyz >> 1] = voxel;
    }

    GroupMemoryBarrierWithGroupSync();

    if ((parity & 3) == 0)
    {
        voxel =
            g_Tile[GI + 0] | g_Tile[GI + 2] | g_Tile[GI + 16] | g_Tile[GI + 18] |
            g_Tile[GI + 128] | g_Tile[GI + 128 + 2] | g_Tile[GI + 128 + 16] | g_Tile[GI + 128 + 18];

        g_Tile[GI] = voxel;
        voxelMip3[DTid.xyz >> 2] = voxel;
    }

    GroupMemoryBarrierWithGroupSync();

    if ((parity & 7) == 0)
    {
        voxel =
            g_Tile[GI + 0] | g_Tile[GI + 4] | g_Tile[GI + 32] | g_Tile[GI + 36] |
            g_Tile[GI + 256] | g_Tile[GI + 256 + 4] | g_Tile[GI + 256 + 32] | g_Tile[GI + 256 + 36];

        voxelMip4[DTid.xyz >> 3] = voxel;
    }
}
