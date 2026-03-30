
cbuffer MVPBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix proj;
}
// Constant buffer for Model-View-Projection matrices

struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PSInput main(VSInput input)
{
    PSInput output;
    float4 p = float4(input.pos, 1.0f);
    p = mul(p, model);
    p = mul(p, view);
    p = mul(p, proj);
    output.pos = p;
    output.color = input.color;
    return output;
}