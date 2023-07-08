
Texture2D<uint> input;
RWTexture2D<uint> output;

[numthreads(8, 8, 1)]
void main(uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID)
{
	// each of the source indexes has 16 values
	uint2 srcIndex = DTid.xy * 2;
	uint2 dstIndex = DTid.xy;
		
	uint a = input[srcIndex];
	uint b = input[srcIndex + uint2(1, 0)];
	uint c = input[srcIndex + uint2(0, 1)];
	uint d = input[srcIndex + uint2(1, 1)];

	uint outputValue =
	   (((((a >> 0)	& 0x1) ||
		((a >> 1)	& 0x1) ||
		((a >> 4)	& 0x1) ||
		((a >> 5)	& 0x1) ) << 0) |	// bit 0

	   ((((a >> 2)	& 0x1) ||
		((a >> 3)	& 0x1) ||
		((a >> 6)	& 0x1) ||
		((a >> 7)	& 0x1) ) << 1) |	// bit 1

	   ((((b	>> 0)	& 0x1) ||
		((b >> 1)	& 0x1) ||
		((b >> 4)	& 0x1) ||
		((b >> 5)	& 0x1) ) << 2) |	// bit 2

	   ((((b >> 2)	& 0x1) ||
		((b >> 3)	& 0x1) ||
		((b >> 6)	& 0x1) ||
		((b >> 7)	& 0x1) ) << 3) |	// bit 3

	   ((((a >> 8)	& 0x1) ||
		((a >> 9)	& 0x1) ||
		((a >> 12)	& 0x1) ||
		((a >> 13)	& 0x1) ) << 4) |	// bit 4

	   ((((a >> 10)	& 0x1) ||
		((a >> 11)	& 0x1) ||
		((a >> 14)	& 0x1) ||
		((a >> 15)	& 0x1) ) << 5) |	// bit 5

	   ((((b >> 8)	& 0x1) ||
		((b >> 9)	& 0x1) ||
		((b >> 12)	& 0x1) ||
		((b >> 13)	& 0x1) ) << 6) |	// bit 6

	   ((((b >> 10)	& 0x1) ||
		((b >> 11)	& 0x1) ||
		((b >> 14)	& 0x1) ||
		((b >> 15)	& 0x1) ) << 7) |	// bit 7

	   ((((c >> 0)	& 0x1) ||
		((c >> 1)	& 0x1) ||
		((c >> 4)	& 0x1) ||
		((c >> 5)	& 0x1) ) << 8) |	// bit 8

	   ((((c >> 2)	& 0x1) ||
		((c >> 3)	& 0x1) ||
		((c >> 6)	& 0x1) ||
		((c >> 7)	& 0x1) ) << 9) |	// bit 9

	   ((((d >> 0)	& 0x1) ||
		((d >> 1)	& 0x1) ||
		((d >> 4)	& 0x1) ||
		((d >> 5)	& 0x1) ) << 10) |	// bit 10

	   ((((d >> 2)	& 0x1) ||
		((d >> 3)	& 0x1) ||
		((d >> 6)	& 0x1) ||
		((d >> 7)	& 0x1) ) << 11) |	// bit 11

	   ((((c >> 8)	& 0x1) ||
		((c >> 9)	& 0x1) ||
		((c >> 12)	& 0x1) ||
		((c >> 13)	& 0x1) ) << 12) |	// bit 12

	   ((((c >> 10)	& 0x1) ||
		((c >> 11)	& 0x1) ||
		((c >> 14)	& 0x1) ||
		((c >> 15)	& 0x1) ) << 13) |	// bit 13

	   ((((d >> 8)	& 0x1) ||
		((d >> 9)	& 0x1) ||
		((d >> 12)	& 0x1) ||
		((d >> 13)	& 0x1) ) << 14) |	// bit 14

	   ((((d >> 10)	& 0x1) ||
		((d >> 11)	& 0x1) ||
		((d >> 14)	& 0x1) ||
		((d >> 15)	& 0x1) ) << 15)); 	// bit 15
	   

	output[dstIndex] = outputValue;
}
