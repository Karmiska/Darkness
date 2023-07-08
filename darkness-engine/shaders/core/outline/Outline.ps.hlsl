
struct PSInput
{
    float4 position     : SV_Position0;
    float4 colora       : COLOR0;
    float edgeFlag      : PSIZE;
};

cbuffer ConstData
{
    float4 color;
    float2 inverseSize;
};

Texture2D<float> depth;
sampler depth_sampler;

float4 main(PSInput input) : SV_Target
{
    float2 screenPos = input.position.xy;
    float2 uv = screenPos * inverseSize;
    float depthSample = depth.Sample(depth_sampler, uv);
    if (depthSample - 0.00005 > input.position.z)
    {
        return float4(input.colora.xyz * 0.05f, 1.0f);
    }
    else
        return input.colora;
}
