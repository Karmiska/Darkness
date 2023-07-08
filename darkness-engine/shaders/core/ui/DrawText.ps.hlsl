
Texture2D<float> image;
sampler imageSampler;

cbuffer UiDrawTextConstants
{
	float4 color;
};

struct PSInput
{
	float4 position : SV_Position;
	float4 uv		: TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
	// float4(input.uv.xy, 0, 1);
	float col = image.SampleLevel(imageSampler, input.uv.xy, 0).x;
	return float4(color.xyz, col);
}
