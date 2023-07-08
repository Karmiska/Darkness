
struct VSOutput
{
	float4 position : SV_Position;
	float4 uv : TEXCOORD0;
};

cbuffer DrawRectangleConstants
{
	float2 screenSize;
	float x;
	float y;

	float width;
	float height;
	float2 padding;
};

VSOutput main(uint id : SV_VertexID)
{
	VSOutput output;
	output.uv = float4((id << 1) & 2, id & 2, 0, 0) / 2.0;
	float2 uv = output.uv.xy;
	uv *= float2(width, height) / screenSize;
	uv += float2(x, y) / screenSize;
	output.position = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1) / float4(1.0, 1.0, 1, 1);
	output.uv = float4(float2(output.uv.x, output.uv.y), 0, 0);
	return output;
}
