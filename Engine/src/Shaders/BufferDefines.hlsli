#ifndef __ENGI_BUFFERS_HLSL__
#define __ENGI_BUFFERS_HLSL__

#pragma pack_matrix(row_major)

cbuffer SceneCB : register(b0)
{
    float g_time;
    float g_timestep;
    float g_dissolutionTime;
    uint g_numDirLights;
    uint g_numSpotLights;
    uint g_numPointLights;
    uint g_useIBL;
    uint g_useShadowmapping;
};

cbuffer MeshData : register(b1)
{
    float4x4 g_meshToModel;
    float4x4 g_modelToMesh;
    uint g_meshHasTexCoords;
};

cbuffer MaterialData : register(b2)
{
    uint g_useAlbedoTexture;
    uint g_useNormalMap;
    uint g_useMetalnessMap;
    uint g_useRoughnessMap;
    float g_roughness;
    float g_metallic;
};

cbuffer ViewData : register(b3)
{
    float4x4 g_views[6];
    float4x4 g_viewsInv[6];
    float4x4 g_proj;
    float4x4 g_projInv;
}

#endif // __ENGI_BUFFERS_HLSL__