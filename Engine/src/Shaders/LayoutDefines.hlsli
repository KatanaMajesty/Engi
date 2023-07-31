#ifndef __ENGI_LAYOUTS_HLSL__
#define __ENGI_LAYOUTS_HLSL__

#pragma pack_matrix(row_major)

struct VS_INPUT
{
    float3 meshPosition : STATIC_MESH_POSITION;
    float3 meshNormal : STATIC_MESH_NORMAL;
    float2 meshTexCoords : STATIC_MESH_TEXUV;
    float3 meshTangent : STATIC_MESH_TANGENT;
    float3 meshBitangent : STATIC_MESH_BITANGENT;
    float4x4 modelToWorld : MODEL_TO_WORLD;
    float4x4 worldToModel : WORLD_TO_MODEL;
    float3 color : INSTANCE_COLOR;
    float time : INSTANCE_TIME;
    float timestep : INSTANCE_TIMESTEP;
    float3 emission : INSTANCE_EMISSION;
    float emissionPow : INSTANCE_EMISSION_POW;
    float3 sphereOrigin : INSTANCE_SPHERE_ORIGIN;
    float sphereRadiusMax : INSTANCE_SPHERE_RADIUS_MAX;
    uint instanceID : INSTANCE_ID;
};

#endif // __ENGI_LAYOUTS_HLSL__