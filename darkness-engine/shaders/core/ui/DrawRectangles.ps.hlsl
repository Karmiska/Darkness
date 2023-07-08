
struct PSInput
{
    float4 position : SV_Position0;
    uint color      : BLENDINDICES0;
};

uint main(PSInput input) : SV_Target
{
	return input.color;
}
