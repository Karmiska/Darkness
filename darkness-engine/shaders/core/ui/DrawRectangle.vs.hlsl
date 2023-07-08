
cbuffer DrawRectangleConstants
{
	float2 screenSize;
	float x;
	float y;
	
	float width;
	float height;
	float2 padding;
};

float4 main(uint id : SV_VertexID) : SV_Position
{
	float2 uv = float2((id << 1) & 2, id & 2) / 2.0f;
	uv *= float2(width, height) / screenSize;
	uv += float2(x, y) / screenSize;
	return float4(uv * float2(2, -2) + float2(-1, 1), 0, 1) / float4(1.0, 1.0, 1, 1);
}
