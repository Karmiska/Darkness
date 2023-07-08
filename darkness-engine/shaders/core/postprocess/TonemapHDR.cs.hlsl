
StructuredBuffer<float> Exposure;
Texture2D<float3> Bloom;
Texture2D<float3> ColorInput;
RWTexture2D<float3> ColorRW;
RWTexture2D<float> OutLuma;
sampler LinearSampler;

cbuffer TonemapHDRConstants
{
    float2 g_RcpBufferDim;
    float2 g_BufferDim;
    float g_BloomStrength;
    float g_chromaticAberrationStrength;

    float g_vignetteInnerRadius;
    float g_vignetteOuterRadius;
    float g_vignetteOpacity;
    float padding;
};

float3 TM_Reinhard(float3 hdr, float k = 1.0)
{
    return hdr / (hdr + k);
}

// The inverse of Reinhard
float3 ITM_Reinhard(float3 sdr, float k = 1.0)
{
    return k * sdr / (k - sdr);
}

float3 TM_Stanard(float3 hdr)
{
    return TM_Reinhard(hdr * sqrt(hdr), sqrt(4.0 / 27.0));
}

float3 ITM_Stanard(float3 sdr)
{
    return pow(ITM_Reinhard(sdr, sqrt(4.0 / 27.0)), 2.0 / 3.0);
}

float LinearToLogLuminance(float x, float gamma = 4.0)
{
    return log2(lerp(1, exp2(gamma), x)) / gamma;
}

float RGBToLuminance(float3 x)
{
    return dot(x, float3(0.212671, 0.715160, 0.072169));        // Defined by sRGB/Rec.709 gamut
}

float RGBToLogLuminance(float3 x, float gamma = 4.0)
{
    return LinearToLogLuminance(RGBToLuminance(x), gamma);
}

float3 ToneMapACES(float3 hdr)
{
    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
    return saturate((hdr * (A * hdr + B)) / (hdr * (C * hdr + D) + E));
}

float3 InverseToneMapACES(float3 sdr)
{
    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
    return 0.5 * (D * sdr - sqrt(((D * D - 4 * C * E) * sdr + 4 * A * E - 2 * B * D) * sdr + B * B) - B) / (A - C * sdr);
}

float3 vignette(float2 uv, float3 sdrColor)
{
    //vec2 uv = fragCoord.xy / iResolution.xy;

    // Center the UV
    float2 centered = uv - float2(0.5f, 0.5f);

    //vec4 image = texture(iChannel0, uv);
    float3 color = float3(1.0f, 1.0f, 1.0f);

    const float innerRadius = g_vignetteInnerRadius;
    const float outerRadius = g_vignetteOuterRadius;
    const float opacity = g_vignetteOpacity;

    // Create the vignette effect in black and white
    color *= 1.0 - smoothstep(innerRadius, outerRadius, length(centered));

    // Apply the vignette to the image
    color *= sdrColor;

    // Mix between the vignette version and the original to change the vignette opacity
    color = lerp(sdrColor, color, opacity);

    return color;
}

float3 getColor(float2 uv)
{
    uint2 dtid = uv * g_BufferDim;
    dtid.x = min(dtid.x, g_BufferDim.x-1);
    dtid.y = min(dtid.y, g_BufferDim.y-1);

    // Load HDR and bloom
    float3 hdrColor = ColorInput[dtid];

    hdrColor += g_BloomStrength * Bloom.SampleLevel(LinearSampler, uv, 0);
    hdrColor *= Exposure[0];

    // Tone map to SDR
    return ToneMapACES(hdrColor);
}

float3 physicalChromaticAberration(float2 uv)
{
    float3 refractiveIndex = float3(1.0, 1.015, 1.03);
    
    float2 normalizedTexCoord = float2(2.0, 2.0) * uv - float2(1.0, 1.0);    // [0, 1] -> [-1, 1]
    float3 texVec = float3(normalizedTexCoord, 1.0);
    float3 normalVec = float3(0.0, 0.0, -1.0);

    float3 redRefractionVec = refract(texVec, normalVec, refractiveIndex.r);
    float3 greenRefractionVec = refract(texVec, normalVec, refractiveIndex.g);
    float3 blueRefractionVec = refract(texVec, normalVec, refractiveIndex.b);

    float2 redTexCoord = ((redRefractionVec / redRefractionVec.z).xy + float2(1.0, 1.0)) / float2(2.0, 2.0);
    float2 greenTexCoord = ((greenRefractionVec / greenRefractionVec.z).xy + float2(1.0, 1.0)) / float2(2.0, 2.0);
    float2 blueTexCoord = ((blueRefractionVec / blueRefractionVec.z).xy + float2(1.0, 1.0)) / float2(2.0, 2.0);

    float colr = getColor(redTexCoord).r;
    float colg = getColor(greenTexCoord).g;
    float colb = getColor(blueTexCoord).b;

    return float3(colr, colg, colb);
}

float2 brownConradyDistortion(float2 uv, float scalar)
{
    uv = (uv - 0.5) * 2.0;

    if (true)
    {
        // positive values of K1 give barrel distortion, negative give pincushion
        float barrelDistortion1 = -0.02 * scalar; // K1 in text books
        float barrelDistortion2 = 0.0 * scalar; // K2 in text books

        float r2 = dot(uv, uv);
        uv *= 1.0 + barrelDistortion1 * r2 + barrelDistortion2 * r2 * r2;
    }

    return (uv / 2.0) + 0.5;
}

float3 lensChromaticAberration(float2 uv)
{
    float maxDistort = 2.0;
    float scalar = 1.0 * maxDistort;

    float3 colourScalar = float3(700.0, 560.0, 490.0);	// Based on the true wavelengths of red, green, blue light.
    colourScalar /= max(max(colourScalar.x, colourScalar.y), colourScalar.z);
    colourScalar *= 2.0;
    colourScalar *= scalar;

    float3 sourceCol = getColor(uv);

    const float numTaps = 8.0;
    float3 result = float3(0.0f, 0.0f, 0.0f);
    for (float tap = 0.0; tap < numTaps; tap += 1.0)
    {
        result.r += getColor(brownConradyDistortion(uv, colourScalar.r)).r;
        result.g += getColor(brownConradyDistortion(uv, colourScalar.g)).g;
        result.b += getColor(brownConradyDistortion(uv, colourScalar.b)).b;

        colourScalar *= 0.99;
    }

    result /= numTaps;

    return result;
}

float3 anotherChromaticAberration(float2 uv)
{
    float ChromaticAberration = g_chromaticAberrationStrength;// iMouse.y / 10.0 + 8.0;

    float2 coords = (uv - 0.5) * 2.0;
    float coordDot = dot(coords, coords);

    float2 precompute = ChromaticAberration * coordDot * coords;
    float2 uvR = uv - g_RcpBufferDim.xy * precompute;
    float2 uvB = uv + g_RcpBufferDim.xy * precompute;
    //uvB.x = min(uvB.x, 0.9999f);
    //uvB.y = min(uvB.y, 0.9999f);

    float3 color;
    color.r = getColor(uvR).r;
    color.g = getColor(uv).g;
    color.b = getColor(uvB).b;

    return color;
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float2 TexCoord = (DTid.xy + 0.5) * g_RcpBufferDim;


#ifdef ENUM_TONEMAP_DEFAULT
    // Load HDR and bloom
    float3 hdrColor = ColorRW[DTid.xy];

    hdrColor += g_BloomStrength * Bloom.SampleLevel(LinearSampler, TexCoord, 0);
    hdrColor *= Exposure[0];

    // Tone map to SDR
    float3 sdrColor = ToneMapACES(hdrColor);

    ColorRW[DTid.xy] = sdrColor;
    OutLuma[DTid.xy] = RGBToLogLuminance(sdrColor);
#endif

#ifdef ENUM_TONEMAP_VIGNETTE
    // Load HDR and bloom
    float3 hdrColor = ColorRW[DTid.xy];

    hdrColor += g_BloomStrength * Bloom.SampleLevel(LinearSampler, TexCoord, 0);
    hdrColor *= Exposure[0];

    // Tone map to SDR
    float3 sdrColor = ToneMapACES(hdrColor);

    sdrColor = vignette(TexCoord, sdrColor);

    ColorRW[DTid.xy] = sdrColor;
    OutLuma[DTid.xy] = RGBToLogLuminance(sdrColor);
#endif

#ifdef ENUM_TONEMAP_CHROMATIC_ABERRATION
    //float3 sdrColor = lensChromaticAberration(TexCoord);
    //float3 sdrColor = physicalChromaticAberration(TexCoord);
    float3 sdrColor = anotherChromaticAberration(TexCoord);

    ColorRW[DTid.xy] = sdrColor;
    OutLuma[DTid.xy] = RGBToLogLuminance(sdrColor);
#endif

#ifdef ENUM_TONEMAP_VIGNETTE_CHROMATIC_ABERRATION
    //float3 sdrColor = lensChromaticAberration(TexCoord);
    //float3 sdrColor = physicalChromaticAberration(TexCoord);
    float3 sdrColor = vignette(TexCoord, anotherChromaticAberration(TexCoord));

    //ColorRW[DTid.xy] = vignette(TexCoord, sdrColor);
    ColorRW[DTid.xy] = sdrColor;
    OutLuma[DTid.xy] = RGBToLogLuminance(sdrColor);
#endif


}