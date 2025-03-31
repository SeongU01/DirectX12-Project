cbuffer cbPerObject : register(b0)
{
    float4x4 world;
}

cbuffer cbPass : register(b1)
{
    float4x4 view;
    float4x4 invView;
    float4x4 proj;
    float4x4 invProj;
    float4x4 viewProj;
    float4x4 invViewProj;
    float3 eyePos;
    float padd1;
    float2 renderTargetSize;
    float2 invRenderTargetSize;
    float nearZ;
    float farZ;
    float totalTime;
    float delta;
}

struct VsInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VsOutput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

VsOutput vs_main(VsInput input)
{
    VsOutput output;
    float4 worldPosition = mul(float4(input.pos, 1.f), world);
    output.pos = mul(worldPosition, viewProj);
    output.color = input.color;
    return output;
}

float4 ps_main(VsOutput input) : SV_Target
{
    return input.color;
}