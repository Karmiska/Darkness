
Buffer<float3> vertexes;

cbuffer ShapeRenderConstants
{
	float4x4 jitterViewProjectionMatrix;
};

struct VSOutput
{
	float4 position     : SV_Position0;
};

VSOutput main(uint id : SV_VertexID)
{
	VSOutput output;
	output.position = mul(jitterViewProjectionMatrix, float4(vertexes[id], 1.0f));
	return output;
}
