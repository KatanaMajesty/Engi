#include "SamplerDefines.hlsli"
#include "BufferDefines.hlsli"
#include "LayoutDefines.hlsli"
#include "TextureDefines.hlsli"

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float2 texUvs : MESH_TEXUV;
    float3 color : INSTANCE_COLOR;
};

VS_OUTPUT vs_main(VS_INPUT input)
{    
    float4x4 viewProj = mul(g_views[0], g_proj);
    float4 pos = mul(mul(float4(input.meshPosition, 1.0f), g_meshToModel), mul(input.modelToWorld, viewProj));
    VS_OUTPUT output;
    output.pos = pos;
    output.texUvs = input.meshTexCoords;
    output.color = input.color;
    return output;
}

float4 ps_main(VS_OUTPUT input) : SV_Target0
{
    float4 color = float4(input.color, 1.0f);
    if (g_useAlbedoTexture)
        color = t_albedoTexture.Sample(g_activeSampler, input.texUvs);
    
    return color;
}