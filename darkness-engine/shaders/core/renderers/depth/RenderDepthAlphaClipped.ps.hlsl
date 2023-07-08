
struct PSInput
{
	float4 position     : SV_Position0;
	float2 uv           : TEXCOORD0;
	uint albedoMaterial	: BLENDINDICES0;
};

Texture2D<float4> materialTextures[];
sampler tex_sampler;

void main(PSInput input) : SV_Target
{
	clip(materialTextures[NonUniformResourceIndex(input.albedoMaterial)].Sample(tex_sampler, input.uv).w - 0.35);
}
