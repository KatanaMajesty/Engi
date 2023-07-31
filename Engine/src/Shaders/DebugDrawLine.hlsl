#include "BufferDefines.hlsli"

struct VS_INPUT
{
    float3 worldPos : WORLD_POS;
};

float4 vs_main(VS_INPUT input) : SV_Position
{
    float4x4 viewProj = mul(g_views[0], g_proj);
    return mul(float4(input.worldPos, 1.0f), viewProj);
}

// Just for testing constant buffer
float4 ps_main(float4 pos : SV_Position) : SV_Target0
{
    return float4(0.0f, 0.9f, 0.2f, 1.0f);
}