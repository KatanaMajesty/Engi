#ifndef __ENGI_POSTPROCESS_HLSL__
#define __ENGI_POSTPROCESS_HLSL__

#include "SamplerDefines.hlsli"

Texture2D<float4> t_hdrBuffer : register(t0);

#define TONEMAPPER_ACES

cbuffer PostprocessorData : register(b0)
{
    float g_ev100;
    float g_gamma;
};

float3 computeExposure(float ev100)
{
    // with saturation-based speed
    // https://en.wikipedia.org/wiki/Film_speed
    // maxLum = 78 / ( S * q ) * N^2 / t
    //        = 78 / ( S * q ) * 2^ EV_100

    const float lensAttenuation = 0.65;
    const float lensImperfectionExposureScale = 78.0 / (100.0 * lensAttenuation); // 78 / ( S * q )

    float maxLuminance = lensImperfectionExposureScale * pow(2.0, ev100);
    return 1.0 / maxLuminance;
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
    { 0.59719, 0.35458, 0.04823 },
    { 0.07600, 0.90834, 0.01566 },
    { 0.02840, 0.13383, 0.83777 }
};

// COLUMN-MAJOR
// static const float3x3 ACESInputMat =
// {
//     float3(0.59719f, 0.07600f, 0.02840f),
// 	float3(0.35458f, 0.90834f, 0.13383f),
// 	float3(0.04823f, 0.01566f, 0.83777f)
// };

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367 },
    { -0.10208, 1.10813, -0.00605 },
    { -0.00327, -0.07276, 1.07602 }
};

// COLUMN-MAJOR
// static const float3x3 ACESOutputMat =
// {
//     float3(1.60475f, -0.10208, -0.00327f),
// 	float3(-0.53108f, 1.10813, -0.07276f),
// 	float3(-0.07367f, -0.00605, 1.07602f)
// };

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

//https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
float3 ACESFitted(float3 color)
{
    //color *= 1.0 / 6.0;
    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);
    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = saturate(color);
    return color;
}

float3 correctGamma(float3 color, float gamma)
{
    return pow(color, 1.0f / gamma);
}

#endif // __ENGI_POSTPROCESS_HLSL__