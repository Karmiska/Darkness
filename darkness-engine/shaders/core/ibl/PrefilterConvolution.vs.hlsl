
static const float4 vertices[36] =
{
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(1.0f, 1.0f,-1.0f,  1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4(1.0f,-1.0f, 1.0f,  1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(1.0f,-1.0f,-1.0f,  1.0f),
    float4(1.0f, 1.0f,-1.0f,  1.0f),
    float4(1.0f,-1.0f,-1.0f,  1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4(1.0f,-1.0f, 1.0f,  1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4(1.0f,-1.0f, 1.0f,  1.0f),
    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(1.0f,-1.0f,-1.0f,  1.0f),
    float4(1.0f, 1.0f,-1.0f,  1.0f),
    float4(1.0f,-1.0f,-1.0f,  1.0f),
    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(1.0f,-1.0f, 1.0f,  1.0f),
    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(1.0f, 1.0f,-1.0f,  1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(1.0f, 1.0f, 1.0f,  1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(1.0f,-1.0f, 1.0f,  1.0f)
};

cbuffer ConstData
{
    float4x4 viewProjectionMatrix;
};

struct VSOutput
{
    float4 position         : SV_Position0;
    float4 pos              : NORMAL;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    output.position = mul(viewProjectionMatrix, vertices[id]);
    output.pos = normalize(vertices[id]);
    return output;
}
