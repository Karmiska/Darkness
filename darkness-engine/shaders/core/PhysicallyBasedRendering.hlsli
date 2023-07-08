
// The normal distribution function DD or the specular distribution statistically 
// approximates the ratio of microfacets aligned to some (halfway) vector hh.
float distributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;

    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nominator = a2;
    float denominator = NdotH2 * (a2 - 1.0) + 1.0;
    denominator = PI * denominator * denominator;

    return nominator / denominator;
}

float GGX_D(float dotNH, float roughness)
{
    float alpha = roughness * roughness;
    float alphaSqr = alpha * alpha;
    float pi = 3.14159f;
    float denom = dotNH * dotNH *(alphaSqr - 1.0) + 1.0f;

    float D = alphaSqr / (pi * denom * denom);
    return D;
}

// The Fresnel equation (pronounced as Freh-nel) describes the ratio of light that 
// gets reflected over the light that gets refracted, which varies over the angle we're looking at a surface.
// this function "really works" only for dielectric mateirals. (ie. non-metal)
float3 fresnelSchlick(float cosTheta, float3 Fo_baseReflectivity)
{
    float3 Fo = Fo_baseReflectivity;
    return Fo + (1.0 - Fo) * pow(1 - cosTheta, 5.0);
}

// The geometry function statistically approximates the ratio of microfacets 
// that overshadow each other causing light rays to lose their energy in the process.
float geometryFunctionSchlicGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float nominator = NdotV;
    float denominator = NdotV * (1.0 - k) + k;
    return nominator / denominator;
}

float geometryFunctionSmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = geometryFunctionSchlicGGX(NdotV, roughness);
    float ggx2 = geometryFunctionSchlicGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float2 GGX_FV(float dotLH, float roughness)
{
    float alpha = roughness * roughness;

    // F
    float F_a, F_b;
    float dotLH5 = pow(1.0f - dotLH, 5);
    F_a = 1.0f;
    F_b = dotLH5;

    // V
    float vis;
    float k = alpha / 2.0f;
    float k2 = k * k;
    float invK2 = 1.0f - k2;
    vis = rcp(dotLH*dotLH*invK2 + k2);

    return float2(F_a*vis, F_b*vis);
}

float GGX_OPT3(float3 N, float3 V, float3 L, float roughness, float F0)
{
    float3 H = normalize(V + L);

    float dotNL = saturate(dot(N, L));
    float dotLH = saturate(dot(L, H));
    float dotNH = saturate(dot(N, H));

    float D = GGX_D(dotNH, roughness);
    float2 FV_helper = GGX_FV(dotLH, roughness);
    float FV = F0 * FV_helper.x + (1.0f - F0)*FV_helper.y;
    float specular = dotNL * D * FV;

    return specular;
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
