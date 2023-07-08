
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

/*
static const float2 uv[36] = {
float2(0.000059f, 1.0f - 0.000004f),
float2(0.000103f, 1.0f - 0.336048f),
float2(0.335973f, 1.0f - 0.335903f),
float2(1.000023f, 1.0f - 0.000013f),
float2(0.667979f, 1.0f - 0.335851f),
float2(0.999958f, 1.0f - 0.336064f),
float2(0.667979f, 1.0f - 0.335851f),
float2(0.336024f, 1.0f - 0.671877f),
float2(0.667969f, 1.0f - 0.671889f),
float2(1.000023f, 1.0f - 0.000013f),
float2(0.668104f, 1.0f - 0.000013f),
float2(0.667979f, 1.0f - 0.335851f),
float2(0.000059f, 1.0f - 0.000004f),
float2(0.335973f, 1.0f - 0.335903f),
float2(0.336098f, 1.0f - 0.000071f),
float2(0.667979f, 1.0f - 0.335851f),
float2(0.335973f, 1.0f - 0.335903f),
float2(0.336024f, 1.0f - 0.671877f),
float2(1.000004f, 1.0f - 0.671847f),
float2(0.999958f, 1.0f - 0.336064f),
float2(0.667979f, 1.0f - 0.335851f),
float2(0.668104f, 1.0f - 0.000013f),
float2(0.335973f, 1.0f - 0.335903f),
float2(0.667979f, 1.0f - 0.335851f),
float2(0.335973f, 1.0f - 0.335903f),
float2(0.668104f, 1.0f - 0.000013f),
float2(0.336098f, 1.0f - 0.000071f),
float2(0.000103f, 1.0f - 0.336048f),
float2(0.000004f, 1.0f - 0.671870f),
float2(0.336024f, 1.0f - 0.671877f),
float2(0.000103f, 1.0f - 0.336048f),
float2(0.336024f, 1.0f - 0.671877f),
float2(0.335973f, 1.0f - 0.335903f),
float2(0.667969f, 1.0f - 0.671889f),
float2(1.000004f, 1.0f - 0.671847f),
float2(0.667979f, 1.0f - 0.335851f)
};*/

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
