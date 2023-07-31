#include "LayoutDefines.hlsli"
#include "BufferDefines.hlsli"
#include "Common.hlsli"
#include "SamplerDefines.hlsli"
#include "TextureDefines.hlsli"

struct VS_OUTPUT
{    
    float4 pos : SV_Position;
    float3 worldPos : WORLD_POS;
    float3 worldNormal : WORLD_NORMAL;
    float3 cameraPos : CAMERA_POS;
    float3 emission : MESH_EMISSION;
    uint instanceID : INSTANCE_ID;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
    float3 worldNormal = convertToOrthogonalBasis(input.modelToWorld, convertToOrthogonalBasis(g_meshToModel, input.meshNormal));
    float3 modelPos = mul(float4(input.meshPosition, 1.0f), g_meshToModel).xyz;
    float3 worldPos = mul(float4(modelPos, 1.0f), input.modelToWorld).xyz;
    
    float4x4 viewProj = mul(g_views[0], g_proj);
    float4 pos = mul(float4(worldPos, 1.0f), viewProj);
    
    VS_OUTPUT output;
    output.pos = pos;
    output.worldPos = worldPos;
    output.worldNormal = worldNormal;
    output.cameraPos = g_viewsInv[0][3].xyz;
    output.emission = input.emission * input.emissionPow;
    output.instanceID = input.instanceID;
    return output;
}

struct PS_OUTPUT
{
    float3 emission : SV_Target2;
    uint objectID : SV_Target4;
};

PS_OUTPUT ps_main(VS_OUTPUT input)
{
    float3 N = normalize(input.worldNormal);
    float3 cameraDir = normalize(input.cameraPos - input.worldPos);

    float3 normedEmission = input.emission / max(input.emission.x, max(input.emission.y, max(input.emission.z, 1.0f)));
    float NoV = dot(cameraDir, N);
    float3 emissionResult = lerp(normedEmission * 0.1f, input.emission, max(0.0f, NoV));
    
    PS_OUTPUT output;
    output.emission = emissionResult;
    output.objectID = input.instanceID;
    return output;
}