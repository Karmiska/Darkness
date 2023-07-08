
#include "../Common.hlsli"
#include "../shared_types/SSRDebug.hlsli"

struct PSInput
{
	float4 position         : SV_Position0;
	float2 uv               : TEXCOORD0;
};

StructuredBuffer<SSRDebug>	ssrDebugBuffer;
Buffer<uint>				ssrDebugBufferCounter;

Texture2D<float> depthPyramid;
Texture2D<float4> frame;

cbuffer SSRDebugConstants
{
	uint ssrDebugCount;
	float2 frameSize;
	uint padding;

    float2 pow2size;
    float2 padding2;
};

float2 tileCount(float level)
{
	return level == 0 ? pow2size : pow2size / exp2(level);
}

float2 tileSize(float level, float2 tileCount)
{
	return pow2size / tileCount;
}

float4 main(PSInput input) : SV_Target
{
	discard;
	return float4(0.0f, 0.0f, 0.0f, 0.0f);

	uint2 gUV = uint2((uint)input.position.x, (uint)input.position.y);
	float4 outputColor = frame.Load(uint3(gUV, 0));

	uint ssrDebugInfoCount = ssrDebugCount + 1 +		ssrDebugBufferCounter[0]- ssrDebugBufferCounter[0];// min(ssrDebugBufferCounter[0] + 1, ssrDebugCount + 1);
	float distance = (1 << 3) / 2;
	bool active = false;
	for (uint i = 0; i < ssrDebugInfoCount; ++i)
	{
		int2 sp = (int2)ssrDebugBuffer[i].screenSpacePosition.xy;
		int2 sp2 = (int2)ssrDebugBuffer[i].screenSpacePositionNextWorldPos.xy;
		int2 guvp = (int2)gUV;
		//if (length(sp - guvp) < (1 << 5) / 2)
		if ((abs(sp.x - guvp.x) < distance) &&
			(abs(sp.y - guvp.y) < distance))
		{
			outputColor = ssrDebugBuffer[i].color;
			active = true;
		}

		if ((abs(sp.x + 30 - guvp.x) < distance*3) &&
			(abs(sp.y - guvp.y) < distance*3))
		{
			float compClr = ssrDebugBuffer[i].worldSpacePosition.x < ssrDebugBuffer[i].worldSpacePosition.y ? 1.0f : 0.0f;
			if(sp.x + 30 - guvp.x > 0.0f)
				outputColor = float4(
					ssrDebugBuffer[i].worldSpacePosition.x / 1.0, 
					compClr, 
					compClr, 1.0f);
			else
				outputColor = float4(
					ssrDebugBuffer[i].worldSpacePosition.y / 1.0,
					compClr,
					compClr, 1.0f);
			active = true;
		}

		// next position
		/*if ((abs(sp2.x - guvp.x) < distance) &&
			(abs(sp2.y - guvp.y) < distance))
		{
			outputColor = ssrDebugBuffer[i].color;
			active = true;
		}*/
	}

	// Draw depth colors based on mip
	uint level = ssrDebugBuffer[ssrDebugInfoCount - 1].rayStep.y;
	float depthSampleDe = depthPyramid.Load(uint3(gUV >> level, level));
	//if(!active)
	//	outputColor += (float4(depthSampleDe * 70, 0.0f, 0.0f, 1.0f)) * 0.06;
	
	// Draw grid lines
	float2 tCount = tileCount(level);
	float2 tSize = tileSize(level, tCount);
	if (level != 0)
	{
		//if (gUV.x % (uint)tSize.x == 0)
		//	outputColor += float4(0.8f, 0.0f, 0.0f, 1.0f) * 0.03;
		//if (gUV.y % (uint)tSize.y == 0)
		//	outputColor += float4(0.8f, 0.0f, 0.0f, 1.0f) * 0.03;
	}

	return outputColor;
}
