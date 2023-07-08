
Texture2D<float4> image;
sampler imageSampler;

struct PSInput
{
	float4 position : SV_Position;
	float4 uv		: TEXCOORD0;
};

float4 main(PSInput input) : SV_Target
{
	return image.SampleLevel(imageSampler, input.uv.xy, 0);
}
