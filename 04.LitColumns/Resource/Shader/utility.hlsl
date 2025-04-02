#define MaxLights 16

struct Light
{
    float3 strength;
    float falloffStart;
    float3 Direction;
    float falloffend;
    float3 position;
    float spotPower;
};

struct Material
{
    float4 DiffuseAlbedo;
    float3 FrenelR0;
    float shiness;
};

float CalculateAttenuation(float d, float falloffStart, float falloffEnd)
{
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

// R0 = ((n-1)/(n-2))^2
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));
    float f0 = 1.f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.f - R0) * (pow(f0, 5));
    return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat)
{
    const float m = mat.shiness * 256.f;
    float3 halfVec = normalize(toEye + lightVec);
    float roughnessFactor = (m + 8.f) * pow(max(dot(halfVec, normal), 0.f), m) / 8.f;
    float3 fresnelFactor = SchlickFresnel(mat.FrenelR0, halfVec, lightVec);
    float3 specAlbedo = fresnelFactor * roughnessFactor;
    specAlbedo = specAlbedo / (specAlbedo + 1.f);
    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float3 ComputeDirectionalLight(Light l, Material mat, float3 normal, float3 toEye)
{
    float3 lightVec = -l.Direction;
    float ndotl = saturate(dot(lightVec, normal));
    //float toonSteps = 4.f;
    //float toonFactor = floor(ndotl * toonSteps) / toonSteps;
    //mat.DiffuseAlbedo.rgb *= toonFactor;
    float3 lightStrength = l.strength * ndotl;
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float3 ComputePointLight(Light l, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = l.position - pos;
    float d = length(lightVec);
    if(d>l.falloffend)
        return 0.f;
    lightVec /= d;
    float3 ndotl = saturate(dot(normal, lightVec));
    float3 lightStrength = l.strength * ndotl;
    float att = CalculateAttenuation(d, l.falloffStart, l.falloffend);
    lightStrength *= att;
    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float3 ComputeSpotLight(Light L, Material mat, float3 pos, float3 normal, float3 toEye)
{
    float3 lightVec = L.position - pos;
    float d = length(lightVec);
    
    if (d > L.falloffend)
        return 0.0f;
    
    lightVec /= d;
    
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.strength * ndotl;
    
    float att = CalculateAttenuation(d, L.falloffStart, L.falloffend);
    lightStrength *= att;
    
    float spotFactor = pow(max(dot(-lightVec, L.Direction), 0.0f), L.spotPower);
    lightStrength *= spotFactor;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}


float4 ComputeLighting(Light gLights[MaxLights], Material mat, float3 pos,
                        float3 normal, float3 toEye,float3 shadowFactor)
{
    float3 result = 0.0f;

    int i = 0;

#if (NUM_DIR_LIGHTS > 0)
    for(i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        result += shadowFactor[i] * ComputeDirectionalLight(gLights[i], mat, normal, toEye);
    }
#endif

#if (NUM_POINT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS+NUM_POINT_LIGHTS; ++i)
    {
        result += ComputePointLight(gLights[i], mat, pos, normal, toEye);
    }
#endif

#if (NUM_SPOT_LIGHTS > 0)
    for(i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        result += ComputeSpotLight(gLights[i], mat, pos, normal, toEye);
    }
#endif 

    return float4(result, 0.0f);
}