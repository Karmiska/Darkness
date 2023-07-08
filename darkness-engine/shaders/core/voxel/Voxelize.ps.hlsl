#include "../Common.hlsli"
#include "../shared_types/InstanceMaterial.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float4 normal           : NORMAL0;
    float3 uv				: TEXCOORD0;
    uint instancePointer    : BLENDINDICES0;
};

struct PSOutput
{
    uint temp;
};

cbuffer VoxelizeConstants
{
    uint mode;
    uint3 padding;
};

#include "../voxel/VoxelDataWrite.hlsli"

RWBuffer<uint> voxelList;
RWBuffer<uint> voxelListCount;

StructuredBuffer<InstanceMaterial> instanceMaterials;
Texture2D<float4> materialTextures[];

sampler tex_sampler;

void main(PSInput input, bool isFrontFace : SV_IsFrontFace) : SV_Target
{
    float3 normal = input.normal.xyz;

    InstanceMaterial material = instanceMaterials[input.instancePointer];
    float2 materialScale = float2(material.scaleX, material.scaleY);

    float4 albedo = float4(material.color, 1.0f);

    if ((material.materialSet & 0x1) == 0x1)
    {
        float2 scaledUVLocal = input.uv.xy * materialScale;
        albedo = materialTextures[NonUniformResourceIndex(material.albedo)].Sample(tex_sampler, scaledUVLocal);
        if (albedo.w < 0.35)
            discard;
    }

    uint3 voxelPosition = uint3(0, 0, 0);

    //float sampDepth = input.uv.z;
    float sampDepth = input.uv.z;// EvaluateAttributeAtSample(input.position.z, sampleIndex);
    sampDepth = voxelDepth - (sampDepth * voxelDepth) + voxelDepth;

    // Z axis
    if (mode == 0)
    {
        voxelPosition = uint3(
            input.position.x,
            input.position.y,
            sampDepth);
    }

    // X axis
    if (mode == 1)
    {
        voxelPosition = uint3(
            sampDepth,
            input.position.y,
            voxelDepth - input.position.x);
    }

    // Y axis
    if (mode == 2)
    {
        voxelPosition = uint3(
            voxelDepth - input.position.x,
            sampDepth,
            input.position.y);
    }

    if (voxelPosition.x < voxelDepth &&
        voxelPosition.x > 0 &&
        voxelPosition.y < voxelDepth &&
        voxelPosition.y > 0 &&
        voxelPosition.z < voxelDepth &&
        voxelPosition.z > 0.0f)
    {
        voxels[voxelPosition] = 1;
        voxelNormals[voxelPosition] = normalize(normal.xyz);

        uint bufferIndex = bufferPosFromVoxel(voxelPosition);

        uint3 encodedColor = encodeColor(albedo.xyz);
        uint2 packedColor = packColor(encodedColor);
        packedColor.y |= 0x1;

        InterlockedAdd(voxelColorgrid[bufferIndex], packedColor.x);
        InterlockedAdd(voxelColorgrid[bufferIndex + 1], packedColor.y);

        uint voxelListPosition = 0;
        InterlockedAdd(voxelListCount[0], 1, voxelListPosition);
        voxelList[voxelListPosition] = bufferIndex;
    }
}
