
struct GlyphRenderNodeData
{
	float2 uvTopLeft;
	float2 uvBottomRight;
	float2 size;
	float2 dstPosition;
};

StructuredBuffer<GlyphRenderNodeData> glyphs;

struct VSOutput
{
	float4 position : SV_Position;
	float4 uv : TEXCOORD0;
};

cbuffer DrawRectangleConstants
{
	float2 screenSize;
	float x;
	float y;

	float width;
	float height;
	uint2 flipUV;

	uint2 startIndex;
	float2 position;
};

VSOutput main(uint id : SV_VertexID)
{
	int glyphId = id % 6;
	int indexId = id / 6;
	GlyphRenderNodeData glyph = glyphs[startIndex.x + indexId];
	glyph.dstPosition *= 0.5;
	glyph.dstPosition += float2(position.x + 2, position.y - 2);
	//glyph.size *= 0.8;
	//glyph.size.x *= 0.9;

	VSOutput output;
	if (glyphId == 0)
	{
		float2 position = float2(0.0f, 0.0f) * glyph.size / screenSize + (glyph.dstPosition / screenSize) * float2(2, -2) + float2(-1, 1);
		output.position = float4(position, 0, 1);
		output.uv = float4(glyph.uvTopLeft, 0, 0);
	}
	else if (glyphId == 1)
	{
		float2 position = float2(1.0f, 0.0f) * glyph.size / screenSize + (glyph.dstPosition / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
		output.uv = float4(glyph.uvBottomRight.x, glyph.uvTopLeft.y, 0, 0);
	}
	else if (glyphId == 2)
	{
		float2 position = float2(0.0f, -1.0f) * glyph.size / screenSize + (glyph.dstPosition / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
		output.uv = float4(glyph.uvTopLeft.x, glyph.uvBottomRight.y, 0, 0);
	}

	else if (glyphId == 3)
	{
		float2 position = float2(0.0f, -1.0f) * glyph.size / screenSize + (glyph.dstPosition / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
		output.uv = float4(glyph.uvTopLeft.x, glyph.uvBottomRight.y, 0, 0);
	}
	else if (glyphId == 4)
	{
		float2 position = float2(1.0f, 0.0f) * glyph.size / screenSize + (glyph.dstPosition / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
		output.uv = float4(glyph.uvBottomRight.x, glyph.uvTopLeft.y, 0, 0);
	}
	else if (glyphId == 5)
	{
		float2 position = float2(1.0f, -1.0f) * glyph.size / screenSize + (glyph.dstPosition / screenSize) * float2(2, -2) + float2(-1, 1);;
		output.position = float4(position, 0, 1);
		output.uv = float4(glyph.uvBottomRight.x, glyph.uvBottomRight.y, 0, 0);
	}
	else
	{
		output.position = float4(0, 0, 0, 0);
		output.uv = float4(0, 0, 0, 0);
	}
	return output;
}
