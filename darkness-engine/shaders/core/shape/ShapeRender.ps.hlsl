
RWBuffer<uint> lightBins;

cbuffer ShapeRenderConstants
{
	uint2 binSize;
	uint pitch;
	uint maxLightsPerBin;

	uint lightId;
	uint3 padding;
};

struct PSInput
{
	float4 position : SV_Position0;
};

[earlydepthstencil]
void main(PSInput input)
{
	uint2 location = uint2((uint)input.position.x, (uint)input.position.y);
	uint2 binLocation = location;

	// pitch = binSize.x * maxLightsPerBin
	uint countLocation = (binLocation.y * pitch) + (binLocation.x * maxLightsPerBin);

	uint newLightCount = 0;
	InterlockedAdd(lightBins[countLocation], 1, newLightCount);

	// the zero element is the counter
	uint thisLightIndex = min(newLightCount + 1, maxLightsPerBin-2);

	lightBins[countLocation + thisLightIndex] = lightId;
}
