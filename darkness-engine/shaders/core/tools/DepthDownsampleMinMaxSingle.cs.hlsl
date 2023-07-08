
Texture2D<float> srcMin;
Texture2D<float> srcMax;
RWTexture2D<float> dstMin;
RWTexture2D<float> dstMax;

sampler srcSampler;

cbuffer Constants
{
	float2 size;
	float2 invSize;
};

[numthreads(8, 8, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchThreadID : SV_DispatchThreadID)
{
	uint2 usize = uint2((uint)size.x, (uint)size.y);
	if (dispatchThreadID.x < usize.x && dispatchThreadID.y < usize.y)
	{
		float2 moveToPixelCenter = float2(0.5f, 0.5f) * invSize;
		float2 uv = (invSize * float2((float)dispatchThreadID.x, (float)dispatchThreadID.y)) + moveToPixelCenter;

		float4 srcDataMin = srcMin.GatherRed(srcSampler, uv, 0);
		float4 srcDataMax = srcMax.GatherRed(srcSampler, uv, 0);
		dstMin[dispatchThreadID.xy] = min(min(min(srcDataMin.x, srcDataMin.y), srcDataMin.z), srcDataMin.w);
		dstMax[dispatchThreadID.xy] = max(max(max(srcDataMax.x, srcDataMax.y), srcDataMax.z), srcDataMax.w);
	}
}

