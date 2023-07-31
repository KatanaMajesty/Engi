#ifndef __ENGI_TEXTURES_HLSL__
#define __ENGI_TEXTURES_HLSL__

#include "Common.hlsli"

Texture2D<float4> t_albedoTexture : register(t0);
Texture2D<float4> t_normalMap : register(t1);
Texture2D<float4> t_metalnessMap : register(t2);
Texture2D<float4> t_roughnessMap : register(t3);
TextureCube<float4> t_IBLDiffuse : register(t5);
TextureCube<float4> t_IBLSpecular : register(t6);
Texture2D<float2> t_LUT : register(t7);

float4 processAlbedoMap(SamplerState s, float2 texCoords)
{
    return t_albedoTexture.Sample(s, texCoords);
}

float3 processNormalMap(SamplerState s, float2 texCoords, float3x3 TBN)
{
    float3 normal = t_normalMap.Sample(s, texCoords).xyz;
    normal = applyTBN(normal, TBN);
    return normal;
}

float4 processMetalnessMap(SamplerState s, float2 texCoords)
{
    return t_metalnessMap.Sample(s, texCoords);
}

float4 processRoughnessMap(SamplerState s, float2 texCoords)
{
    return t_roughnessMap.Sample(s, texCoords);
}

#endif // __ENGI_TEXTURES_HLSL__