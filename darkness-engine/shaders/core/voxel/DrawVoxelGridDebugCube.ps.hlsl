
cbuffer DrawVoxelGridDebugCubeConstants
{
	float4 color;
};

struct PSInput
{
	float4 position : SV_Position0;
};

float4 main(PSInput input) : SV_Target
{
	return color;
}
