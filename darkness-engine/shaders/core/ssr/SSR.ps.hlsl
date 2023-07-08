
#include "../Common.hlsli"
#include "../GBuffer.hlsli"
#include "../PhysicallyBasedRendering.hlsli"
#include "../renderers/Normalmapping.hlsli"
#include "../shared_types/SSRDebug.hlsli"
#include "../ColorConversions.hlsli"

struct PSInput
{
	float4 position         : SV_Position0;
	float2 uv               : TEXCOORD0;
};

Texture2D<float4> color;
Texture2D<float> depthPyramid;
Texture2D<float2> rougnessMetalness;
Texture2D<float2> gbufferNormals;
Texture2D<float2> gbufferMotion;
Texture2D<uint4> noiseTexture;

RWStructuredBuffer<SSRDebug> ssrDebugBuffer;
RWBuffer<uint> ssrDebugBufferCounter;

cbuffer Constants
{
	float4x4 cameraProjectionMatrix;
	float4x4 cameraViewMatrix;
	float4x4 invView;
	float4x4 invProjection;

	float2 frameSize;
	float2 onePerFrameSize;

	float2 nearFar;
	float2 jitter;

	float3 cameraWorldSpacePosition;
	uint frameNumber;

	float mipLevels;
	float3 cameraForward;

	float4 topLeft;
	float4 topRight;
	float4 bottomLeft;
	float4 bottomRight;

	float3 cameraUp;
	float fov;

	float3 cameraRight;
	float padding2;

	uint2 ssrDebugPoint;
    float2 pow2size;
};

// ----------------------------------------------
float2 cell(float2 ray, float2 cell_count) {
	// shouldn't this be /
	return floor(ray.xy * cell_count);
}

float2 cell_count(float level) {
	return frameSize / (level == 0.0 ? 1.0 : exp2(level));
}

float3 intersect_cell_boundary(float3 pos, float3 dir, float2 cell_id, float2 cell_count, float2 cross_step, float2 cross_offset) {
	float2 cell_size = 1.0 / cell_count;
	float2 planes = cell_id / cell_count + cell_size * cross_step;

	float2 solutions = (planes - pos.xy) / dir.xy;
	float3 intersection_pos = pos + dir * min(solutions.x, solutions.y);

	intersection_pos.xy += (solutions.x < solutions.y) ? float2(cross_offset.x, 0.0) : float2(0.0, cross_offset.y);

	return intersection_pos;
}

bool crossed_cell_boundary(float2 cell_id_one, float2 cell_id_two) {
	return (int)cell_id_one.x != (int)cell_id_two.x || (int)cell_id_one.y != (int)cell_id_two.y;
}

float minimum_depth_plane(float2 ray, float level, float2 cell_count) {
	return depthPyramid.Load(int3(ray.xy * cell_count, level)).r;
}

float3 hi_z_trace(float3 p, float3 v, out int iterations) {
	float HIZ_STOP_LEVEL = 0;
	float HIZ_START_LEVEL = 2;
	float MAX_ITERATIONS = 30;
	float HIZ_MAX_LEVEL = mipLevels - 1;

	float level = HIZ_START_LEVEL;
	float3 v_z = v / v.z;
	float2 hi_z_size = cell_count(level);
	float3 ray = p;

	float2 cross_step = float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
	float2 cross_offset = cross_step * 0.00001;
	cross_step = saturate(cross_step);

	float2 ray_cell = cell(ray.xy, hi_z_size.xy);
	ray = intersect_cell_boundary(ray, v, ray_cell, hi_z_size, cross_step, cross_offset);

	iterations = 0;
	while (level >= HIZ_STOP_LEVEL && iterations < MAX_ITERATIONS) {
		// get the cell number of the current ray
		float2 current_cell_count = cell_count(level);
		float2 old_cell_id = cell(ray.xy, current_cell_count);

		// get the minimum depth plane in which the current ray resides
		float min_z = minimum_depth_plane(ray.xy, level, current_cell_count);

		// intersect only if ray depth is below the minimum depth plane
		float3 tmp_ray = ray;
		if (v.z > 0) {
			float min_minus_ray = min_z - ray.z;
			tmp_ray = min_minus_ray > 0 ? ray + v_z * min_minus_ray : tmp_ray;
			float2 new_cell_id = cell(tmp_ray.xy, current_cell_count);
			if (crossed_cell_boundary(old_cell_id, new_cell_id)) {
                tmp_ray = intersect_cell_boundary(ray, v, old_cell_id, current_cell_count, cross_step, cross_offset);
                level = min(HIZ_MAX_LEVEL, level + 2.0f);
            }
            else {
                if (level == 1 && abs(min_minus_ray) > 0.0001) {
                    tmp_ray = intersect_cell_boundary(ray, v, old_cell_id, current_cell_count, cross_step, cross_offset);
                    level = 2;
                }
            }
        }
        else if (ray.z < min_z) {
            tmp_ray = intersect_cell_boundary(ray, v, old_cell_id, current_cell_count, cross_step, cross_offset);
            level = min(HIZ_MAX_LEVEL, level + 2.0f);
        }

        ray.xyz = tmp_ray.xyz;
        --level;

        ++iterations;
    }
    return ray;
}
// ----------------------------------------------

float linDepth(float depth, float2 nearFar)
{
	return 1.0 / (((nearFar.y - nearFar.x) - nearFar.x) * depth + 1.0);
}

float3 worldPosFromUVDepth(float2 uv, float depth)
{
	float3 inputpos = float3(uv.x * 2.0f - 1.0f, (uv.y * 2.0f - 1.0f) * -1.0f, depth);
	float4 ci = mul(invProjection, float4(inputpos.xyz, 1.0f));
	ci.xyz /= ci.w;
	return mul(invView, float4(ci.xyz, 1.0f)).xyz;
}

float2 worldPosToUV(float3 worldPos)
{
	float4 projected = mul(cameraProjectionMatrix, mul(cameraViewMatrix, float4(worldPos, 1.0f)));
	projected.xy /= projected.w;
	projected.xy = projected.xy * 0.5 + 0.5;
	projected.y = 1.0f - projected.y;
	return projected.xy;
}

uint2 toScreenU(float3 pnt)
{
	float4 tpnt = mul(cameraProjectionMatrix, mul(cameraViewMatrix, float4(pnt, 1.0f)));
	tpnt.xy /= tpnt.w;
	tpnt.xy = (tpnt.xy * 0.5) + 0.5;
	tpnt.y = 1.0f - tpnt.y;
	tpnt.xy *= frameSize;
	return (uint2)tpnt.xy;
}

float2 march(float3 pos, float3 direction, float3 cameraPosition, float roughness, bool debugging, out bool hit)
{
	int maxSteps = 30;
	float stepLength = 0.35f;
	float maxDeviation = 0.15;
	float bck = 0.5f;
	float fwd = 1.05f;

	if (roughness < 0.05)
	{
		maxSteps = 200;
		stepLength = 0.1;

		fwd = 1.0f;
		bck = 0.5f;
	}
	else if (roughness < 0.15)
	{
		maxSteps = 50;
		stepLength = 0.3;

		fwd = 1.01f;
		bck = 0.5f;
	}

	hit = true;
	float dlen = 0.0f;
	float clen = 0.0f;

	uint currentStep = 0;

	float3 currentPos = pos + (direction * stepLength);
	[loop]
	for (int i = 0; i < maxSteps; ++i)
	{
		float2 uv = worldPosToUV(currentPos);
		if (uv.x < 0.0f || uv.y < 0.0f || uv.x > 1.0f || uv.y > 1.0f)
		{
			hit = false;
			break;
		}

		float dsample = depthPyramid.Load(uint3(uv.x * frameSize.x / 4, uv.y * frameSize.y / 4, 2));

		float3 depthWPos = worldPosFromUVDepth(uv, dsample);

		dlen = length(depthWPos - cameraPosition);
		clen = length(currentPos - cameraPosition);

		if (debugging)
		{
			uint newDebugIndex = ssrDebugBufferCounter[0];
			++ssrDebugBufferCounter[0];
			ssrDebugBuffer[1 + newDebugIndex].worldSpacePosition = float4(currentPos, 0.0f);
			ssrDebugBuffer[1 + newDebugIndex].screenSpacePosition = uint4(toScreenU(currentPos), 0, 0);
			ssrDebugBuffer[1 + newDebugIndex].rayStep = uint4(currentStep, 0, 0, 0);
			if (dlen < clen)
				ssrDebugBuffer[1 + newDebugIndex].color = float4(1.0f, 0.0f, 0.0f, 1.0f);
			else
				ssrDebugBuffer[1 + newDebugIndex].color = float4(0.0f, 1.0f, 0.0f, 1.0f);
		}

		float s = sign(dlen - clen);

		if (s < 0)
			stepLength *= bck;
		else
			stepLength *= fwd;

		currentPos += direction * stepLength * s;
		++currentStep;
	}
	if (abs(dlen - clen) > maxDeviation)
		hit = false;

	return worldPosToUV(currentPos);
}

float2 tileCount(float level)
{
	return level == 0 ? pow2size : pow2size / exp2(level);
}

float2 tileSize(float level, float2 tileCount)
{
	return pow2size / tileCount;
}

float2 tilePosition(float2 pos, float2 tileSize)
{
	return pos / tileSize;
}

float2 toScreen(float3 pnt)
{
	float4 tpnt = mul(cameraProjectionMatrix, mul(cameraViewMatrix, float4(pnt, 1.0f)));
	tpnt.xy /= tpnt.w;
	tpnt.xy = (tpnt.xy * 0.5) + 0.5;
	tpnt.y = 1.0f - tpnt.y;
	tpnt.xy *= frameSize;
	return tpnt.xy;
}

float3 fromScreen(float4 pnt)
{
	pnt.xy /= frameSize;
	pnt.y = 1.0f - pnt.y;
	pnt.xy = (pnt.xy - 0.5) * 2.0f;
	pnt.xy *= pnt.w;
	return mul(invView, mul(invProjection, pnt)).xyz;
}

float3 closestPointOnTwoLines(float3 point1, float3 dir1, float3 point2, float3 dir2)
{
	float a = dot(dir1, dir1);
	float b = dot(dir1, dir2);
	float e = dot(dir2, dir2);

	float d = a * e - b * b;

	//lines are not parallel
	if (d != 0.0f)
	{

		float3 r = point1 - point2;
		float c = dot(dir1, r);
		float f = dot(dir2, r);
		float s = (b * f - c * e) / d;
		//float t = (a * f - c * b) / d;

		return point1 + dir1 * s;
		//return point2 + dir2 * t;
	}
	else
	{
		return float3(0.0f, 0.0f, 0.0f);
	}
}

bool linePlaneIntersect(out float2 intersection, float2 planePoint, float2 planeNormal, float2 linePoint, float2 lineDirection)
{
	float2 diff = linePoint - planePoint;
	float prod1 = dot(diff, planeNormal);
	float prod2 = dot(lineDirection, planeNormal);
	float prod3 = prod1 / (prod2 + 0.00001f);
	intersection = linePoint - lineDirection * prod3;
	return true;
}

bool linePlaneIntersectWorld(
	out float3 intersection, 
	float3 planePoint, 
	float3 planeNormal, 
	float3 linePoint, 
	float3 lineDirection)
{
	intersection = float3(0.0f, 0.0f, 0.0f);

	if (dot(planeNormal, normalize(lineDirection)) == 0)
	{
		return false;
	}

	float t = (dot(planeNormal, planePoint) - dot(planeNormal, linePoint)) / dot(planeNormal, normalize(lineDirection));
	intersection = linePoint + (normalize(lineDirection) * t);
	return true;
}

float3 screenToWorldSpaceViewRay(float2 screenPos)
{
	// this does work..
	float2 uv = screenPos / frameSize;
	return normalize(worldPosFromUVDepth(uv, 0) - cameraWorldSpacePosition);
}

float3 worldPosFromScreenPos(float2 screenPos, int level)
{
	float depth = depthPyramid.Load(uint3((uint2)screenPos.xy >> level, level)).x;
	return worldPosFromUVDepth(screenPos.xy / frameSize, depth);
}

float3 worldPosFromScreenPosU(uint2 screenPos, int level)
{
	float depth = depthPyramid.Load(uint3(screenPos >> level, level)).x;
	return worldPosFromUVDepth(screenPos.xy / frameSize, depth);
}

float3 world(float3 pos, float3 direction, float2 screenPos)
{
	float3 viewRay = screenToWorldSpaceViewRay(screenPos);
	return closestPointOnTwoLines(
		pos, normalize(direction) * 100,
		cameraWorldSpacePosition, viewRay);
}

float2 intersectTile(float2 screenPos, float2 direction, int level, out bool canSwitch)
{
	float2 tSize = tileSize(level, tileCount(level));
	float2 tPos = floor(tilePosition(screenPos, tSize));
    uint2 utPos = (uint2)tilePosition(screenPos, tSize);

	float2 xintersect;
	float2 yintersect;
	float2 boundary;
	boundary.x = direction.x >= 0 ? (tPos.x + 1) * tSize.x : tPos.x * tSize.x;
	boundary.y = direction.y >= 0 ? (tPos.y + 1) * tSize.y : tPos.y * tSize.y;

	linePlaneIntersect(
		xintersect,
		float2(boundary.x, 0.0f),
		float2(1.0f, 0.0f),
		screenPos,
		direction
		);

	linePlaneIntersect(
		yintersect,
		float2(0.0f, boundary.y),
		float2(0.0f, 1.0f),
		screenPos,
		direction);

	float distx = length(xintersect - screenPos);
	float disty = length(yintersect - screenPos);
		
	if (distx < disty)
	{
        canSwitch = (utPos.x & 0x1) == 1;
		return xintersect;
	}
	else
	{
        canSwitch = (utPos.y & 0x1) == 1;
		return yintersect;
	}
}

float3 intersectTileWorld(float3 pos, float3 direction, int level, out bool canSwitch)
{
	float2 screenPos = worldPosToUV(pos) * frameSize;

	float2 tSize = tileSize(level, tileCount(level));
	float2 tPos = floor(tilePosition(screenPos, tSize));
	uint2 utPos = (uint2)tilePosition(screenPos, tSize);

	float2 ascreenPos = toScreenU(pos);
	float3 anext = pos + direction;
	float2 ascreenNext = toScreen(anext);
	float2 ascreenDirection = normalize((float2)(ascreenNext - ascreenPos));

	float2 boundary;
	boundary.x = ascreenDirection.x >= 0 ? (tPos.x + 1) * tSize.x : tPos.x * tSize.x;
	boundary.y = ascreenDirection.y >= 0 ? (tPos.y + 1) * tSize.y : tPos.y * tSize.y;

	float3 xintersect;
	float3 yintersect;
	linePlaneIntersectWorld(
		xintersect,
		cameraWorldSpacePosition,
		cross(cameraUp, screenToWorldSpaceViewRay(boundary)),
		pos - (direction * 500.0f),
		direction * 1000.0f);

	linePlaneIntersectWorld(
		yintersect,
		cameraWorldSpacePosition,
		cross(cameraRight, screenToWorldSpaceViewRay(boundary)),
		pos - (direction * 500.0f),
		direction * 1000.0f);

	float distx = length(xintersect - pos);
	float disty = length(yintersect - pos);

	if (distx < disty)
	{
		canSwitch = (utPos.x & 0x1) == 1;
		return xintersect;
	}
	else
	{
		canSwitch = (utPos.y & 0x1) == 1;
		return yintersect;
	}
}

float2 marchPyramid(float3 pos, float3 direction, bool debugging, out bool hit)
{
	hit = true;
	float dlen = 0.0f;
	float clen = 0.0f;
	float clenB = 0.0f;

	//pos += direction;

	float3 normalizedDir = normalize(direction);

	float maxDeviation = 10.35;
	uint maxSteps = 30;
	uint maxMip = 5;//mipLevels - 1

	float2 screenPos = uint2(0, 0);
	float2 screenNext = float2(0, 0);
	float2 screenNextPos = uint2(0, 0);
	int level = 5;
	float3 lastPos = pos;

	uint currentStep = 0;
	while (maxSteps && level > 0)
	{
		// point A on screen
		screenPos = toScreen(pos);

		// point B on screen
		float3 next = pos + direction;
		screenNext = toScreen(next);

		// screen direction
		float2 screenDirection = normalize(screenNext - screenPos);
		float2 screenEpsilon = screenDirection * 1.1;

		// point C on screen
        bool canSwitch;
		bool canSwitch2;
		screenNextPos = intersectTile(screenPos, screenDirection, level, canSwitch) + screenEpsilon;
		uint2 screenNextPosBack = intersectTile(screenNextPos, screenDirection, level + (int)canSwitch, canSwitch2) - screenEpsilon;

		// create a view ray from C
		float3 viewRay = screenToWorldSpaceViewRay(screenNextPos);
        //float3 wp = worldPosFromScreenPos(screenNextPos, level);

		// get intersection with original
		float3 nextWorldPosA = closestPointOnTwoLines(pos, direction * 1000, cameraWorldSpacePosition, viewRay * 1000);
        //float3 nextWorldPos = closestPointOnTwoLines(pos, direction * 1000, cameraWorldSpacePosition, wp - cameraWorldSpacePosition);
		
		float3 viewRayB = screenToWorldSpaceViewRay(screenNextPosBack);
		float3 nextWorldPosB = closestPointOnTwoLines(pos, direction * 1000, cameraWorldSpacePosition, viewRayB * 1000);

		// check if we went outside the screen
		float2 uv = screenNextPos / frameSize;
		if (uv.x < 0.0f || uv.y < 0.0f || uv.x > 1.0f || uv.y > 1.0f)
		{
			hit = false;
			break;
		}

		uint2 tilePos = (uint2)tilePosition(screenNextPos, tileSize(level, tileCount(level)));

		// get depth
		float3 position = worldPosFromScreenPosU(screenNextPos, level);

		dlen = length(cameraWorldSpacePosition - position);		// depth
		clen = length(cameraWorldSpacePosition - nextWorldPosB);	// simulated
		clenB = length(cameraWorldSpacePosition - nextWorldPosB);	// simulated

		if (debugging)
		{
			uint newDebugIndex = ssrDebugBufferCounter[0];
			++ssrDebugBufferCounter[0];
			ssrDebugBuffer[1 + newDebugIndex].worldSpacePosition = float4(dlen, clen, 0.0f, 1.0f);
			ssrDebugBuffer[1 + newDebugIndex].screenSpacePosition = uint4(screenNextPos, 0, 0);
			ssrDebugBuffer[1 + newDebugIndex].screenSpacePositionNextWorldPos = uint4(toScreenU(nextWorldPosB), 0, 0);
			ssrDebugBuffer[1 + newDebugIndex].rayStep = uint4(currentStep, level, 0, 0);
			/*if (dlen < clen)
				ssrDebugBuffer[1 + newDebugIndex].color = float4(1.0f, 0.0f, 0.0f, 1.0f);
			else*/
			{
				// ssrDebugBuffer[1 + newDebugIndex].color = colormap((float)currentStep / maxSteps);
				if (level == 0) // pink
					ssrDebugBuffer[1 + newDebugIndex].color = float4(1.0f, 0.0f, 0.964f, 1.0f);
				else if(level == 1) // blue
					ssrDebugBuffer[1 + newDebugIndex].color = float4(0.0f, 0.0f, 1.0f, 1.0f);
				else if (level == 2) // cyan
					ssrDebugBuffer[1 + newDebugIndex].color = float4(0.0f, 0.86f, 1.0f, 1.0f);
				else if (level == 3) // green
					ssrDebugBuffer[1 + newDebugIndex].color = float4(0.0f, 1.0f, 0.0f, 1.0f);
				else if (level == 4) // yellow
					ssrDebugBuffer[1 + newDebugIndex].color = float4(1.0f, 1.0f, 0.0f, 1.0f);
				else if (level == 5) // orange
					ssrDebugBuffer[1 + newDebugIndex].color = float4(1.0f, 0.684f, 0.0f, 1.0f);
				else if (level == 6) // violet
					ssrDebugBuffer[1 + newDebugIndex].color = float4(0.561f, 0.208f, 1.0f, 1.0f);
				else if (level == 7) // red
					ssrDebugBuffer[1 + newDebugIndex].color = float4(1.0f, 0.0f, 0.0f, 1.0f);
			}
			
			/*ssrDebugBuffer[1 + newDebugIndex].color = float4(
				HSVtoRGB(float3(
					currentStep,
					50.0,
					30.0)),
				//float3(1, 0, 237.0 / 255.0),
				//float3(0, 0, 0),
				1.0f);*/
		}

		

		if (dlen < clen || dlen < clenB)
		{
			level = max(level - 1, 0);
		}
		else
		{
			pos = nextWorldPosB;
            if(canSwitch)
			    level = min(level + 1, maxMip);
		}
		
		++currentStep;
		--maxSteps;
	}

	if (abs(dlen - clen) > maxDeviation)
		hit = false;

	return screenNextPos.xy / frameSize;
}



float3 importanceSampleGGX(float2 Xi, float3 N, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	float3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float4 main(PSInput input) : SV_Target
{
	float2 uv = input.uv;
	uint3 uintUV = uint3(uv.x * frameSize.x, uv.y * frameSize.y, 0);
	uint2 halfUintUV = uintUV.xy / 2;
	/*float depth = depthPyramid.Load(uint3(uintUV.xy >> 0, 0)).x;
	float3 position = worldPosFromUVDepth(uv, depth);*/
	float3 position = worldPosFromScreenPosU(uintUV.xy, 0);
	float3 normal = unpackNormalOctahedron(gbufferNormals.Load(uintUV));
	float3 viewRay = normalize(position - cameraWorldSpacePosition);

	float2 roughnessMetalnessValue = rougnessMetalness.Load(uintUV / 2.0f);
	//float roughness = pow(roughnessMetalnessValue.x, 4.0 / 3.0);
	float roughness = roughnessMetalnessValue.x;
	float metalness = roughnessMetalnessValue.y;

	float2 noiseScale = frameSize / 128;
	float2 noiseUV = uv * noiseScale;
	uint2 noiseSampleUint = noiseTexture.Load(uint3(
		(noiseUV.x * frameSize.x + (jitter.x * 1.8)) % 128,
		(noiseUV.y * frameSize.y + (jitter.y * 1.8)) % 128,
		0)).xy;
	float2 noiseSample = float2(
		(float)noiseSampleUint.x / 65535.0f, 
		(float)noiseSampleUint.y / 65535.0f);

	float3 important = importanceSampleGGX(min(noiseSample, 1.0), viewRay, roughness);
	float3 r = reflect(important, normal);
	//r.y *= -1.0f;

	float3 rs = r;
	rs.xy = normalize(rs.xy);
	rs.z *= rs.x / r.x;

	bool hit = true;
	int iterations;
	
	bool isDebugging = halfUintUV.x == ssrDebugPoint.x && halfUintUV.y == ssrDebugPoint.y;

	if (isDebugging)
	{
		ssrDebugBuffer[0].worldSpacePosition = float4(
			length(cameraWorldSpacePosition - position),
			length(cameraWorldSpacePosition - position),
			0.0f, 0.0f);
		ssrDebugBuffer[0].screenSpacePosition = uint4(toScreenU(position), 0, 0);
		ssrDebugBuffer[0].rayStep = uint4(0, 0, 0, 0);
		ssrDebugBuffer[0].color = float4(1, 1, 1, 1);
	}

    //r = (r * 0.000001f) + reflect(viewRay, normal);

    /*float4 test = mul(cameraProjectionMatrix, float4(position, 1.0f));
    test.xyz /= test.w;
    test.xy = (test.xy * 0.5) + 0.5;
    test.y = 1.0f - test.y;*/
    //test.xy *= frameSize;

	//float2 resColUV = march(position, rs, cameraWorldSpacePosition, roughness, isDebugging, hit);
	float2 resColUV = marchPyramid(position, r, isDebugging, hit);
	//float3 wpos = hi_z_trace(position, r, iterations);
	//float2 resColUV = worldPosToUV(wpos);

	float3 resCol = float3(0.0f, 0.0f, 0.0f);
	if (!hit)
	{
		discard;
	}
	else
	{

		float2 motion = gbufferMotion.Load(uint3(resColUV.x * frameSize.x, resColUV.y * frameSize.y, 0));
		float2 prevUV = resColUV - motion;
		resCol = color.Load(uint3(prevUV.x * frameSize.x, prevUV.y * frameSize.y, 0)).xyz;
	}
	
	//float2 screenPos = float2(uv.x * frameSize.x, uv.y * frameSize.y);
	//float2 test = intersectTile(screenPos, normalize(float2(0.0, 1.0)), 0) - screenPos;

	//return (float4(resCol, 1.0) * 0.000001f) + (float4(resColUV, 0.0f, 1.0f) * 1.000001f);
	//return (float4(resCol, 1.0) * 0.000001f) + float4((float)iterations / 100, 0.0, 0.0, 1.0f);
	
    //return (float4(resCol, 1.0) * 0.00001f) + float4(viewRay, 1.0f);
    //return (float4(resCol, 1.0) * 0.00001f) + float4(r, 1.0f);
    //return (float4(resCol, 1.0) * 0.00001f) + float4(r.z, 0.0f, 0.0f, 1.0f);
    return float4(resCol, 1.0);
}
