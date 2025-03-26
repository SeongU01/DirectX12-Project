cbuffer cbPerObject : register(b0)
{
    float4x4 WVP;
}

struct VertexIn
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

VertexOut vs_main(VertexIn vin)
{
    VertexOut output;
    output.pos = mul(float4(vin.pos, 1.f), WVP);
    output.color = vin.color;
    return output;
}

float4 ps_main(VertexOut pin) : SV_Target
{
    return pin.color;
}