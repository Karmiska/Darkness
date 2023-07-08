
struct PSInput
{
	float4 position         : SV_Position0;
	float2 uv               : TEXCOORD0;
};

Texture2D<float> srcDepth;

cbuffer CopyDepthConstants
{
	uint mip;
};

float main(PSInput input) : SV_Depth
{
	return srcDepth.Load(int3(input.position.xy, (int)mip));
}
