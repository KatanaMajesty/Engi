#ifndef __ENGI_DECAL_HLSL__
#define __ENGI_DECAL_HLSL__

#include "Common.hlsli"
#include "BufferDefines.hlsli"
#include "SamplerDefines.hlsli"

#pragma pack_matrix(row_major)

Texture2D<float4> t_normalMap : register(t1);

Texture2D<uint> t_gbufferObjectID : register(t25);
Texture2D<float> t_depthBuffer : register(t26);
Texture2D<float4> t_gbufferNormals : register(t27);

float3 GetAbsolutePosition(SamplerState samplerState, float2 uv)
{
    float x = uv.x * 2.0 - 1.0;
    float y = -uv.y * 2.0 + 1.0;
    float z = t_depthBuffer.Sample(samplerState, uv);
    float4 clip = float4(x, y, z, 1.0);
    
    float4 view = mul(clip, g_projInv);
    view /= view.w;
    
    float4 world = mul(view, g_viewsInv[0]);
    return world.xyz;
}

uint getObjectID(in float4 pos)
{
    uint objectID = t_gbufferObjectID.Load(int3(pos.xy, 0));
    return objectID;
}

void queryGBufferDimensions(out float width, out float height)
{
    t_gbufferObjectID.GetDimensions(width, height);
}

struct VS_INPUT
{
    float3 meshPosition     : STATIC_MESH_POSITION;
    float3 meshNormal       : STATIC_MESH_NORMAL;
    float4x4 decalToWorld   : DECAL_TO_WORLD;
    float4x4 worldToDecal   : WORLD_TO_DECAL;
    float3 decalAlbedo      : DECAL_ALBEDO;
    float metalness         : DECAL_METALNESS;
    float3 decalEmissive    : DECAL_EMISSIVE;
    float roughness         : DECAL_ROUGHNESS;
    uint parentInstanceID   : PARENT_INSTANCE_ID;
};

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 albedo : ALBEDO;
    float3 emissive : EMISSIVE;
    float metalness : METALNESS;
    float roughness : ROUGHNESS;
    float4x4 worldToDecal : WORLD_TO_DECAL;
    float4x4 decalToWorld : DECAL_TO_WORLD;
    uint parentInstanceID : PARENT_INSTANCE_ID;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
    // REMARK: Actually decalPos may be removed from the shader as we may assert cube's MeshToModel to be identity matrix
    // float4 decalPos = mul(float4(input.meshPosition, 1.0), g_meshToModel);
    float4 decalPos = float4(input.meshPosition, 1.0);
    float4 worldPos = mul(decalPos, input.decalToWorld);
    
    float4x4 viewProj = mul(g_views[0], g_proj);
    float4 pos = mul(worldPos, viewProj);
    
    VS_OUTPUT output;
    output.pos = pos;
    output.albedo = input.decalAlbedo;
    output.emissive = input.decalEmissive;
    output.metalness = input.metalness;
    output.roughness = input.roughness;
    output.worldToDecal = input.worldToDecal;
    output.decalToWorld = input.decalToWorld;
    output.parentInstanceID = input.parentInstanceID;
    return output;
}

struct PS_OUTPUT
{
    float4 albedo : SV_Target0;
    float4 normals : SV_Target1;
    float4 emissive : SV_Target2;
    float4 roughnessMetalness : SV_Target3;
};

cbuffer DecalProperties : register(b4)
{
    float g_normalThreshold; // Already in cosine form
}

PS_OUTPUT ps_main(VS_OUTPUT input)
{
    // Effectively, in our case viewport's width and height will always be equal to gbuffer's width and height
    // TODO: Change this to use real viewport dimensions in future
    float viewportWidth;
    float viewportHeight;
    queryGBufferDimensions(viewportWidth, viewportHeight);
    
    float2 uv = input.pos.xy;
    uv /= float2(viewportWidth, viewportHeight);
    float4 bufferNormals = t_gbufferNormals.Sample(g_samplerLinear, uv);
    
    uint objectID = getObjectID(input.pos);
    if (objectID != input.parentInstanceID)
    {
#if 0 // visualize different instances' pixels with red
        PS_OUTPUT output;
        output.albedo = float4(1.0, 0.0, 0.0, 1.0);
        output.normals = bufferNormals;
        return output;
#else
        discard;
#endif
    }
    
    float3 worldPos = GetAbsolutePosition(g_samplerLinear, uv);
    float3 decalPos = mul(float4(worldPos, 1.0), input.worldToDecal);
    
    // We use a [-0.5, 0.5] bounded unit cube
#if 0 // visualize clipped pixels with blue
    if (any(0.5 - abs(decalPos) < 0.0))
    {
        PS_OUTPUT output;
        output.albedo = float4(0.0, 0.0, 1.0, 1.0);
        output.normals = bufferNormals;
        return output;
    }
#else
    // From Paper "Screen Space Decals in Warhammer 40k: Space Marine"
    clip(0.5 - abs(decalPos));
#endif
    
    float2 decalUv = decalPos.xy + 0.5;
    float4 decalN = t_normalMap.Sample(g_activeSampler, decalUv);
    float3 N = decalN.xyz;
    float alpha = decalN.a;
    
    float3 gN = unpackOctahedron(bufferNormals.zw);
    
    // Side stretching fix from paper "Screen Space Decals in Warhammer 40k: Space Marine"
    float3 Orientation = normalize(-input.decalToWorld[2].xyz);
    clip(dot(gN, Orientation) - g_normalThreshold);
    
    float3 T = normalize(input.decalToWorld[0].xyz);
    T = normalize(T - gN * dot(gN, T));
    
    float3 B = normalize(cross(gN, T));
    float3x3 TBN = constructTBN(input.decalToWorld, T, B, gN);
    
    N = applyTBN(N, TBN);
    N = normalize(lerp(gN, N, alpha));
    
    PS_OUTPUT output;
    output.albedo = float4(input.albedo, alpha);
    output.normals = float4(bufferNormals.xy, packOctahedron(N));
    output.emissive = float4(input.emissive, alpha);
    output.roughnessMetalness = float4(input.roughness, input.metalness, 0.0, alpha);
    return output;
}

#endif // __ENGI_DECAL_HLSL__