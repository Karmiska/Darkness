
struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

cbuffer ConstData
{
    float width;
    float height;
};

Texture2D<float4> color;
Texture2D<float> alpha;

sampler tex_sampler;

float4 main(PSInput input) : SV_Target
{
    float4 accum = color.SampleLevel(tex_sampler, input.uv, 0);
    float reveal = alpha.Load(int3(input.uv.x * width, input.uv.y * height, 0)).r;
    
    return float4(accum.rgb / clamp(accum.a, 1e-9, 5e9), reveal);
}
