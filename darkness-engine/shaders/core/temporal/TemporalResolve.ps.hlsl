
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

Texture2D<float4> currentFrame;
Texture2D<float4> history;
Texture2D<float> depth;
Texture2D<float2> gbufferMotion;
sampler pointSampler;
sampler bilinearSampler;

float luminance(float3 color)
{
    return dot(color, float3(0.299f, 0.587f, 0.114f));
};

float max3(float x, float y, float z)
{
    return max(x, max(y, z));
}

cbuffer Constants
{
    float4x4 inverseJitterMatrix;
    float2 textureSize;
    float2 texelSize;
    float2 nearFar;
    float2 jitter;
    float2 previousJitter;
};

#include "../GBuffer.hlsli"

float2 getClosestUV(float2 uv)
{
	float closest = 0.0f;
	float2 closestUV = float2(0.0f, 0.0f);

	float distance00 = depth.Load(int3((int)((uv.x - texelSize.x) * textureSize.x), (int)((uv.y + texelSize.y) * textureSize.y), 0));
	float distance02 = depth.Load(int3((int)((uv.x + texelSize.x) * textureSize.x), (int)((uv.y + texelSize.y) * textureSize.y), 0));
	float distance11 = depth.Load(int3((int) (uv.x				  * textureSize.x), (int)( uv.y				   * textureSize.y), 0));
	float distance20 = depth.Load(int3((int)((uv.x - texelSize.x) * textureSize.x), (int)((uv.y - texelSize.y) * textureSize.y), 0));
	float distance22 = depth.Load(int3((int)((uv.x + texelSize.x) * textureSize.x), (int)((uv.y - texelSize.y) * textureSize.y), 0));

	if (distance00 > closest) { closest = distance00; closestUV = uv + float2(-texelSize.x, texelSize.y); }
	if (distance02 > closest) { closest = distance02; closestUV = uv + float2( texelSize.x, texelSize.y); }
	if (distance11 > closest) { closest = distance11; closestUV = uv; }
	if (distance20 > closest) { closest = distance20; closestUV = uv + float2(-texelSize.x, -texelSize.y); }
	if (distance22 > closest) { closest = distance22; closestUV = uv + float2( texelSize.x, -texelSize.y); }

	return closestUV;

	/*float closest = 0.0f;
	float2 closestUV = float2(0.0f, 0.0f);

	float distance00 = depth.Load(int3((int)((uv.x - texelSize.x) * textureSize.x), (int)((uv.y + texelSize.y) * textureSize.y), 0));
	float distance01 = depth.Load(int3((int)(uv.x * textureSize.x),					(int)((uv.y + texelSize.y) * textureSize.y), 0));
	float distance02 = depth.Load(int3((int)((uv.x + texelSize.x) * textureSize.x), (int)((uv.y + texelSize.y) * textureSize.y), 0));

	float distance10 = depth.Load(int3((int)((uv.x - texelSize.x) * textureSize.x), (int)(uv.y * textureSize.y), 0));
	float distance11 = depth.Load(int3((int)(uv.x * textureSize.x),					(int)(uv.y * textureSize.y), 0));
	float distance12 = depth.Load(int3((int)((uv.x + texelSize.x) * textureSize.x), (int)(uv.y * textureSize.y), 0));

	float distance20 = depth.Load(int3((int)((uv.x - texelSize.x) * textureSize.x), (int)((uv.y - texelSize.y) * textureSize.y), 0));
	float distance21 = depth.Load(int3((int)(uv.x * textureSize.x),					(int)((uv.y - texelSize.y) * textureSize.y), 0));
	float distance22 = depth.Load(int3((int)((uv.x + texelSize.x) * textureSize.x), (int)((uv.y - texelSize.y) * textureSize.y), 0));

	if (distance00 > closest) { closest = distance00; closestUV = uv + float2(-texelSize.x, texelSize.y); }
	if (distance01 > closest) { closest = distance01; closestUV = uv + float2(0,			texelSize.y); }
	if (distance02 > closest) { closest = distance02; closestUV = uv + float2(texelSize.x,	texelSize.y); }

	if (distance10 > closest) { closest = distance10; closestUV = uv + float2(-texelSize.x, 0); }
	if (distance11 > closest) { closest = distance11; closestUV = uv; }
	if (distance12 > closest) { closest = distance12; closestUV = uv + float2(texelSize.x, 0); }

	if (distance20 > closest) { closest = distance20; closestUV = uv + float2(-texelSize.x, -texelSize.y); }
	if (distance21 > closest) { closest = distance21; closestUV = uv + float2(0,			-texelSize.y); }
	if (distance22 > closest) { closest = distance22; closestUV = uv + float2(texelSize.x,	-texelSize.y); }

	return closestUV;*/
}

float3 clampToBox(float3 historySample, float2 uv, out float3 maxBox, out float3 currentSharpened)
{
    // clamp to box
	int3 du = int3(1, 0, 0);
	int3 dv = int3(0, 1, 0);
	int3 uvu = int3((int)(uv.x * textureSize.x), (int)(uv.y * textureSize.y), 0);

	/*float3 near0 = RGB_YCoCg(currentFrame.Load(uvu - du + dv).xyz);
	float3 near2 = RGB_YCoCg(currentFrame.Load(uvu + du + dv).xyz);
    float3 near4 = RGB_YCoCg(currentFrame.Load(uvu).xyz);
	float3 near6 = RGB_YCoCg(currentFrame.Load(uvu - du - dv).xyz);
	float3 near8 = RGB_YCoCg(currentFrame.Load(uvu + du - dv).xyz);
	float3 boxMin = min(near0, min(near2, min(near4, min(near6, near8))));
	float3 boxMax = max(near0, max(near2, max(near4, max(near6, near8))));*/

	// sharpen
	/*float sharpenAmount = 0.375f;
	float neighbourDiff = luminance(abs(near0 - near4));
	neighbourDiff += luminance(abs(near2 - near4));
	neighbourDiff += luminance(abs(near6 - near4));
	neighbourDiff += luminance(abs(near8 - near4));
	float sharpening = (1 - saturate(2 * neighbourDiff)) * sharpenAmount;
	currentSharpened = float3(
		near0 * -sharpening +
		near2 * -sharpening +
		near6 * -sharpening +
		near8 * -sharpening +
		near4 * 5
		) * 1.0 / (5.0 + sharpening * -4.0);
	*/

	float3 near0 = RGB_YCoCg(currentFrame.Load(uvu - du + dv).xyz);
	float3 near1 = RGB_YCoCg(currentFrame.Load(uvu + dv).xyz);
	float3 near2 = RGB_YCoCg(currentFrame.Load(uvu + du + dv).xyz);
	float3 near3 = RGB_YCoCg(currentFrame.Load(uvu - du).xyz);
	float3 near4 = RGB_YCoCg(currentFrame.Load(uvu).xyz);
	float3 near5 = RGB_YCoCg(currentFrame.Load(uvu + du).xyz);
	float3 near6 = RGB_YCoCg(currentFrame.Load(uvu - du - dv).xyz);
	float3 near7 = RGB_YCoCg(currentFrame.Load(uvu - dv).xyz);
	float3 near8 = RGB_YCoCg(currentFrame.Load(uvu + du - dv).xyz);
	float3 boxMin = min(near0, min(near1, min(near2, min(near3, min(near4, min(near5, min(near6, min(near7, near8))))))));
	float3 boxMax = max(near0, max(near1, max(near2, max(near3, max(near4, max(near5, max(near6, max(near7, near8))))))));
    
	// sharpen
	float sharpenAmount = 0.375f;
	float neighbourDiff = luminance(abs(near0 - near4));
	neighbourDiff += luminance(abs(near1 - near4));
	neighbourDiff += luminance(abs(near2 - near4));
	neighbourDiff += luminance(abs(near3 - near4));
	neighbourDiff += luminance(abs(near5 - near4));
	neighbourDiff += luminance(abs(near6 - near4));
	neighbourDiff += luminance(abs(near7 - near4));
	neighbourDiff += luminance(abs(near8 - near4));
	float sharpening = (1 - saturate(2 * neighbourDiff)) * sharpenAmount;
	currentSharpened = float3(
		near0 * -sharpening +
		near1 * -sharpening +
		near2 * -sharpening +
		near3 * -sharpening +
		near5 * -sharpening +
		near6 * -sharpening +
		near7 * -sharpening +
		near8 * -sharpening +
		near4 * 9
		) * 1.0 / (9.0 + sharpening * -8.0);
	//currentSharpened = near4;

    maxBox = boxMax;

    return clamp(historySample, boxMin, boxMax);
}

float4 main(PSInput input) : SV_Target
{
    float2 uv = input.uv;
    float2 closestUV = getClosestUV(uv);
    float2 motion = gbufferMotion.Load(int3(closestUV * textureSize, 0));
    float2 previousUV = uv - motion;

    float3 historySample = RGB_YCoCg(SampleTextureCatmullRom4Samples(history, bilinearSampler, previousUV, textureSize).xyz);
    
    float3 maxBox;
	float3 currentSample;
    historySample = clampToBox(historySample, uv, maxBox, currentSample);
    
    float blendFactor = 1.0 / 2.0;
    float currLum = luminance(currentSample);
    float historyLum = luminance(historySample);
    float diff = 1.0 - (abs(currLum - historyLum) / (FLT_EPS + max(currLum, max(historyLum, luminance(maxBox)))));
    blendFactor *= diff;
	
	float nomovemul = 0.1f;
	if(length(motion.xy) < 0.00001)
		blendFactor *= nomovemul;
    
	float3 result = lerp(historySample, currentSample, max(min(blendFactor, 1.0f), 0.0f));
	result = YCoCg_RGB(result);
#ifdef OPTION_VISUALIZE_MOTION
    return (float4(result.xyz, 1.0) * 0.0001f) + float4(abs(motion.xy) * 20.0f, 0.0f, 1.0);
#else
    return float4(result.xyz, 1.0);
#endif
}
