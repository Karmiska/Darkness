
struct PSInput
{
    float4 position : SV_Position;
    float4 uv : TEXCOORD0;
    float mip : PSIZE0;
    uint slice : BLENDINDICES0;
};

Texture2DArray<float4> textureArray;

sampler samp;

float4 main(PSInput input) : SV_Target
{
    
    return textureArray.SampleLevel(samp, float3(input.uv.xy, (float)input.slice), input.mip);
}
