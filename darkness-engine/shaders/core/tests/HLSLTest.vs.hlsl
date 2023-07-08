
Buffer<float3> position;
Buffer<float4> color;

struct VSOutput
{
    float4 position : SV_Position;
    float4 color    : COLOR;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
    output.position = float4(position[id], 1.0f);
    output.color = color[id];

    return output;
};
