#ifndef __ENGI_EMISSIVE_HLSL__
#define __ENGI_EMISSIVE_HLSL__

#include "Common.hlsli"
#include "BufferDefines.hlsli"
#include "SamplerDefines.hlsli"

Texture2D<float3> t_gbufferAlbedo : register(t21);
Texture2D<float4> t_gbufferNormals : register(t22);
Texture2D<float3> t_gbufferEmission : register(t23);
Texture2D<float2> t_gbufferRoughnessMetalness : register(t24);
Texture2D<uint> t_gbufferObjectID : register(t25);

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float2 uv : TEX_COORD;
};

VS_OUTPUT vs_main(uint vid : SV_VertexID)
{
    VS_OUTPUT output;
    output.pos = float4(g_xy[vid + 1], g_xy[vid], 0.0f, 1.0f);
    output.uv = float2(g_uv[(vid * 2)], g_uv[(vid * 2) + 1]);
    return output;
}

float4 ps_main(VS_OUTPUT input) : SV_Target0
{
    SamplerState ss = g_samplerLinear;
    float3 emission = t_gbufferEmission.Sample(ss, input.uv);
    
    return float4(emission, 1.0f);
}

#endif // __ENGI_EMISSIVE_HLSL__