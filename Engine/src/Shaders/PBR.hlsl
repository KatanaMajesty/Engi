#include "BufferDefines.hlsli"
#include "LayoutDefines.hlsli"
#include "Lighting.hlsli"
#include "TextureDefines.hlsli"
#include "BRDF.hlsli"
#include "IBL.hlsli"
#include "Common.hlsli"

Texture2D<float3> t_gbufferAlbedo : register(t21);
Texture2D<float4> t_gbufferNormals : register(t22);
Texture2D<float3> t_gbufferEmission : register(t23);
Texture2D<float2> t_gbufferRoughnessMetalness : register(t24);
Texture2D<uint> t_gbufferObjectID : register(t25);

Texture2D<float> t_depthBuffer : register(t26);

float3 GetAbsolutePosition(SamplerState samplerState, float2 uv)
{
    float x = uv.x * 2.0 - 1.0;
    float y = -uv.y * 2.0 + 1.0;
    float z = t_depthBuffer.Sample(samplerState, uv);
    float4 clip = float4(x, y, z, 1.0);
    
    float4 view = mul(clip, g_projInv);
    view /= view.w;
    
    float4 world = mul(view, g_viewsInv[0]);
    return world.xyz;
}

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float2 uv : TEX_COORD;
    float3 cameraPos : CAMERA_POS;
};

VS_OUTPUT vs_main(uint vid : SV_VertexID)
{
    VS_OUTPUT output;
    output.pos = float4(g_xy[vid + 1], g_xy[vid], 0.0f, 1.0f);
    output.uv = float2(g_uv[(vid * 2)], g_uv[(vid * 2) + 1]);
    output.cameraPos = g_viewsInv[0][3].xyz;
    return output;
}

float4 ps_main(VS_OUTPUT input) : SV_Target0
{
    SamplerState ss = g_samplerLinear;
    float3 albedo = t_gbufferAlbedo.Sample(ss, input.uv);

    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    float3 fragpos = GetAbsolutePosition(ss, input.uv);
    float3 V = normalize(input.cameraPos - fragpos);
    
    float4 normals = t_gbufferNormals.Sample(ss, input.uv);
    // float3 N = unpackOctahedron(normals.xy);
    float3 N = unpackOctahedron(normals.zw);
    
    float3 Reflection = reflect(-V, N);
    
    float2 mr = t_gbufferRoughnessMetalness.Sample(ss, input.uv);
    float3 metalness = mr.g;
    float roughness = mr.r;
    
    float3 emission = t_gbufferEmission.Sample(ss, input.uv);
    
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
        
    return float4(Lo + emission, 1.0f);
}