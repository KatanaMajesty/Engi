#include "BufferDefines.hlsli"
#include "LayoutDefines.hlsli"
#include "SamplerDefines.hlsli"
#include "TextureDefines.hlsli"
#include "Common.hlsli"

struct VS_OUTPUT
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD;
    float3 worldNormal : WORLD_NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BITANGENT;
    float3x3 TBN : TBN_MAT;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
    float3 worldNormal = convertToOrthogonalBasis(input.modelToWorld, convertToOrthogonalBasis(g_meshToModel, input.meshNormal));
    float3 modelPos = mul(float4(input.meshPosition, 1.0f), g_meshToModel).xyz;
    
    float4x4 viewProj = mul(g_views[0], g_proj);
    VS_OUTPUT output;
    output.position = mul(float4(modelPos, 1.0f), mul(input.modelToWorld, viewProj));
    output.worldNormal = worldNormal;
    output.texCoords = input.meshTexCoords;
    if (g_useNormalMap)
    {
        float3x3 TBN = constructTBN(input.modelToWorld, input.meshTangent, input.meshBitangent, worldNormal);
        output.TBN = TBN;
    }
    return output;
}

// Just for testing constant buffer
float4 ps_main(VS_OUTPUT input) : SV_Target0
{
    float3 normal = input.worldNormal;
    if (g_useNormalMap)
        normal = processNormalMap(g_activeSampler, input.texCoords, input.TBN);

    float3 n = normal / 2.0f + 0.5;
    float4 output = float4(n, 1.0f);
    return output;
}