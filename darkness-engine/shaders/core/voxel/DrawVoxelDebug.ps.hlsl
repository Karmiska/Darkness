
struct PSInput
{
    float4 position : SV_Position0;
    float3 color			: COLOR0;
};

float4 main(PSInput input) : SV_Target
{
    return float4(input.color, 1.0f);
}
