
Buffer<uint> Histogram;
RWStructuredBuffer<float> Exposure;

cbuffer AdaptiveExposureConstants
{
    float TargetLuminance;
    float AdaptationRate;
    float MinExposure;
    float MaxExposure;
    uint PixelCount;
}

groupshared float gs_Accum[256];

[numthreads(256, 1, 1)]
void main(uint GI : SV_GroupIndex)
{
    float WeightedSum = (float)GI * (float)Histogram.Load(GI);

    [unroll]
    for (uint i = 1; i < 256; i *= 2)
    {
        gs_Accum[GI] = WeightedSum;                    // Write
        GroupMemoryBarrierWithGroupSync();            // Sync
        WeightedSum += gs_Accum[(GI + i) % 256];    // Read
        GroupMemoryBarrierWithGroupSync();            // Sync
    }

    float MinLog = Exposure[4];
    float MaxLog = Exposure[5];
    float LogRange = Exposure[6];
    float RcpLogRange = Exposure[7];

    // Average histogram value is the weighted sum of all pixels divided by the total number of pixels
    // minus those pixels which provided no weight (i.e. black pixels.)
    float weightedHistAvg = WeightedSum / (max(1, PixelCount - (Histogram.Load(0) & 0x000000ff))) - 1.0;
    float logAvgLuminance = exp2(weightedHistAvg / 254.0 * LogRange + MinLog);
    float targetExposure = TargetLuminance / logAvgLuminance;
    //float targetExposure = -log2(1 - TargetLuminance) / logAvgLuminance;

    float exposure = Exposure[0];
    exposure = lerp(exposure, targetExposure, AdaptationRate);
    exposure = clamp(exposure, MinExposure, MaxExposure);

    if (GI == 0)
    {
        Exposure[0] = exposure;
        Exposure[1] = 1.0 / exposure;
        Exposure[2] = exposure;
        Exposure[3] = weightedHistAvg;

        // First attempt to recenter our histogram around the log-average.
        float biasToCenter = (floor(weightedHistAvg) - 128.0) / 255.0;
        if (abs(biasToCenter) > 0.1)
        {
            MinLog += biasToCenter * RcpLogRange;
            MaxLog += biasToCenter * RcpLogRange;
        }

        // TODO:  Increase or decrease the log range to better fit the range of values.
        // (Idea) Look at intermediate log-weighted sums for under- or over-represented
        // extreme bounds.  I.e. break the for loop into two pieces to compute the sum of
        // groups of 16, check the groups on each end, then finish the recursive summation.

        Exposure[4] = MinLog;
        Exposure[5] = MaxLog;
        Exposure[6] = LogRange;
        Exposure[7] = 1.0 / LogRange;
    }
}
