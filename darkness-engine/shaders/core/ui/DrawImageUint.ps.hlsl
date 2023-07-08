Texture2D<uint> image;

struct PSInput
{
	float4 position : SV_Position;
	float4 uv		: TEXCOORD0;
};

cbuffer PSDrawRectangleConstants
{
	float2 size;
	float2 padding;
};

float4 main(PSInput input) : SV_Target
{
	float4 val = image.Load(uint3(
		(uint)(input.uv.x * size.x),
		(uint)(input.uv.y * size.y), 0)) / 256.0;
	return float4(val.x, val.x, val.x, val.x);
}
