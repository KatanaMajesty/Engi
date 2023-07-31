#include "LayoutDefines.hlsli"
#include "BufferDefines.hlsli"
#include "Common.hlsli"
#include "SamplerDefines.hlsli"
#include "TextureDefines.hlsli"

Texture2D<float> t_dissolutionTexture : register(t20);

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 worldNormal : MESH_NORMAL;
    float2 texCoords : TEXCOORD;
    float3 color : MESH_COLOR;
    float3 emission : MESH_EMISSION;
    float3x3 TBN : TBN_MAT;
    float time : TIME;
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
    output.worldNormal = worldNormal;
    output.texCoords = input.meshTexCoords;
    
    output.color = input.color;
    output.emission = input.emission * input.emissionPow;
    
    output.TBN = constructTBN(input.modelToWorld, input.meshTangent, input.meshBitangent, worldNormal);
    
    output.time = input.time;
    
    output.instanceID = input.instanceID;
    return output;
}

struct PS_OUTPUT
{
    float4 albedo : SV_Target0;
    float4 normals : SV_Target1;
    float3 emission : SV_Target2;
    float2 roughnessMetalness : SV_Target3;
    uint objectID : SV_Target4;
};

PS_OUTPUT ps_main(VS_OUTPUT input)
{
    float3 albedo = input.color;
    if (g_useAlbedoTexture)
    {
        albedo = processAlbedoMap(g_activeSampler, input.texCoords);
    }
    
    float3 N = normalize(input.worldNormal);
    float3 Ng = N;
    if (g_useNormalMap)
    {
        Ng = processNormalMap(g_activeSampler, input.texCoords, input.TBN);
    }
    
    float roughness = g_roughness;
    if (g_useRoughnessMap)
    {
        roughness = processRoughnessMap(g_activeSampler, input.texCoords).r;
    }
    
    float metalness = g_metallic;
    if (g_useMetalnessMap)
    {
        metalness = processMetalnessMap(g_activeSampler, input.texCoords).r;
    }
    
    PS_OUTPUT output; 
    output.emission = 0.0;
   
    if (g_meshHasTexCoords)
    {
        float dissolution = t_dissolutionTexture.Sample(g_activeSampler, input.texCoords).r;
        float dA = input.time / g_dissolutionTime;
        
        if (dissolution > dA)
            discard;
        
        // Highlight color for dissolved pixels
        float3 dissolutionEmission = input.emission;
        
        // Attenuation factor for the highlight over time
        float eps = 0.05f;
        float dAEps = dA - eps;
        float invEps = 1.0f / eps;
        float attenuationFactor = saturate((dissolution - dAEps) * invEps);
        
        // Interpolate between the 0 and the highlight color, which is bright white
        float3 interpolatedEmission = lerp(0.0, dissolutionEmission, attenuationFactor);
        
        // Apply the highlight color
        output.emission += interpolatedEmission;
    }
    
    output.albedo = float4(albedo, 1.0);
    output.normals = float4(packOctahedron(N), packOctahedron(Ng));
    output.roughnessMetalness = float2(roughness, metalness);
    output.objectID = input.instanceID;
    
    return output;
}