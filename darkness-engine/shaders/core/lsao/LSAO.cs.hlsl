
struct LineInfo
{
    float2 startPos;
    float2 stepDir;

    int dirIndex;
    int numSteps;

    int idleSteps;
    int layerDistance;

    int layerOffset;
    float tangent;
};

Texture2D<float> depth;
StructuredBuffer<LineInfo> lineInfos;
sampler depthSampler;

float SSEOdot2(float2 a, float2 b)
{
    return a.x * b.x + a.y * b.y;
};

float2 SSEOnormalize2(float2 a)
{
    float coef = rsqrt(a.x * a.x + a.y * a.y);
    return float2(a.x * coef, a.y * coef);
};

float vecFalloff(float2 horVec)
{
    float invCoef = 0.25;
    return (1.0f / (1.0f + invCoef * SSEOdot2(horVec, horVec)));
};

float fallOff(float distance)
{
    float coef = 1.0f;
    return coef / (coef + (distance * distance));
};

bool occlusionCompare(float2 v1, float2 v2, float2 upVec)
{
    return
        (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v1)) - -0.85)) * vecFalloff(v1) >
        (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -0.85)) * vecFalloff(v2);
};

float2 snapCoord(float2 inCoord)
{
    return float2(
        (float)((int)(inCoord.x * 1.536e+3f)) * 6.510417e-4f + 3.255208e-4f,
        (float)((int)(inCoord.y * 8.64e+2f)) * 1.157407e3f + 5.787037e-4f
        );
};

groupshared float2 convexHull[16][32];
groupshared float2 pLocalS[128];

float sweep(LineInfo li, uint3 groupThreadID)
{
    if (li.numSteps < 2)
        return 0.0f;

    int destIndex = li.layerDistance;
    float res = 0; // this is supposed to be a parameter that comes and goes out?

    res += (li.dirIndex < 8) ? 2 : 3;
    int myStripe = (li.dirIndex < 8) ? 32 : -32;
    float2 dirStep = SSEOnormalize2(float2(li.stepDir.x * 1.536e3f, li.stepDir.y * 8.64e2f));
    int convexIndex = 1;

    float2 h1, h2, h3;
    float2 pLocal = float2(0.0f, 0.0f);
    float2 upVec = float2(0.0f, 0.0f);

    h3 = float2(0.0, -10000.0f);

    // sample
    {
        float2 tempSnapCoord = snapCoord(li.startPos);
        float height = depth.SampleLevel(depthSampler, tempSnapCoord, 0);
        float2 projXY = float2(
            1.0f + tempSnapCoord.x * -2.0f, 
            5.625e-1f + tempSnapCoord.y * -1.125f);
        h2 = float2((projXY.x * dirStep.x + projXY.y * dirStep.y) * height, height);
    }

    // step forward
    li.numSteps--;
    li.idleSteps--;
    li.startPos += li.stepDir;

    // sample
    {
        float2 tempSnapCoord = snapCoord(li.startPos);
        float height = depth.SampleLevel(depthSampler, tempSnapCoord, 0);
        float2 projXY = float2(
            1.0f + tempSnapCoord.x * -2.0f,
            5.625e-1f + tempSnapCoord.y * -1.125f);
        h1 = float2((projXY.x * dirStep.x + projXY.y * dirStep.y) * height, height);
    }

    // step forward
    li.numSteps--;
    li.idleSteps--;
    li.startPos += li.stepDir;

    while (li.idleSteps > 4)
    {
        for (int slota = 0; slota < 4; ++slota)
        {
            // sample
            {
                float2 tempSnapCoord = snapCoord(li.startPos);
                float height = depth.SampleLevel(depthSampler, tempSnapCoord, 0);
                float2 projXY = float2(
                    1.0f + tempSnapCoord.x * -2.0f,
                    5.625e-1f + tempSnapCoord.y * -1.125f);
                pLocalS[slota*32 + groupThreadID.x] = float2((projXY.x * dirStep.x + projXY.y * dirStep.y) * height, height);
            }

            li.startPos += li.stepDir;
        }

        for (int slotb = 0; slotb < 4; ++slotb)
        {
            pLocal = pLocalS[slotb * 32 + groupThreadID.x];
            upVec = SSEOnormalize2(-pLocal);
            float2 v1 = h1 - pLocal;
            float2 v2 = h2 - pLocal;
            float dot1 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v1)) - -8.5e-1f));
            float dot2 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -8.5e-1f));
            float occ1 = dot1 * vecFalloff(v1);
            float occ2 = dot2 * vecFalloff(v2);
            int fullIters = 15;

            if (convexIndex && 
                (occ1 <= occ2 + 1.0e-3f) && 
                (dot1 <= dot2 + 1.0e-3f))
            {
                dot1 = dot2;
                occ1 = occ2;
                h1 = h2;
                h2 = h3;
                convexIndex--;
                fullIters--;
                v2 = h2 - pLocal;
                dot2 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -8.5e-1f));
                occ2 = dot2 * vecFalloff(v2);

                [unroll]
                while (
                    fullIters && 
                    convexIndex && 
                    occ1 <= occ2 + 1.0e-3f && 
                    dot1 <= dot2 + 1.0e-3f
                    )
                {
                    dot1 = dot2;
                    occ1 = occ2;
                    h1 = h2;
                    convexIndex--;
                    h2 = convexHull[convexIndex & 15][groupThreadID.x];
                    fullIters--;
                    v2 = h2 - pLocal;
                    dot2 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -8.5e-1f));
                    occ2 = dot2 * vecFalloff(v2);
                }
            }

            h3 = h2;
            if (fullIters == 15)
            {
                convexHull[convexIndex & 15][groupThreadID.x] = h2;
            }
            convexIndex++;
            h2 = h1;
            h1 = pLocal;
            li.numSteps--;
            li.idleSteps--;
        }
    }

    while (li.idleSteps > 0)
    {
        {
            float2 tempSnapCoord = snapCoord(li.startPos);
            float height = depth.SampleLevel(depthSampler, tempSnapCoord, 0);
            float2 projXY = float2(
                1.0f + tempSnapCoord.x * -2.0f,
                5.625e-1f + tempSnapCoord.y * -1.125f);
            pLocal = float2((projXY.x * dirStep.x + projXY.y * dirStep.y) * height, height);
            upVec = SSEOnormalize2(-pLocal);
        }

        float2 v1 = h1 - pLocal;
        float2 v2 = h2 - pLocal;
        float dot1 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v1)) - -8.5e-1f));
        float dot2 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -8.5e-1f));
        float occ1 = dot1 * vecFalloff(v1);
        float occ2 = dot2 * vecFalloff(v2);
        int fullIters = 15;

        if (convexIndex &&
            (occ1 <= occ2 + 1.0e-3f) &&
            (dot1 <= dot2 + 1.0e-3f))
        {
            dot1 = dot2;
            occ1 = occ2;
            h1 = h2;
            h2 = h3;
            convexIndex--;
            fullIters--;
            v2 = h2 - pLocal;
            dot2 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -8.5e-1f));
            occ2 = dot2 * vecFalloff(v2);

            while (
                fullIters &&
                convexIndex &&
                occ1 <= occ2 + 1.0e-3f &&
                dot1 <= dot2 + 1.0e-3f
                )
            {
                dot1 = dot2;
                occ1 = occ2;
                h1 = h2;
                convexIndex--;
                h2 = convexHull[convexIndex & 15][groupThreadID.x];
                fullIters--;
                v2 = h2 - pLocal;
                dot2 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -8.5e-1f));
                occ2 = dot2 * vecFalloff(v2);
            }
        }

        h3 = h2;
        if (fullIters == 15)
        {
            convexHull[convexIndex & 15][groupThreadID.x] = h2;
        }
        convexIndex++;
        h2 = h1;
        h1 = pLocal;

        li.numSteps--;
        li.idleSteps--;
        li.startPos += li.stepDir;
    }

    float occlusion = 0.0f;
    while (li.numSteps > 0)
    {
        for (int slotc = 0; slotc < 4; ++slotc)
        {
            {
                float2 tempSnapCoord = snapCoord(li.startPos);
                float height = depth.SampleLevel(depthSampler, tempSnapCoord, 0);
                float2 projXY = float2(
                    1.0f + tempSnapCoord.x * -2.0f,
                    5.625e-1f + tempSnapCoord.y * -1.125f);
                pLocalS[slotc * 32 + groupThreadID.x] = float2((projXY.x * dirStep.x + projXY.y * dirStep.y) * height, height);
                upVec = SSEOnormalize2(-pLocal);
            }
            li.startPos += li.stepDir;
        }

        for (int slotd = 0; slotd < 4; ++slotd)
        {
            pLocal = pLocalS[slotd * 32 + groupThreadID.x];
            upVec = SSEOnormalize2(-pLocal);
            float2 v1 = h1 - pLocal;
            float2 v2 = h2 - pLocal;
            float dot1 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v1)) - -8.5e-1f));
            float dot2 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -8.5e-1f));
            float occ1 = dot1 * vecFalloff(v1);
            float occ2 = dot2 * vecFalloff(v2);
            int fullIters = 15;

            if (convexIndex &&
                (occ1 <= occ2 + 1.0e-3f) &&
                (dot1 <= dot2 + 1.0e-3f))
            {
                dot1 = dot2;
                occ1 = occ2;
                h1 = h2;
                h2 = h3;
                convexIndex--;
                fullIters--;
                v2 = h2 - pLocal;
                dot2 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -8.5e-1f));
                occ2 = dot2 * vecFalloff(v2);

                while (
                    fullIters &&
                    convexIndex &&
                    occ1 <= occ2 + 1.0e-3f &&
                    dot1 <= dot2 + 1.0e-3f
                    )
                {
                    dot1 = dot2;
                    occ1 = occ2;
                    h1 = h2;
                    convexIndex--;
                    h2 = convexHull[convexIndex & 15][groupThreadID.x];
                    fullIters--;
                    v2 = h2 - pLocal;
                    dot2 = (max(0.0f, SSEOdot2(upVec, SSEOnormalize2(v2)) - -8.5e-1f));
                    occ2 = dot2 * vecFalloff(v2);
                }
            }

            h3 = h2;
            if (fullIters == 15)
            {
                convexHull[convexIndex & 15][groupThreadID.x] = h2;
            }
            convexIndex++;
            h2 = h1;
            h1 = pLocal;
            occlusion = -8.5e-1f + occ1;
            if (li.dirIndex < 8)
            {
                // TODO HERE PROB
                float outDestIndex = pLocal.y;
            }
            //outOcc[destIndex * 4] = occlusion;
            li.numSteps--;
            destIndex += myStripe;
        }
    }

    return 0.0f;
};


[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint3 groupThreadID : SV_GroupThreadID)
{
    float sweepRes = sweep(lineInfos[dispatchThreadID.x], groupThreadID);
}
