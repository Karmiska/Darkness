
#include "../Common.hlsli"

struct PSInput
{
    float4 position         : SV_Position0;
    float2 uv               : TEXCOORD0;
};

Texture2D<float4> image;
Buffer<float> blurKernel;
sampler imageSampler;

cbuffer Constants
{
    float2 texelSize;
    int kernelSize;
};

float normpdf(float x, float sigma)
{
    return 0.39894*exp(-0.5*x*x / (sigma*sigma)) / sigma;
}

float4 main(PSInput input) : SV_Target
{
    int ksize = 2;
    float BSIGMA = 0.7f;
    //return image.Sample(imageSampler, input.uv).r;

    float result = 0.0;
    float z = 0.0;
    float c = image.Sample(imageSampler, input.uv).r;
    float bZ = 1.0 / normpdf(0.0, BSIGMA);

    for (int i = -ksize; i <= ksize; ++i)
    {
        for (int j = -ksize; j <= ksize; ++j)
        {
            float2 offset = float2(float(i), float(j)) * texelSize;
            float cc = image.Sample(imageSampler, input.uv + offset).r;
            float factor = normpdf(cc - c, BSIGMA) * bZ * blurKernel[ksize + j] * blurKernel[ksize + i];
            z += factor;
            result += factor*cc;
        }
    }
    return result / z;
    
    /*float result = 0.0;
    for (int x = -ksize; x < ksize; ++x)
    {
        for (int y = -ksize; y < ksize; ++y)
        {
            float2 offset = float2(float(x), float(y)) * texelSize;
            result += image.Sample(imageSampler, input.uv + offset).r;
        }
    }
    return result / ((ksize * 2) * (ksize * 2));*/
    
}
