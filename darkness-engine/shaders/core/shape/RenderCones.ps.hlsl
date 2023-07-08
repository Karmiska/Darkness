
#include "../Common.hlsli"

RWBuffer<uint> lightBins;

cbuffer RenderConesPSConstants
{
	uint2 binSize;
	uint pitch;
	uint maxLightsPerBin;
};

struct PSInput
{
	float4 position : SV_Position0;
	uint lightId	: BLENDINDICES0;
};

[earlydepthstencil]
uint main(PSInput input) : SV_Target
{
	uint2 location = uint2((uint)input.position.x, (uint)input.position.y);
	uint countLocation = (location.y * binSize.x) + location.x;

	const uint bucket_index = input.lightId / 32;
	const uint bucket_place = input.lightId % 32;
	uint lightLocation = (countLocation + (bucket_index * (binSize.x * binSize.y)));
	InterlockedOr(lightBins[lightLocation], (uint)1 << bucket_place);

	return 0;
	/*uint newLightCount = 0;
	InterlockedAdd(lightBins[countLocation], 1, newLightCount);

	// the zero element is the counter
	uint thisLightIndex = min(newLightCount + 1, maxLightsPerBin - 2);

	uint lightLocation = (countLocation + (thisLightIndex * (binSize.x * binSize.y)));
	lightBins[lightLocation] = input.lightId;*/
}
