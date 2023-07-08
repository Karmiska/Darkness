
static const float4 vertices[36] =
{
    // POS X
    float4( 1.0f, 1.0f,-1.0f, 1.0f),
    float4( 1.0f,-1.0f,-1.0f, 1.0f),
    float4( 1.0f, 1.0f, 1.0f, 1.0f),

    float4( 1.0f, 1.0f, 1.0f, 1.0f),
    float4( 1.0f,-1.0f,-1.0f, 1.0f),
    float4( 1.0f,-1.0f, 1.0f, 1.0f),

    // NEG X
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),

    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),

    // POS Y
    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4( 1.0f, 1.0f, 1.0f, 1.0f),

    float4( 1.0f, 1.0f, 1.0f, 1.0f),
    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4( 1.0f, 1.0f,-1.0f, 1.0f),

    // NEG Y
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4( 1.0f,-1.0f,-1.0f, 1.0f),

    float4( 1.0f,-1.0f,-1.0f, 1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),
    float4( 1.0f,-1.0f, 1.0f, 1.0f),

    // POS Z
    float4( 1.0f, 1.0f, 1.0f, 1.0f),
    float4( 1.0f,-1.0f, 1.0f, 1.0f),
    float4(-1.0f, 1.0f, 1.0f, 1.0f),

    float4(-1.0f, 1.0f, 1.0f, 1.0f),
    float4( 1.0f,-1.0f, 1.0f, 1.0f),
    float4(-1.0f,-1.0f, 1.0f, 1.0f),

    // NEG Z
    float4(-1.0f, 1.0f,-1.0f, 1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4( 1.0f, 1.0f,-1.0f, 1.0f),

    float4( 1.0f, 1.0f,-1.0f, 1.0f),
    float4(-1.0f,-1.0f,-1.0f, 1.0f),
    float4( 1.0f,-1.0f,-1.0f, 1.0f)
};

cbuffer ConstData
{
    float4x4 viewProjectionMatrix;
};

struct VSOutput
{
    float4 position : SV_Position0;
    float4 normal   : NORMAL;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    output.position = mul(viewProjectionMatrix, vertices[id]);
    output.normal = vertices[id];
    return output;
}
