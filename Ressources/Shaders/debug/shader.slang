struct VSInput {
    float3 inPos   : POSITION;
    float3 inColor : COLOR0;  
};

struct UniformBuffer {
    float4x4 InverseView;
    float4x4 view;
    float4x4 proj;
};


struct PushConstant
{
    float4x4 model;
}

struct Data
{
    UniformBuffer CamUbo;
}
ParameterBlock<Data> data;

[[vk::push_constant()]]
ConstantBuffer<PushConstant> pushConstantData;

struct VSOutput
{
    float4 pos : SV_Position;
    float3 color : COLOR0;
};

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    float4 wsPos = mul(pushConstantData.model, float4(input.inPos, 1.0f));
    output.pos = mul(data.CamUbo.proj, mul(data.CamUbo.view, wsPos));
    output.color = input.inColor;

    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_TARGET {
    return float4(input.color, 1.0f);
}