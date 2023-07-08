
Texture2D<float> srcMin;
Texture2D<float> srcMax;

RWTexture2D<float> dst0Min;
RWTexture2D<float> dst1Min;
RWTexture2D<float> dst2Min;
RWTexture2D<float> dst3Min;

RWTexture2D<float> dst0Max;
RWTexture2D<float> dst1Max;
RWTexture2D<float> dst2Max;
RWTexture2D<float> dst3Max;

sampler srcSampler;

cbuffer Constants
{
	float2 size;
	float2 invSize;
};

groupshared float dTileMin[64];
groupshared float dTileMax[64];

[numthreads(8, 8, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint parity = dispatchThreadID.x | dispatchThreadID.y;

	// 8x8 block
	uint2 usize = uint2((uint)size.x, (uint)size.y);
	if (dispatchThreadID.x < usize.x && dispatchThreadID.y < usize.y)
	{
		float2 moveToPixelCenter = float2(0.5f, 0.5f) * invSize;
		float2 uv = (invSize * float2((float)dispatchThreadID.x, (float)dispatchThreadID.y)) + moveToPixelCenter;

		float4 srcDataMin = srcMin.GatherRed(srcSampler, uv, 0);
		float4 srcDataMax = srcMax.GatherRed(srcSampler, uv, 0);
		float tiledepthMin = min(min(min(srcDataMin.x, srcDataMin.y), srcDataMin.z), srcDataMin.w);
		float tiledepthMax = max(max(max(srcDataMax.x, srcDataMax.y), srcDataMax.z), srcDataMax.w);
		dst0Min[dispatchThreadID.xy] = tiledepthMin;
		dst0Max[dispatchThreadID.xy] = tiledepthMax;
		dTileMin[groupIndex] = tiledepthMin;
		dTileMax[groupIndex] = tiledepthMax;
	}

	GroupMemoryBarrierWithGroupSync();

	// 4x4 block
	if ((parity & 1) == 0)
	{
		float tiledepthMin = min(min(min(dTileMin[groupIndex], dTileMin[groupIndex + 1]), dTileMin[groupIndex + 8]), dTileMin[groupIndex + 9]);
		float tiledepthMax = max(max(max(dTileMax[groupIndex], dTileMax[groupIndex + 1]), dTileMax[groupIndex + 8]), dTileMax[groupIndex + 9]);
		dTileMin[groupIndex] = tiledepthMin;
		dTileMax[groupIndex] = tiledepthMax;
		dst1Min[dispatchThreadID.xy >> 1] = tiledepthMin;
		dst1Max[dispatchThreadID.xy >> 1] = tiledepthMax;
	}

	GroupMemoryBarrierWithGroupSync();

	// 2x2 block
	if ((parity & 3) == 0)
	{
		float tiledepthMin = min(min(min(dTileMin[groupIndex], dTileMin[groupIndex + 2]), dTileMin[groupIndex + 16]), dTileMin[groupIndex + 18]);
		float tiledepthMax = max(max(max(dTileMax[groupIndex], dTileMax[groupIndex + 2]), dTileMax[groupIndex + 16]), dTileMax[groupIndex + 18]);
		dTileMin[groupIndex] = tiledepthMin;
		dTileMax[groupIndex] = tiledepthMax;
		dst2Min[dispatchThreadID.xy >> 2] = tiledepthMin;
		dst2Max[dispatchThreadID.xy >> 2] = tiledepthMax;
	}

	GroupMemoryBarrierWithGroupSync();

	// 1x1 block
	if ((parity & 7) == 0)
	{
		float tiledepthMin = min(min(min(dTileMin[groupIndex], dTileMin[groupIndex + 4]), dTileMin[groupIndex + 32]), dTileMin[groupIndex + 36]);
		float tiledepthMax = max(max(max(dTileMax[groupIndex], dTileMax[groupIndex + 4]), dTileMax[groupIndex + 32]), dTileMax[groupIndex + 36]);
		dst3Min[dispatchThreadID.xy >> 3] = tiledepthMin;
		dst3Max[dispatchThreadID.xy >> 3] = tiledepthMax;
	}
}
