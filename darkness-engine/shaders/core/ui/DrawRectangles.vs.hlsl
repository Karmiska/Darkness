
struct RectangleItem
{
	float2 position;
	float2 size;
	uint color;
	uint3 padding;
};

StructuredBuffer<RectangleItem> rectangles;

cbuffer DrawRectangleConstants
{
	float2 screenSize;
	uint startIndex;
	float padding;
};

struct VSOutput
{
	float4 position : SV_Position0;
	uint color		: BLENDINDICES0;
};

VSOutput main(uint id : SV_VertexID)
{
	int rectangleId = id / 6;
	int indexId = id % 6;
	RectangleItem rec = rectangles[startIndex + rectangleId];

	rec.size *= 2;

	VSOutput output;
	output.color = rec.color;
	if (indexId == 0)
	{
		float2 position = float2(0.0f, 0.0f) * rec.size / screenSize + (rec.position / screenSize) * float2(2, -2) + float2(-1, 1);
		output.position = float4(position, 0, 1);
	}
	else if (indexId == 1)
	{
		float2 position = float2(1.0f, 0.0f) * rec.size / screenSize + (rec.position / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
	}
	else if (indexId == 2)
	{
		float2 position = float2(0.0f, -1.0f) * rec.size / screenSize + (rec.position / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
	}

	else if (indexId == 3)
	{
		float2 position = float2(0.0f, -1.0f) * rec.size / screenSize + (rec.position / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
	}
	else if (indexId == 4)
	{
		float2 position = float2(1.0f, 0.0f) * rec.size / screenSize + (rec.position / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
	}
	else if (indexId == 5)
	{
		float2 position = float2(1.0f, -1.0f) * rec.size / screenSize + (rec.position / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
	}
	else
	{
		output.position = float4(0, 0, 0, 0);
	}

	return output;
}
