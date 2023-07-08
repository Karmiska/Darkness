
static const float3 vertices[24] =
{
	float3(-1.0, +1.0, -1.0),	// top		0
	float3(-1.0, +1.0, +1.0),
	float3(+1.0, +1.0, +1.0),
	float3(+1.0, +1.0, -1.0),

	float3(-1.0, -1.0, +1.0),	// bottom	4
	float3(-1.0, -1.0, -1.0),
	float3(+1.0, -1.0, -1.0),
	float3(+1.0, -1.0, +1.0),

	float3(-1.0, +1.0, +1.0),	// front	8
	float3(-1.0, -1.0, +1.0),
	float3(+1.0, -1.0, +1.0),
	float3(+1.0, +1.0, +1.0),

	float3(+1.0, +1.0, -1.0),	// back		12
	float3(+1.0, -1.0, -1.0),
	float3(-1.0, -1.0, -1.0),
	float3(-1.0, +1.0, -1.0),

	float3(+1.0, +1.0, +1.0),	// right	16
	float3(+1.0, -1.0, +1.0),
	float3(+1.0, -1.0, -1.0),
	float3(+1.0, +1.0, -1.0),

	float3(-1.0, +1.0, -1.0),	// left		20
	float3(-1.0, -1.0, -1.0),
	float3(-1.0, -1.0, +1.0),
	float3(-1.0, +1.0, +1.0)
};

static const uint indexes[36] =
{
	0, 1, 2, 2, 3, 0,		// top
	4, 5, 6, 6, 7, 4,		// bottom
	8, 9, 10, 10, 11, 8,	// front
	12, 13, 14, 14, 15, 12,	// back
	16, 17, 18, 18, 19, 16,	// right
	20, 21, 22, 22, 23, 20
};

cbuffer RenderConesConstants
{
	float4x4 jitterViewProjectionMatrix;
	float3 cornera;
	float padding;
	float3 cornerb;
	float padding2;
};

struct VSOutput
{
	float4 position : SV_Position0;
};

VSOutput main(uint id : SV_VertexID)
{
	float3 cvert = vertices[indexes[id % 36]];
	cvert += 1.0f;
	cvert /= 2.0f;
	cvert *= cornerb - cornera;
	cvert += cornera;

	VSOutput output;
	output.position = mul(jitterViewProjectionMatrix, float4(cvert, 1.0f));
	
	return output;
}
