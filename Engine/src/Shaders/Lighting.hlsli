#ifndef __ENGI_LIGHTING_HLSL__
#define __ENGI_LIGHTING_HLSL__

#include "SamplerDefines.hlsli"
#include "LightCasters.hlsli"
#include "Common.hlsli"

StructuredBuffer<ENGI_DirectionalLight> b_dirLights : register(t8);
StructuredBuffer<ENGI_SpotLight> b_spotLights : register(t9);
StructuredBuffer<ENGI_PointLight> b_pointLights : register(t10);
Texture2D<float4> t_spotlightMask : register(t11);
Texture2DArray<float> t_dirDepthmapArray : register(t12);
Texture2DArray<float> t_spotDepthmapArray : register(t13);
TextureCubeArray<float> t_pointDepthmapArray : register(t14);

float inverseSquareLaw(float i, float d2)
{
    return i / d2;
}

float getDistance2(float3 toLight)
{
    return dot(toLight, toLight);
}

float getSpotlightIntensity(float3 NToLight, float3 NLightDir, float cutoff, float outerCutoff)
{
    float th = dot(NToLight, -NLightDir);
    float epsilon = cutoff - outerCutoff;
    float intensity = clamp((th - outerCutoff) / epsilon, 0.0f, 1.0f);
    return intensity;
}

float3 getCookieAttenuation(float cutoffTan, float3 worldPos, float4x4 worldToLight)
{
    float4 vertexPos_InLight = mul(float4(worldPos, 1.0f), worldToLight);
    float radius = vertexPos_InLight.z * cutoffTan;
        
    float u = remap(-radius, radius, 0.0f, 1.0f, vertexPos_InLight.x);
    float v = remap(-radius, radius, 0.0f, 1.0f, vertexPos_InLight.y);
    float3 cookieAttenuation = t_spotlightMask.Sample(g_activeSampler, float2(u, -v)).rgb;
    return cookieAttenuation;
}

float ShadowPCF_2DOrtho(SamplerComparisonState state, Texture2DArray<float> depthmapArray, float2 uv, uint arrayslice, float z)
{
    float depth = 0.0;
    float3 sampleCoords = float3(uv, arrayslice);
    depth += depthmapArray.SampleCmp(state, sampleCoords, z);
    depth += depthmapArray.SampleCmp(state, sampleCoords, z, int2(1, 0));
    depth += depthmapArray.SampleCmp(state, sampleCoords, z, int2(-1, 0));
    depth += depthmapArray.SampleCmp(state, sampleCoords, z, int2(0, 1));
    depth += depthmapArray.SampleCmp(state, sampleCoords, z, int2(0, -1));
    depth /= 5.0;
    return depth;
}

// 0.0f - maximum contribution (full shadow), 1.0f - no shadows
float ShadowContribution_Point(float3 worldPos, float4x4 worldToLights[6], float3 lightdir, uint arrayslice)
{
    uint face = selectCubeFace(lightdir);
    float4 lightpos = mul(float4(worldPos, 1.0f), worldToLights[face]);
    float3 projCoords = lightpos.xyz / lightpos.w;
    if (projCoords.z < 0.0f)
        return 1.0f;
    
    return t_pointDepthmapArray.SampleCmp(g_samplerShadow, float4(lightdir, arrayslice), projCoords.z);
}

// 0.0f - maximum contribution (full shadow), 1.0f - no shadows
float ShadowContribution_Spot(float3 worldPos, float4x4 worldToLight, uint arrayslice)
{
    float4 lightpos = mul(float4(worldPos, 1.0f), worldToLight);
    float3 projCoords = lightpos.xyz / lightpos.w;
    if (projCoords.z < 0.0f)
        return 1.0f;
    
    float2 uv = float2(projCoords.x * 0.5f + 0.5f, projCoords.y * -0.5f + 0.5f);
    return t_spotDepthmapArray.SampleCmp(g_samplerShadow, float3(uv, arrayslice), projCoords.z);
}

//// 0.0f - maximum contribution (full shadow), 1.0f - no shadows
//float ShadowContribution_Spot_PCF(float3 worldPos, float4x4 worldToLight, uint arrayslice)
//{
//    float4 lightpos = mul(float4(worldPos, 1.0f), worldToLight);
//    float3 projCoords = lightpos.xyz / lightpos.w;
//    if (projCoords.z < 0.0f)
//        return 1.0f;
    
//    float2 uv = float2(projCoords.x * 0.5f + 0.5f, projCoords.y * -0.5f + 0.5f);
//    return smoothstep(0.33f, 1.0f, ShadowPCF_2DArray(g_samplerShadow, t_spotDepthmapArray, uv, arrayslice, projCoords.z));
//}

// 0.0f - maximum contribution (full shadow), 1.0f - no shadows

float ShadowContribution_Directional_PCF(float3 worldPos, float4x4 worldToLight, uint arrayslice)
{
    float4 lightpos = mul(float4(worldPos, 1.0f), worldToLight);
    float3 projCoords = lightpos.xyz / lightpos.w;
    if (projCoords.z < 0.0f)
        return 1.0f;
    
    float2 uv = float2(projCoords.x * 0.5f + 0.5f, projCoords.y * -0.5f + 0.5f);
    return smoothstep(0.33f, 1.0f, ShadowPCF_2DOrtho(g_samplerShadow, t_dirDepthmapArray, uv, arrayslice, projCoords.z));
}

#endif // __ENGI_LIGHTING_HLSL__