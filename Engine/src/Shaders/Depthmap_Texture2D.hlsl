#include "BufferDefines.hlsli"
#include "LayoutDefines.hlsli"

float4 vs_main(VS_INPUT input) : SV_Position
{
    float4x4 meshToWorld = mul(g_meshToModel, input.modelToWorld);
    
    float4 worldPos = mul(float4(input.meshPosition, 1.0), meshToWorld);
    float4x4 viewProj = mul(g_views[0], g_proj);
    float4 pos = mul(worldPos, viewProj);
    return pos;
}