
struct Vertex
{
    float3 vert;
};
StructuredBuffer<Vertex> vertex;
StructuredBuffer<Vertex> color;

cbuffer CameraMatrixes
{
	float4x4 viewProjectionMatrix;
};

struct VSOutput
{
	float4 position         : SV_Position0;
	float3 color			: COLOR0;
};

VSOutput main(uint id : SV_VertexID)
{
	VSOutput output;
	output.position = mul(
		viewProjectionMatrix,
		float4(
			vertex[id].vert ,
			1.0f));
	output.color = color[id / 24].vert;
	return output;
}
