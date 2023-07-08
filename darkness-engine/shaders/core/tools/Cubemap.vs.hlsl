
static const float3 vertices[36] =
{
	float3(-1.0, +1.0, -1.0),	// top		0
	float3(-1.0, +1.0, +1.0),
	float3(+1.0, +1.0, +1.0),
	float3(+1.0, +1.0, +1.0),
	float3(+1.0, +1.0, -1.0),			//	3
	float3(-1.0, +1.0, -1.0),
						
	float3(-1.0, -1.0, +1.0),	// bottom	4
	float3(-1.0, -1.0, -1.0),
	float3(+1.0, -1.0, -1.0),
	float3(+1.0, -1.0, -1.0),
	float3(+1.0, -1.0, +1.0),			//	7
	float3(-1.0, -1.0, +1.0),
						
	float3(-1.0, +1.0, +1.0),	// front	8
	float3(-1.0, -1.0, +1.0),
	float3(+1.0, -1.0, +1.0),
	float3(+1.0, -1.0, +1.0),
	float3(+1.0, +1.0, +1.0),			//	11
	float3(-1.0, +1.0, +1.0),
						
	float3(+1.0, +1.0, -1.0),	// back		12
	float3(+1.0, -1.0, -1.0),
	float3(-1.0, -1.0, -1.0),
	float3(-1.0, -1.0, -1.0),
	float3(-1.0, +1.0, -1.0),			//	15
	float3(+1.0, +1.0, -1.0),
						
	float3(+1.0, +1.0, +1.0),	// right	16
	float3(+1.0, -1.0, +1.0),
	float3(+1.0, -1.0, -1.0),
	float3(+1.0, -1.0, -1.0),
	float3(+1.0, +1.0, -1.0),			//	19
	float3(+1.0, +1.0, +1.0),
												 
	float3(-1.0, +1.0, -1.0),	// left		20
	float3(-1.0, -1.0, -1.0),
	float3(-1.0, -1.0, +1.0),
	float3(-1.0, -1.0, +1.0),
	float3(-1.0, +1.0, +1.0),			//	23
	float3(-1.0, +1.0, -1.0)
};

cbuffer ConstData
{
    float4x4 viewProjectionMatrix;
	float cameraFarPlane;
};

struct VSOutput
{
    float4 position         : SV_Position0;
    float4 pos              : NORMAL;
};

VSOutput main(uint id : SV_VertexID)
{
    VSOutput output;
	float w = cameraFarPlane;
	float a = sqrt((w * w) + (w * w)) * 0.4f;
    output.position = mul(viewProjectionMatrix, float4(vertices[id] * a, 1.0f));
    output.pos = normalize(float4(vertices[id], 1.0f));
    return output;
}
