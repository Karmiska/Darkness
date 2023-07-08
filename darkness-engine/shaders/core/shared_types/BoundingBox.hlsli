#ifdef __cplusplus
#pragma once
#endif

struct BoundingBox
{
	float3 min;
	float padding1;

	float3 max;
	float padding2;
};
