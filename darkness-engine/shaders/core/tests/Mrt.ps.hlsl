
struct PSInput
{
    float4 position : SV_Position;
};

struct PSOutput
{
    float4 target01;
    float4 target02;
    float4 target03;
    float4 target04;
};

PSOutput main(PSInput input) : SV_Target
{
    PSOutput output;
    output.target01 = float4(1.0f, 0.0f, 0.0f, 1.0f);
    output.target02 = float4(0.0f, 1.0f, 0.0f, 1.0f);
    output.target03 = float4(0.0f, 0.0f, 1.0f, 1.0f);
    output.target04 = float4(1.0f, 1.0f, 0.0f, 1.0f);
    return output;
}
