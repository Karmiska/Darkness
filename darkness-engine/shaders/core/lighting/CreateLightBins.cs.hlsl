Buffer<float4> lightWorldPosition;
Buffer<float4> lightDirection;
Buffer<float4> lightParameters;
Buffer<uint> lightType;
Buffer<float> lightRange;

RWBuffer<uint> bins;

cbuffer LightBinningConstants
{
	uint lightCount;
	uint width;
	uint height;
};

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
	for (uint i = 0; i < lightCount; ++i)
	{
		if (lightType[i] == 0)
		{
			// directional
		}
		else if (lightType[i] == 1)
		{
			// spot light
		}
		else if (lightType[i] == 2)
		{
			// point light
		}
	}
}
