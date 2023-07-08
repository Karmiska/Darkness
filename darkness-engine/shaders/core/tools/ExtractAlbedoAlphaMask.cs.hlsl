
Texture2D<float4> input;
RWTexture2D<uint> output;

cbuffer ExtractAlphaMaskConstants
{
	uint2 size;
	float threshold;
	float padding;
};

groupshared uint lds[16];

[numthreads(8, 8, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	uint2 dstIndex = dispatchThreadId.xy / 4;
	uint2 dstBitPos = dispatchThreadId.xy - (dstIndex * 4);
	uint bitPos = (dstBitPos.y * 4) + dstBitPos.x;

	if ((dispatchThreadId.x < size.x) && (dispatchThreadId.y < size.y))
		lds[bitPos] = input[dispatchThreadId.xy].w >= threshold ? 1 : 0;
	else
		lds[bitPos] = 0;

	GroupMemoryBarrierWithGroupSync();

	if (bitPos == 0)
	{
		uint value = 0;
		for (int i = 0; i < 16; ++i)
		{
			value |= lds[i] << i;
		}
		output[dstIndex] = value;
	}
}
