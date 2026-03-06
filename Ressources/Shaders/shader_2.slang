struct VSInput {
    float3 inPosition;
    float3 normal;
    float4 inColor;
    float2 inTexCoord;
};

struct UniformBuffer {
    float4x4 InverseView;
    float4x4 view;
    float4x4 proj;
};

struct Transform
{
    float4x4 model;
}

struct Data
{
    UniformBuffer CamUbo;
    Sampler2D texture;
}
ParameterBlock<Data> data;

[[vk::push_constant()]]
ConstantBuffer<Transform> pushConstantData;

struct VSOutput
{
    float4 pos : SV_Position;
    float3 normal;
    float3 worldspacePosition;
    float3 eyePos;
    float2 fragTexCoord;
    float4 fragColor;
};


[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;

    float4 wsPos = mul(pushConstantData.model, float4(input.inPosition, 1.0f));
    output.pos = mul(data.CamUbo.proj, mul(data.CamUbo.view, wsPos));
    output.worldspacePosition = wsPos.xyz;
    output.eyePos = mul(data.CamUbo.InverseView, float4(0.0f.xxx, 1.0f)).xyz;
    output.fragTexCoord = input.inTexCoord;
    output.fragColor = input.inColor;
    output.normal =  mul(pushConstantData.model, float4(input.normal, 0.0f)).xyz;
    return output;
}

struct LightData
{
    float4 pos;
    float4 dir;
    float4 color;
    float4 specColor;
    float intensity;
    float openAngle;
    float AdditionalSmooth;
    float shiny;
    uint4 type;
};

[[vk::binding(0, 1)]]
StructuredBuffer<LightData> Alllights;

[[vk::binding(1, 1)]]
ConstantBuffer<uint> lightCount;
[[vk::binding(0, 2)]]
ConstantBuffer<Sampler2D> sampler;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    float3 result = float(0.0f).xxx;
    for (int i = 0; i < lightCount; ++i)
    {
        LightData currentLight = Alllights[i];

        // point Light
        if (currentLight.type[0] == 0)
        {
            float3 lightPos = currentLight.pos.xyz;
            // light color
            float3 lightColor = currentLight.color.xyz;
            float3 L = normalize(lightPos - vertIn.worldspacePosition.xyz);
            float3 N = normalize(vertIn.normal);
            float3 V = normalize(vertIn.eyePos - vertIn.worldspacePosition.xyz);
            float3 H = normalize(L + V);

            // base color
            float3 Kd = sampler.Sample(vertIn.fragTexCoord).rgb;
            Kd = pow(Kd, 2.2f.xxx);
            // specular color
            float3 Ks = currentLight.specColor.xyz;
            // shiny
            float Ns = currentLight.shiny;
            // Intensity
            float Intensity = currentLight.intensity;
            // distance betwen light and wsPos
            float r = length(lightPos - vertIn.worldspacePosition.xyz);
            // beause its a shpere
            float attenuation = (Intensity / (4.0f * 3.14159265f * pow(r, 2.0f)));
            float diffuse = max(dot(N, L), 0.0);
            float specular = ((Ns + 2.0f) / (2.0f * 3.14159265f)) * pow(max(dot(N, H), 0.0f), Ns);

            float3 Fr = Kd + Ks * specular;
            float Ej = diffuse * attenuation;

            result += Fr * Ej * lightColor;
        }
        // Directional
        else if (currentLight.type[0] == 1)
        {
            float3 lightDir = normalize(currentLight.dir.xyz);
            // light color
            float3 lightColor = currentLight.color.xyz;
            float3 N = normalize(vertIn.normal);
            float3 V = normalize(vertIn.eyePos - vertIn.worldspacePosition.xyz);
            float3 H = normalize(-lightDir + V);

            // base color
            float3 Kd = sampler.Sample(vertIn.fragTexCoord).rgb;
            // specular color
            float3 Ks = currentLight.specColor.xyz;
            // shiny
            float Ns = currentLight.shiny;
            float diffuse = max(dot(N, -lightDir), 0.0);
            float specular = ((Ns + 2.0f) / (2.0f * 3.14159265f)) * pow(max(dot(N, H), 0.0f), Ns);

            float3 Fr = Kd + Ks * specular;
            float Ej = diffuse;

             result += Fr * Ej * lightColor;
        }
        // spot Light
        else
        {
            // light postition temporar
            float3 lightPos =  currentLight.pos.xyz;
            // light color
            float3 lightColor = currentLight.color.xyz;
            float3 L = normalize(lightPos - vertIn.worldspacePosition.xyz);
            float3 N = normalize(vertIn.normal);
            float3 V = normalize(vertIn.eyePos - vertIn.worldspacePosition.xyz);
            float3 H = normalize(L + V);

            // base color
            float3 Kd = sampler.Sample(vertIn.fragTexCoord).rgb;
            // specular color
            float3 Ks = currentLight.specColor.xyz;
            // shiny
            float Ns = currentLight.shiny;
            // Intensity
            float Intensity = currentLight.intensity;
            // distance betwen light and wsPos
            float r = length(lightPos - vertIn.worldspacePosition.xyz);

            float3 lightDir = normalize(currentLight.dir.xyz);
            float spotAngle = dot(normalize(-lightDir), L);
            float openAngle = currentLight.openAngle;
            float cosInner = cos(openAngle);
            float cosOuter = cos(openAngle +currentLight.AdditionalSmooth);
            float spotAttenuation = smoothstep(cosOuter, cosInner, spotAngle);

            // beause its a shpere
            float spotNormalization = 1.0f / (2.0f * 3.14159265f * (1.0f - cosOuter));
            float attenuation = ((Intensity * spotNormalization) / pow(r, 2.0f));
            float diffuse = max(dot(N, L), 0.0);
            float specular = ((Ns + 2.0f) / (2.0f * 3.14159265f)) * pow(max(dot(N, H), 0.0f), Ns);

            float3 Fr = Kd + Ks * specular;
            float Ej = diffuse * attenuation * spotAttenuation;

             result += Fr * Ej * lightColor;
        }
    }

    return float4(result,1.0f);
}