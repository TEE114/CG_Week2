struct PSInput
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

// 这里必须改成 main
float4 main(PSInput input) : SV_Target
{
    return input.color;
}