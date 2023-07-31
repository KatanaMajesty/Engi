#include "BufferDefines.hlsli"
#include "LayoutDefines.hlsli"
#include "Lighting.hlsli"
#include "TextureDefines.hlsli"
#include "BRDF.hlsli"
#include "IBL.hlsli"
#include "Common.hlsli"

Texture2D<float> t_dissolutionTexture : register(t20);

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 worldPos : WORLD_POSITION;
    float3 worldNormal : WORLD_NORMAL;
    float3 cameraPos : CAMERA_POS;
    float2 texCoords : TEXCOORD;
    uint hasTexCoords : HAS_TEXCOORD;
    float3 color : INSTANCE_COLOR;
    float3x3 TBN : TBN_MAT;
    float time : INSTANCE_TIME;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
    float3 worldNormal = convertToOrthogonalBasis(input.modelToWorld, convertToOrthogonalBasis(g_meshToModel, input.meshNormal));
    float3 modelPos = mul(float4(input.meshPosition, 1.0f), g_meshToModel).xyz;
    float3 worldPos = mul(float4(modelPos, 1.0f), input.modelToWorld).xyz;

    float4x4 viewProj = mul(g_views[0], g_proj);
    float4 pos = mul(float4(worldPos, 1.0f), viewProj);
    VS_OUTPUT output;
    output.pos = pos;
    output.worldPos = worldPos;
    output.worldNormal = worldNormal;
    output.cameraPos = g_viewsInv[0][3].xyz;
    output.texCoords = input.meshTexCoords;
    output.hasTexCoords = g_meshHasTexCoords;
    output.color = input.color;
    output.time = input.time;
    if (g_useNormalMap)
    {
        float3x3 TBN = constructTBN(input.modelToWorld, input.meshTangent, input.meshBitangent, worldNormal);
        output.TBN = TBN;
    }
    return output;
}

float4 ps_main(VS_OUTPUT input) : SV_Target0
{
    float3 albedo = input.color;
    float alpha = 1.0f;
    if (g_useAlbedoTexture && input.hasTexCoords)
    {
        float4 a = processAlbedoMap(g_activeSampler, input.texCoords);
        albedo = a.xyz;
        alpha = a.a;
    }

    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    float3 fragpos = input.worldPos;
    float3 V = normalize(input.cameraPos - fragpos);
    float3 N = normalize(input.worldNormal);
    if (g_useNormalMap && input.hasTexCoords)
        N = normalize(processNormalMap(g_activeSampler, input.texCoords, input.TBN));

    float3 Reflection = reflect(-V, N);
    float3 metalness = float3(g_metallic, g_metallic, g_metallic);
    if (g_useMetalnessMap && input.hasTexCoords)
        metalness = processMetalnessMap(g_activeSampler, input.texCoords).xyz;

    float roughness = g_roughness;
    if (g_useRoughnessMap && input.hasTexCoords)
        roughness = processRoughnessMap(g_activeSampler, input.texCoords).r;

    roughness = clamp(roughness, 0.01, 1.0);
    
    float3 F0 = calcF0(albedo, metalness);
    uint lightIndex;
    for (lightIndex = 0; lightIndex < g_numDirLights; ++lightIndex)
    {
        ENGI_DirectionalLight light = b_dirLights[lightIndex];
        float r = light.radius;

        float3 L = normalize(-light.direction);
        float height = dot(L, N); // distance from surface to spherelight's center
        float falloff = min(1.0f, (height + r) / (2.0f * r));
        float3 Lr = light.ambient * falloff;
        
        float3 H = normalize(V + L);
        float3 Fd = BRDF_CookTorrance_Lambertian(F0, albedo, metalness, N, L);
        float3 Fs = BDRF_CookTorrance_Specular(F0, roughness, N, V, H, L);
        float shadow = 1.0f;
        if (g_useShadowmapping)
            shadow = ShadowContribution_Directional_PCF(fragpos, light.worldToLight, light.depthArraySlice);
        
        Lo += shadow * (Fd + Fs) * Lr;
    }

    for (lightIndex = 0; lightIndex < g_numPointLights; ++lightIndex)
    {
        ENGI_PointLight light = b_pointLights[lightIndex];
        float3 L = light.position - fragpos;
        float3 lightDir = normalize(-L); // for shadowmapping
        
        float r = light.radius;
        float d2 = getDistance2(L);
        float height = dot(L, N); // distance from surface to spherelight's center
        float falloff = min(1.0f, (height + r) / (2.0f * r));
        float3 Lr = calcSolidAngle(r * r, d2) * light.intensity * light.color * falloff;
        L = normalize(sphereClosestPoint_Karis(L, r, Reflection));

        float3 H = normalize(V + L);
        float3 Fd = BRDF_CookTorrance_Lambertian(F0, albedo, metalness, N, L);
        float3 Fs = BDRF_CookTorrance_Specular(F0, roughness, N, V, H, L);
        float shadow = 1.0f;
        if (g_useShadowmapping)
            shadow = ShadowContribution_Point(fragpos, light.worldToLight, lightDir, light.depthArraySlice);
        
        Lo += shadow * (Fd + Fs) * Lr;
    }
    
    for (lightIndex = 0; lightIndex < g_numSpotLights; ++lightIndex)
    {
        ENGI_SpotLight light = b_spotLights[lightIndex];
        float3 L = light.position - fragpos;
        
        float3 cookieAttenuation = 1.0f;
        if (light.useSpotlightMask)
        {
            float sin = sqrt(1.0f - pow(light.outerCutoff, 2.0f));
            float tan = sin / light.outerCutoff;
            cookieAttenuation = getCookieAttenuation(tan, fragpos, light.worldToLocal);
        }
        
        float r = light.radius;
        float d2 = getDistance2(L);
        float height = dot(L, N); // distance from surface to spherelight's center
        float falloff = min(1.0f, (height + r) / (2.0f * r));
        float cutoffIntensity = getSpotlightIntensity(normalize(L), normalize(light.direction), light.cutoff, light.outerCutoff);
        float3 Lr = light.color * light.intensity * cookieAttenuation * cutoffIntensity * calcSolidAngle(r * r, d2) * falloff;
        L = normalize(sphereClosestPoint_Karis(L, r, Reflection));
        
        float3 H = normalize(V + L);
        float3 Fd = BRDF_CookTorrance_Lambertian(F0, albedo, metalness, N, L);
        float3 Fs = BDRF_CookTorrance_Specular(F0, roughness, N, V, H, L);
        float shadow = 1.0f;
        if (g_useShadowmapping)
            shadow = ShadowContribution_Spot(fragpos, light.worldToLight, light.depthArraySlice);
        
        Lo += shadow * (Fd + Fs) * Lr;
    }

    float3 ambient = albedo * 0.04f;
    if (g_useIBL)
    {
        float NdotV = max(dot(N, V), MATH_EPS);
        ambient = float3(0.0f, 0.0f, 0.0f);
        ambient += addDiffuseReflection(albedo, N, metalness);
        ambient += addSpecularReflection(roughness, NdotV, F0, Reflection);
    }
    Lo += ambient;
    
    if (input.hasTexCoords)
    {
        float dissolution = t_dissolutionTexture.Sample(g_activeSampler, input.texCoords).r;
        float dA = input.time / g_dissolutionTime;
        
        if (dissolution > dA)
            discard;
        
        // Highlight color for dissolved pixels
        float3 highlightColor = Lo * 32.0f; // Bright white highlight color
        
        // Attenuation factor for the highlight over time
        float eps = 0.05f;
        float dAEps = dA - eps;
        float invEps = 1.0f / eps;
        float attenuationFactor = saturate((dissolution - dAEps) * invEps);
        
        // Interpolate between the object's color and the highlight color
        float3 interpolatedColor = lerp(Lo, highlightColor, attenuationFactor);
        
        // Apply the highlight color
        return float4(interpolatedColor, alpha);
    }
    else return float4(Lo, alpha);
        
}