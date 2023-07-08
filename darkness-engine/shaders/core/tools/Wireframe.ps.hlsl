
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float4 color            : COLOR0;
};

cbuffer ConstData
{
    float4 color;
};

sampler tex_sampler;

[earlydepthstencil]
float4 main(PSInput input) : SV_Target
{
    return float4(input.color.xyz, 1.0);
}
