
#define MAX_MESHLET_SIZE 128 
#define GROUP_SIZE MAX_MESHLET_SIZE 

struct Meshlet
{
    uint32_t VertCount;
    uint32_t VertOffset;
    uint32_t PrimCount;
    uint32_t PrimOffset;

    float3 AABBMin;
    float3 AABBMax;
    float4 NormalCone;
};

struct MeshInfo
{
    uint32_t IndexBytes;
    uint32_t MeshletCount;
    uint32_t LastMeshletSize;
};

cbuffer Constants
{
    float4x4    View;
    float4x4    ViewProj;
};

cbuffer Instance
{
    float4x4 World;
    float4x4 WorldInvTrans;
};

cbuffer MeshInfo
{
    uint IndexBytes;
};

struct Vertex
{
    float3 Position;
    float3 Normal;
};

StructuredBuffer<Vertex>    Vertices;
StructuredBuffer<Meshlet>   Meshlets;
ByteAddressBuffer           UniqueVertexIndices;
StructuredBuffer<uint>      PrimitiveIndices;

uint3 GetPrimitive(Meshlet m, uint index)
{
    uint primitiveIndex = PrimitiveIndices[m.PrimOffset + index];
    return uint3(primitiveIndex & 0x3FF, (primitiveIndex >> 10) & 0x3FF, (primitiveIndex >> 20) & 0x3FF);
}

uint GetVertexIndex(Meshlet m, uint localIndex)
{
    localIndex = m.VertOffset + localIndex;
    if (IndexBytes == 4) // 32-bit Vertex Indices 
    {
        return UniqueVertexIndices.Load(localIndex * 4);
    }
    else // 16-bit Vertex Indices 
    {
        // Byte address must be 4-byte aligned. 
        uint wordOffset = (localIndex & 0x1);
        uint byteOffset = (localIndex / 2) * 4;

        // Grab the pair of 16-bit indices, shift & mask off proper 16-bits. 
        uint indexPair = UniqueVertexIndices.Load(byteOffset);
        uint index = (indexPair >> (wordOffset * 16)) & 0xffff;

        return index;
    }
}

struct VertexOut
{
    float4 PositionVS   : SV_Position0;
    float4 PositionHS   : POSITION0;
    float3 Normal       : NORMAL0;
    uint MeshletIndex   : BLENDINDICES0;
};

VertexOut GetVertexAttributes(uint meshletIndex, uint vertexIndex)
{
    Vertex v = Vertices[vertexIndex];

    float4 positionWS = mul(float4(v.Position, 1), World);

    VertexOut vout;
    vout.PositionVS = mul(positionWS, View);
    vout.PositionHS = mul(positionWS, ViewProj);
    vout.Normal = mul(float4(v.Normal, 0), WorldInvTrans).xyz;
    vout.MeshletIndex = meshletIndex;

    return vout;
}

[outputtopology("triangle")]
[NumThreads(128, 1, 1)]
void main(
    in uint3 gtid : SV_GroupThreadID,
    in uint3 gid : SV_GroupID,
    out indices uint3 triangles[128],
    out vertices VertexOut verts[128]
)
{
    Meshlet m = Meshlets[gid.x];
    SetMeshOutputCounts(m.VertCount, m.PrimCount);
    if (gtid.x < m.PrimCount)
    {
        triangles[gtid.x] = GetPrimitive(m, gtid.x);
    }

    if (gtid.x < m.VertCount)
    {
        uint vertexIndex = GetVertexIndex(m, gtid.x);
        verts[gtid.x] = GetVertexAttributes(gid.x, vertexIndex);
    }
}
