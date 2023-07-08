
float3 prefilteredColor = environmentSpecular.SampleLevel(tri_sampler, probeValue, roughness * MaxReflectionLod).xyz;

bool ssrEnabled = any(ssrSample);
if (ssrEnabled)
	prefilteredColor = ssrSample;

float3 F = fresnelSchlickRoughness(max(dot(normal, V), 0.0), F0, roughness) * 0.7;
float2 envBRDF = environmentBrdfLut.Sample(point_sampler, float2(min(max(dot(normal, V), 0.0), 0.99999), roughness)).rg;
float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);


// COULD DO THIS AFTER VOXEL STUFF
float3 environIrradianceSample = sampleEnvironment(normal, 0.0f);
float3 kS = F;
float3 kD = 1.0 - kS;
kD *= 1.0f - metalness;
float3 irradiance = environIrradianceSample;
float3 diffuse = irradiance * albedo.xyz;

float3 ambient = float3(0.0f, 0.0f, 0.0f); 
if (ssrEnabled)
{
	ambient = max((kD* diffuse + specular)* occlusion * environmentStrength, float3(0.0, 0.0, 0.0));
}
else
{
	ambient = max((kD* diffuse + specular)* occlusion * environmentStrength, float3(0.0, 0.0, 0.0));
}


float3 color = (ambient + Lo);

color *= exposure;
//color = 1.0 - exp(-exposure * color);
