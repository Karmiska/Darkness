#ifdef __cplusplus
#pragma once
#endif

struct SSRDebug
{
	float4 worldSpacePosition;
	uint4 screenSpacePosition;
	uint4 screenSpacePositionNextWorldPos;
	uint4 rayStep;
	float4 color;
};
