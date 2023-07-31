
#ifndef __cplusplus
	#pragma pack_matrix(row_major)
#else
	#pragma once
	#include "Shaders/HLSL.h"
#endif

struct ENGI_DirectionalLight
{
	float3 ambient;
	float3 direction;
    float radius;
    uint depthArraySlice;
    float4x4 worldToLight;
};

struct ENGI_PointLight
{
    float3 position;
	float3 color;
	float intensity;
    float radius;
    float4x4 worldToLight[6];
    uint depthArraySlice;
};

struct ENGI_SpotLight
{
    float4x4 worldToLocal; // For cookie
	float4x4 worldToLight;
    uint depthArraySlice;
    float3 direction;
    float3 position;
	float intensity;
	float3 color;
	float cutoff;
	uint useSpotlightMask;
	float outerCutoff;
    float radius;
};