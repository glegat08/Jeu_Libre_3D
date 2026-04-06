struct VSInput {
    float3 inPosition;
    float3 normal;
    float4 inColor;
    float2 inTexCoord;
	float4 Tangeant;
    int4   inJoints;
    float4 inWeights;
};

struct UniformBuffer {
    float4x4 InverseView;
    float4x4 view;
    float4x4 proj;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UniformBuffer> CamUbo;

[[vk::binding(1, 0)]]
StructuredBuffer<float4x4> boneData;

struct PushConstantData
{
    float4x4 model;
    int boneOffset;
    int pad1;
    int pad2;
    int pad3;
};

[[vk::push_constant()]]
ConstantBuffer<PushConstantData> pushConstantData;

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
VSOutput vertMain(VSInput input) 
{
VSOutput output;

    float weightSum = dot(input.inWeights, float4(1.0));
    float4 w = (weightSum > 0.0) ? (input.inWeights / weightSum) : float4(1.0, 0.0, 0.0, 0.0);

    int4 j = clamp(input.inJoints + int4(pushConstantData.boneOffset), int4(0), int4(9999));

    float4x4 skinMat = 
        boneData[j.x] * w.x +
        boneData[j.y] * w.y +
        boneData[j.z] * w.z +
        boneData[j.w] * w.w;

    float4 localPos = mul(skinMat, float4(input.inPosition, 1.0f));
    float3 localNormal = mul((float3x3)skinMat, input.normal);
    float3 localTangent = mul((float3x3)skinMat, input.Tangeant.xyz);

    float4 wsPos = mul(pushConstantData.model, localPos);
    output.pos = mul(CamUbo.proj, mul(CamUbo.view, wsPos));
    output.worldspacePosition = wsPos.xyz;
    output.eyePos = mul(CamUbo.InverseView, float4(0, 0, 0, 1)).xyz;
    
    output.fragTexCoord = input.inTexCoord;
    output.fragColor = input.inColor;

    float3 N = normalize(mul((float3x3)pushConstantData.model, localNormal));
    float3 T = normalize(mul((float3x3)pushConstantData.model, localTangent));
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * input.Tangeant.w);

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