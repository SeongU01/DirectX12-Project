#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif
#include "utility.hlsl"

cbuffer cbPerObj : register(b0)
{
    float4x4 world;
}

cbuffer cbMaterial : register(b1)
{
    float4 diffuseAlbedo;
    float3 fresnelR0;
    float roughness;
    float4x4 matTransform;
}

cbuffer cbPass : register(b2)
{
    matrix view;
    matrix invView;
    matrix proj;
    matrix invProj;
    matrix viewProj;
    matrix invViewProj;
    float3 eyePos;
    float padd1;
    float2 renderTargetSize;
    float2 invRenderTargetSize;
    float nearZ;
    float farZ;
    float totalTime;
    float delta;
    float4 ambientLight;
    
    Light lights[MaxLights];
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
};

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 worldPos : POSITION;
    float3 worldNormal : NORMAL;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;
    float4 worldPos = mul(float4(input.pos, 1.f), world);
    output.worldPos = worldPos.xyz;
    output.worldNormal = mul(input.normal, (float3x3)world);
    output.pos = mul(worldPos, viewProj);
    return output;
}

float4 ps_main(VS_OUTPUT input):SV_Target
{
    input.worldNormal = normalize(input.worldNormal);
    float3 toEye = normalize(eyePos - input.worldPos);
    float4 ambient = ambientLight * diffuseAlbedo;
    const float shininess = 1.f - roughness;
    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    float3 shadowFactor = 1.f;
    float4 directLight = ComputeLighting(lights, mat, input.worldPos, input.worldNormal, toEye, shadowFactor);
    float4 col = ambient + directLight;
    col.a = diffuseAlbedo.a;
    return col;
}