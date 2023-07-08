
struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

Texture2D<float4> image;
sampler imageSampler;

void main(PSInput input)
{
    clip(image.SampleLevel(imageSampler, input.uv, 0).w - 0.8);
}
