#include "Common.hlsli"
#include "TextureDefines.hlsli"
#include "SamplerDefines.hlsli"

uint querySpecularNumMips()
{
    uint width, height, mips;
    t_IBLSpecular.GetDimensions(0, width, height, mips);
    return mips;
}

float3 addDiffuseReflection(in float3 albedo, in float3 N, in float3 metalness)
{
    return albedo * (1.0f - metalness) * t_IBLDiffuse.SampleLevel(g_samplerLinear, N, 0.0f).rgb;
}

float3 addSpecularReflection(float roughness, float NdotV, float3 F0, float3 Reflection)
{
    // roughnessLinear is initial roughness value set by artist, not rough^2 or rough^4
    float2 reflectanceLUT = t_LUT.Sample(g_samplerLinear, float2(NdotV, roughness)).rg;
    float3 reflectance = reflectanceLUT.r * F0 + reflectanceLUT.g;
    float3 specularReflection = reflectance * t_IBLSpecular.SampleLevel(g_activeSampler, Reflection, roughness * querySpecularNumMips()).rgb;
    return specularReflection;
}

// random2D from random1D
// Sample i-th point from Hammersley point set of NumSamples points total.
float2 randomHammersley(uint i, float invNumSamples)
{
    return float2(i * invNumSamples, radicalInverse_VdC(i));
}

// Importance sample GGX normal distribution function for a fixed roughness value.
// This returns normalized half-vector between Li & Lo.
// For derivation see: http://blog.tobias-franke.eu/2014/03/30/notes_on_importance_sampling.html
// alpha is basically roughness^2
float3 sampleGGX(float u1, float u2, float alpha)
{
    float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha * alpha - 1.0) * u2));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta); // Trig. identity
    float phi = 2.0f * MATH_PI * u1;

	// Convert to Cartesian upon return.
    return float3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

// GGX importance sampling, returns microsurface normal (half-vector)
// rough4 is initial roughness value in power of 4
float3 sampleGGX(out float NdotH, uint i, float invNumSamples, float alpha, float3 N)
{
    float2 u = randomHammersley(i, invNumSamples);
    float3 H = sampleGGX(u.x, u.y, alpha);
    NdotH = saturate(H.z);
    float3x3 r = basisFromDir(N);
    return mul(H, r);
}

// Single term for separable Schlick-GGX below.
float G_SchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method (IBL version).
// Reference: "Real Shading in Unreal Engine 4" by Brian Karis, Epic Games, p.3 "Specular G"
float G_SchlickGGX_IBL(float alpha, float NdotV, float NdotL)
{
    // Epic suggests using this roughness remapping for IBL lighting.
    // With this modification, the Schlick model exactly matches Smith for alpha = 1 
    // and is a fairly close approximation over the range[0, 1]
    float k = alpha / 2.0f;
    return G_SchlickG1(NdotV, k) * G_SchlickG1(NdotL, k);
}

float getSamplingMipLevel(float cubeSize, float importance)
{
    float vt = 3.0f * cubeSize * cubeSize; // estimated amount of visible texels;
    float mip = 0.5f * log2(importance * vt) + 1.0f;
    return mip; // Mip value is actually log4 of the log argument, that’s what 0.5 factor does
}