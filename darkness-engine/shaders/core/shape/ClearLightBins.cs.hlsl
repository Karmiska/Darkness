
RWBuffer<uint> lightBins;

cbuffer ClearLightBinsConstants
{
	uint binCount;
	uint maxLightsPerBin;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	if (dispatchThreadID.x < binCount)
	{
		lightBins[dispatchThreadID.x] = 0;
	}
}
