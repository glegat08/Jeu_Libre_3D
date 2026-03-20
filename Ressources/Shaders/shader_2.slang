struct VSInput {
    float3 inPosition;
    float3 normal;
    float4 inColor;
    float2 inTexCoord;
	float4 Tangeant;
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
    float3 tangent;
    float3 bitangent;
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
    //output.normal =  mul(pushConstantData.model, float4(input.normal, 0.0f)).xyz;

    // normal to ws 
    float3 N = normalize(mul(pushConstantData.model, float4(input.normal, 0.0f)).xyz);
    // tangeant to ws 
    float3 T = normalize(mul(pushConstantData.model, float4(input.Tangeant.xyz, 0.0f)).xyz);
    // prosses bi tengeant * 1.0f
    float3 B = cross(N, T) * input.Tangeant.w;

    output.normal = N;
    output.tangent = T;
    output.bitangent = B;

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
ConstantBuffer<Sampler2D> sampler; //baseColor
[[vk::binding(0, 3)]]
ConstantBuffer<Sampler2D> metallicRoughnessSampler;
[[vk::binding(0, 4)]]
ConstantBuffer<Sampler2D> emissiveSampler;
[[vk::binding(0, 5)]]
ConstantBuffer<Sampler2D> normalSampler;



[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {
    // float metalness = 1.f;
    float oclusion = metallicRoughnessSampler.Sample(vertIn.fragTexCoord).r;
	float metalness = metallicRoughnessSampler.Sample(vertIn.fragTexCoord).b;
	float roughness = metallicRoughnessSampler.Sample(vertIn.fragTexCoord).g;
	float3 emessive = pow(emissiveSampler.Sample(vertIn.fragTexCoord).rgb, 2.2f.xxx);

    // get normal map rgb
	float3 normalMap = normalSampler.Sample(vertIn.fragTexCoord).rgb;

    // convert -1 1 to 0 1 
    normalMap.xy = normalMap.xy * 2.0f - 1.0f;

    // create matrix TBN and transpose it 
    float3x3 TBM = transpose(float3x3(vertIn.tangent, vertIn.bitangent, vertIn.normal));

    // compute the normal
    float3 normal = normalize(mul(TBM,normalMap));

   // float3 normal = vertIn.normal;
    float3 result = float(0.f).xxx;
    float3 baseColor = pow(sampler.Sample(vertIn.fragTexCoord).rgb, 2.2f.xxx);
    result += baseColor * (1 - metalness) * oclusion * 0.02f;

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
            float3 N = normalize(normal);
            float3 V = normalize(vertIn.eyePos - vertIn.worldspacePosition.xyz);
            float3 H = normalize(L + V);

            

            // base color
            float3 Kd = lerp(baseColor, 0.0f.xxx, metalness);
            // specular
            float3 Ks = lerp(1.0f.xxx, baseColor, metalness);
            // shiny
            float Ns = 1.0f/pow(roughness,2.0f);
            // Intensity
            float Intensity = currentLight.intensity;
            // distance betwen light and wsPos
            float r = length(lightPos - vertIn.worldspacePosition.xyz);
            // beause its a shpere
            float attenuation = (Intensity / (4.0f * 3.14159265f * pow(r, 2.0f)));
			
            float3 diffuse = max(dot(N, L), 0.0);
            float3 specular = Ks * pow(max(dot(N, H), 0.0f), Ns);
            float3 Fr = (Kd * 1.0f/3.14159265f) +  specular;
            float3 Ej = diffuse * attenuation;

             result += (Fr * Ej) * lightColor;
        }
        // Directional
        else if (currentLight.type[0] == 1)
        {
            float3 lightDir = normalize(currentLight.dir.xyz);
            // light color
            float3 lightColor = currentLight.color.xyz;
            float3 N = normalize(normal);
            float3 V = normalize(vertIn.eyePos - vertIn.worldspacePosition.xyz);
            float3 H = normalize(-lightDir + V);


            // base color
            float3 Kd = lerp(baseColor, 0.0f.xxx, metalness);
            // specular
            float3 Ks = lerp(1.0f.xxx, baseColor, metalness);
            // shiny
            float Ns = 1.0f/pow(roughness,2.0f);
			
            float3 diffuse = max(dot(N, -lightDir), 0.0);
            float3 specular = Ks * pow(max(dot(N, H), 0.0f), Ns);
            float3 Fr = (Kd * 1.0f/3.14159265f) +  specular;
            float3 Ej = diffuse;
			
			
             result += (Fr * Ej) * lightColor ;
        }
        // spot Light
        else
        {
            // light postition temporar
            float3 lightPos =  currentLight.pos.xyz;
            // light color
            float3 lightColor = currentLight.color.rgb;
            float3 L = normalize(lightPos - vertIn.worldspacePosition.xyz);
            float3 N = normalize(normal);
            float3 V = normalize(vertIn.eyePos - vertIn.worldspacePosition.xyz);
            float3 H = normalize(L + V);

          

            // base color
            float3 Kd = lerp(baseColor, 0.0f.xxx, metalness);
            // specular
            float3 Ks = lerp(1.0f.xxx, baseColor, metalness);
            // shiny
            float Ns = 1.0f/pow(roughness,2.0f);
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
			 
            float3 diffuse = max(dot(N, L), 0.0);
            float3 specular = Ks * pow(max(dot(N, H), 0.0f), Ns);
            float3 Fr = (Kd * 1.0f/3.14159265f) +  specular;
            float3 Ej = diffuse * attenuation * spotAttenuation;
			
		

             result += (Fr * Ej) * lightColor;
        }
    }
    result += emessive * 3.0f;

    return float4(result,1.0f);
}