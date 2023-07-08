
struct PSInput
{
    float4 position : SV_Position;
    float4 uv : TEXCOORD0;
};

Texture2D<float4> tex;
sampler tex_sampler;

float4 main(PSInput input) : SV_Target
{
    // for normal map
    //return tex.SampleLevel(tex_sampler, input.uv.xy, 0.0f) * 40.0f;

    float4 sampl = tex.SampleLevel(tex_sampler, input.uv.xy, 0.0f);
    return float4(sampl.x, sampl.y, sampl.z, 1.0f);
}
