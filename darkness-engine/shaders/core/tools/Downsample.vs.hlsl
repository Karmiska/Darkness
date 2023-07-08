
struct VSOutput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    output.uv = float2((id << 1) & 2, id & 2);
    output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);
    return output;
}
