
cbuffer DrawRectangleConstants
{
	float4 color;
};

float4 main(float4 position : SV_Position0) : SV_Target
{
	return color;
}
