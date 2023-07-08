
#include "../Common.hlsli"

struct Transform
{
	float4x4 trans;
};

Buffer<float4> lightParameters;
Buffer<float> spotLightRange;
Buffer<uint> spotLightIds;
StructuredBuffer<Transform> spotLightTransforms;
Buffer<float4> lightPositions;
Buffer<float4> lightDirections;

cbuffer RenderConesConstants
{
	float4x4 jitterViewProjectionMatrix;
	float3 cameraPosition;
	uint sectors;
};

struct VSOutput
{
	float4 position     : SV_Position0;
	uint lightId		: BLENDINDICES0;
};

bool isInsideCone(float3 x, float3 direction, float height, float r, float3 p)
{
	float coneDistance = dot(p - x, direction);
	if (coneDistance < 0 || coneDistance > height)
		return false;

	r *= 1.4;
	/*float3 pp = p - (x + (direction * coneDistance));
	float3 add = (pp / coneDistance) * height;
	return length(add) < r;*/
	//x + direction oneDistance

	float coneRadius = (coneDistance / height) * r;
	float orthogonalDistance = length((p - x) - coneDistance * direction);
	return orthogonalDistance < coneRadius;
}

VSOutput main(uint id : SV_VertexID, uint instanceId : SV_InstanceID)
{
	float range = spotLightRange[instanceId];
	float coneAngle = (max(lightParameters[spotLightIds[instanceId]].y, lightParameters[spotLightIds[instanceId]].x) + 1) * DEG_TO_RAD;

	float angleIncrementRad = TWO_PI / (float)(sectors - 1);
	float currentAngle = angleIncrementRad * id;
	float distance = cos(coneAngle) * range;
	float radius = sin(coneAngle) * range;

	if (!isInsideCone(lightPositions[instanceId].xyz, normalize(lightDirections[instanceId].xyz) *-1, range, radius, cameraPosition))
	{
		radius /= cos(angleIncrementRad / 2.0f);

		float3 vertex = float3(0.0f, 0.0f, 0.0f);
		if (id < sectors)
			vertex = float3(
				radius * cos(currentAngle),
				radius * sin(currentAngle),
				-distance);
		else if (id == sectors + 1)
			vertex = float3(0.0f, 0.0f, -distance);

		VSOutput output;
		output.position = mul(jitterViewProjectionMatrix, mul(spotLightTransforms[instanceId].trans, float4(vertex, 1.0f)));
		output.lightId = spotLightIds[instanceId];
		return output;
	}
	else
	{
		// we're inside the cone. need to act accordingly

		if (id == 0 || id == 30 || id == 1)
		{
			VSOutput output;
			if(id == 1)
				output.position = float4(-1.0f, 1.0f, 1.0f, 1.0f);
			else if(id == 30)
				output.position = float4(-1.0f, -3.0f, 1.0f, 1.0f);
			else if (id == 0)
				output.position = float4(3.0f, 1.0f, 1.0f, 1.0f);
			output.lightId = spotLightIds[instanceId];
			return output;
		}
		else
		{
			const float4 invalid = float4(
				asfloat(0x7f800001),
				asfloat(0x7f800001),
				asfloat(0x7f800001),
				asfloat(0x7f800001));

			VSOutput output;
			output.position = invalid;
			output.lightId = 0;
			return output;
		}
	}
    
}
