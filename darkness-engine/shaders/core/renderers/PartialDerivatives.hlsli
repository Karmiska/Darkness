
float4 calculateDdxDdyFromUV(float2 gbufferUVSample, uint2 uintUV, float2 materialScale, uint instanceId)
{
	uint idUp = gbufferInstanceId[uintUV + uint2(0, 1)];
	uint idDown = gbufferInstanceId[uintUV + uint2(0, -1)];
	uint idLeft = gbufferInstanceId[uintUV + uint2(-1, 0)];
	uint idRight = gbufferInstanceId[uintUV + uint2(1, 0)];

	float2 dxUp = ((float2)gbufferUV[uintUV + uint2(0, 1)] / 65535.0f) * materialScale;
	float2 dxDown = ((float2)gbufferUV[uintUV + uint2(0, -1)] / 65535.0f) * materialScale;
	float2 dxLeft = ((float2)gbufferUV[uintUV + uint2(-1, 0)] / 65535.0f) * materialScale;
	float2 dxRight = ((float2)gbufferUV[uintUV + uint2(1, 0)] / 65535.0f) * materialScale;

    float2 scalegBufferUVSample = gbufferUVSample * materialScale;

	const float2 gradientThreshold = 0.03f;
	bool up = any(abs(scalegBufferUVSample - dxUp) <= gradientThreshold) && (idUp == instanceId);
	bool down = any(abs(scalegBufferUVSample - dxDown) <= gradientThreshold) && (idDown == instanceId);
	bool left = any(abs(scalegBufferUVSample - dxLeft) <= gradientThreshold) && (idLeft == instanceId);
	bool right = any(abs(scalegBufferUVSample - dxRight) <= gradientThreshold) && (idRight == instanceId);

	float2 uvDX = 0.0f;
	float2 uvDY = 0.0f;

	if (up)
		uvDY = scalegBufferUVSample - dxUp;
	else
		uvDY = dxDown - scalegBufferUVSample;

	if (left)
		uvDX = scalegBufferUVSample - dxLeft;
	else
		uvDX = dxRight - scalegBufferUVSample;

	if (uvDX.x > 1.0f)
		uvDX.x -= 2.0f;
	else if (uvDX.x < -1.0f)
		uvDX.x += 2.0f;
	if (uvDX.y > 1.0f)
		uvDX.y -= 2.0f;
	else if (uvDX.y < -1.0f)
		uvDX.y += 2.0f;

	if (uvDY.x > 1.0f)
		uvDY.x -= 2.0f;
	else if (uvDY.x < -1.0f)
		uvDY.x += 2.0f;
	if (uvDY.y > 1.0f)
		uvDY.y -= 2.0f;
	else if (uvDY.y < -1.0f)
		uvDY.y += 2.0f;

	return float4(uvDX, uvDY);
}

float mipFromDdxDdy(float4 ddxDdy)
{
	float deltaMaxSqr = max(dot(ddxDdy.xy, ddxDdy.xy), dot(ddxDdy.zw, ddxDdy.zw));
	float mml = 0.5 * log2(deltaMaxSqr);
	return max(0, mml);
}
