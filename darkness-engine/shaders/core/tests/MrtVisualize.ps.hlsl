
struct PSInput
{
    float4 position : SV_Position;
    float4 uv : TEXCOORD0;
    uint texid : BLENDINDICES0;
};

sampler tex_sampler;

Texture2D<float4> mrt[];

float4 main(PSInput input) : SV_Target
{
    return mrt[input.texid].SampleLevel(tex_sampler, input.uv.xy, 0.0f);
}
