#include "BufferDefines.hlsli"
#include "SamplerDefines.hlsli"

TextureCube t_skybox : register(t0);

static const float g_xy[] = { -1.0f, -1.0f, 3.0f, -1.0f };

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 texCoords : SKYBOX_TEX_UV;
};

VS_OUTPUT vs_main(uint vid : SV_VertexID)
{
    VS_OUTPUT output;
    output.pos = float4(g_xy[vid + 1], g_xy[vid], 0.0f, 1.0f);
    float4x4 viewProjInv = mul(g_projInv, g_viewsInv[0]);
    float4 wp = mul(output.pos, viewProjInv); 
    wp /= wp.w;
    float3 camerapos = g_viewsInv[0][3].xyz;
    output.texCoords = wp.xyz - camerapos;
    return output;
}

#include "LightCasters.hlsli"

RWStructuredBuffer<ENGI_DirectionalLight> u_dirLightBuffer : register(u1);

float4 ps_main(VS_OUTPUT input) : SV_Target0
{
    ENGI_DirectionalLight light;
    light.ambient = 0.0;
    light.direction = 0.0;
    light.radius = 0.0;
    light.depthArraySlice = 0;
    light.worldToLight = 0.0;
    u_dirLightBuffer[0] = light;
    return t_skybox.Sample(g_activeSampler, input.texCoords);
}