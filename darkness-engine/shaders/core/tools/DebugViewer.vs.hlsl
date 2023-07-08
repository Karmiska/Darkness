
static const float2 vertices[4] =
{
    float2(-1,  1),
    float2(1,  1),
    float2(-1, -1),
    float2(1, -1)
};

static const float2 uv[4] =
{
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f),
    float2(0.0f, 1.0f),
    float2(1.0f, 1.0f),
};

cbuffer ConstData
{
    float width;
    float height;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 uv : TEXCOORD0;
};

VSOutput main(uint id : SV_VertexID)
{
	float xp = width / 1.2f;//width / 200000; // 0.003;// 4.0f / width;
	float yp = (height / width) * xp;//height / 200000; // 0.002;// 2.0f / height;

    float kx = xp / width;
    float ky = yp / height;

    VSOutput output;
    output.position = float4(vertices[id % 4] * float2(kx, ky), 0, 1);
    output.uv = float4(uv[id % 4], 0, 0);

    //output.position += float4(0.75f, -0.30f, 0.0f, 0.0f);

	float2 inv = float2(1.0f / width, 1.0f / height);
	// output.position += float4(1.0f - kx, 1.0f - ky, 0.0f, 0.0f); // top right corner
	output.position += float4(1.0f - kx, -(1.0f - ky), 0.0f, 0.0f); // bottom right corner
    return output;
}
