
cbuffer Constants
{
	float4x4 viewMatrix;
	float4 topLeftViewRay;
	float4 topRightViewRay;
	float4 bottomLeftViewRay;
	float4 bottomRightViewRay;
};

struct VSOutput
{
	float4 position         : SV_Position0;
	float4 viewRay          : TEXCOORD0;
	float2 uv               : TEXCOORD1;
};

VSOutput main(uint id : SV_VertexID)
{
	VSOutput output;
	output.uv = float2((id << 1) & 2, id & 2);
	output.position = float4(output.uv * float2(2, -2) + float2(-1, 1), 0, 1);

	float3 v1 = lerp(topLeftViewRay.xyz, bottomLeftViewRay.xyz, output.uv.y);
	float3 v2 = lerp(topRightViewRay.xyz, bottomRightViewRay.xyz, output.uv.y);
	output.viewRay = mul(viewMatrix, float4(lerp(v1, v2, output.uv.x), 0.0));

	return output;
}
