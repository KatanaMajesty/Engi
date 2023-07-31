#ifndef __ENGI_BRDF_HLSL__
#define __ENGI_BRDF_HLSL__

#include "Common.hlsli"

// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
//
// alpha param is essentially a roughness^2
float D_GGXTrowbridgeReitz(float alphaSq, float NdotH)
{
    float f = (NdotH * NdotH) * (alphaSq - 1.0f) + 1.0f;
    f = max(f, MATH_EPS);
    float D = alphaSq / (MATH_PI * f * f);
    return D;
}

float G1_SmithGGX(float alpha, float3 NdotX)
{
    float3 G = 1.0f + (alpha * (1.0f - NdotX)) / NdotX;
    G = sqrt(G);
    return G;
}

// Height-correlated Smith G2 for GGX,
// see see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. 
// Journal of Computer Graphics Techniques, 3 (2).
//
// alpha param is essentially a roughness^2
float G_SmithGGX(float alpha, float NdotV, float NdotL)
{
    NdotV *= NdotV;
    NdotL *= NdotL;
    return 2.0f / (G1_SmithGGX(alpha, NdotV) + G1_SmithGGX(alpha, NdotL));
}

float3 calcF0(float3 albedo, float3 metalness)
{
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedo, metalness);
    return F0;
}

// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
//
// XdotL is either dot(H, L) for specular or dot(N, L) for lambertian
float3 F_Schlick(float3 F0, float XdotL)
{
    return F0 + (float3(1.0f, 1.0f, 1.0f) - F0) * pow(1.0f - XdotL, 5.0f);
}

float3 BDRF_CookTorrance_Specular(float3 F0, float roughness, float3 N, float3 V, float3 H, float3 L)
{
    float alpha = roughness * roughness;
    float NdotH = max(dot(N, H), MATH_EPS);
    float NdotV = max(dot(N, V), MATH_EPS);
    float NdotL = max(dot(N, L), MATH_EPS);
    float HdotL = max(dot(H, L), MATH_EPS);
    float3 Ks = F_Schlick(F0, HdotL);
    float torranceNormFactor = 1.0f / (4.0f * NdotV); // removed NdotL
    float3 Fs = Ks * min(1.0f, D_GGXTrowbridgeReitz(alpha * alpha, NdotH) * torranceNormFactor) * G_SmithGGX(alpha, NdotV, NdotL);
    return Fs;
}

float3 BRDF_CookTorrance_Lambertian(float3 F0, float3 albedo, float3 metalness, float3 N, float3 L)
{
    float NdotL = max(dot(N, L), MATH_EPS);
    float lambertianNormFactor = 1.0f / MATH_PI; // NdotL because we removed that from PBR.hlsl
    float3 Kd = (1.0f - F_Schlick(F0, NdotL));
    float3 Fd = Kd * albedo * (1.0f - metalness) * lambertianNormFactor * NdotL;
    return Fd;
}

#endif // __ENGI_BRDF_HLSL__