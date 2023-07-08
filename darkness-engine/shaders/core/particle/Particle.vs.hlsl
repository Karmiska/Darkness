
Buffer<float3> positions;

cbuffer ConstData
{
    float4x4 jitterModelViewProjectionMatrix;
    float4x4 modelMatrix;
    float4 cameraPosition;
    float2 particleSize;
};

struct VSOutput
{
    float4 position     : SV_Position0;
    float4 normal       : NORMAL0;
    float4 tangent      : NORMAL1;
    float2 uv           : TEXCOORD0;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    uint quadId = id / 6;
    uint cornerId = id % 6;

    float2 halfSize = particleSize / 2.0f;
    float3 position = positions[quadId];
    float3 normal = normalize(cameraPosition.xyz - position);
    float3 tangent = normalize(cross(normal, float3(0.0f, 1.0f, 0.0f)));
    float3 bitangent = normalize(cross(normal, tangent));
    float2 uv = float2(0.0f, 0.0f);

    if (cornerId == 0)
    {
        position -= tangent * halfSize.x;
        position -= bitangent * halfSize.y;
        uv = float2(0.0f, 0.0f);
    }
    else if (cornerId == 1)
    {
        position -= tangent * halfSize.x;
        position += bitangent * halfSize.y;
        uv = float2(0.0f, 1.0f);
    }
    else if (cornerId == 2)
    {
        position += tangent * halfSize.x;
        position -= bitangent * halfSize.y;
        uv = float2(1.0f, 0.0f);
    }
    else if (cornerId == 3)
    {
        position += tangent * halfSize.x;
        position -= bitangent * halfSize.y;
        uv = float2(1.0f, 0.0f);
    }
    else if (cornerId == 4)
    {
        position -= tangent * halfSize.x;
        position += bitangent * halfSize.y;
        uv = float2(0.0f, 1.0f);
    }
    else if (cornerId == 5)
    {
        position += tangent * halfSize.x;
        position += bitangent * halfSize.y;
        uv = float2(1.0f, 1.0f);
    }

    output.position = mul(jitterModelViewProjectionMatrix, float4(position, 1.0f));
    output.normal = mul(modelMatrix, float4(normal, 0));
    output.tangent = mul(modelMatrix, float4(tangent, 0));
    output.uv = uv;
    return output;
}
