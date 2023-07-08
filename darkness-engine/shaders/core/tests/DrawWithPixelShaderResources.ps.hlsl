
struct PSInput
{
    float4 position : SV_Position;
};

Buffer<float4> color;

float4 main(PSInput input) : SV_Target
{
    if(input.position.x >= 0.0f && input.position.x < 341.0f)
        return color[0];
    if (input.position.x >= 341.0f && input.position.x < 682.0f)
        return color[1];
    if (input.position.x >= 682.0f && input.position.x <= 1024.0f)
        return color[2];
    return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
