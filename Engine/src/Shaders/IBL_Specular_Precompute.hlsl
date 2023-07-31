#include "Common.hlsli"
#include "SamplerDefines.hlsli"
#include "BRDF.hlsli"
#include "IBL.hlsli"

TextureCube<float4> t_skybox : register(t15);

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float2 uv : TEX_COORD;
};

cbuffer SpecularPrecomputeCB : register(b8)
{
    uint g_cubeSize;
    uint g_mipSlice;
    uint g_mipLevels;
};

VS_OUTPUT vs_main(uint vid : SV_VertexID)
{
    VS_OUTPUT output;
    output.pos = fullscreenTrianglePos(vid);
    output.uv = fullscreenTriangleUV(vid);
    return output;
}

struct PS_OUTPUT
{
    float4 o0 : SV_Target0; // +x
    float4 o1 : SV_Target1; // -x
    float4 o2 : SV_Target2; // +y
    float4 o3 : SV_Target3; // -y
    float4 o4 : SV_Target4; // +z
    float4 o5 : SV_Target5; // -z
};

// Select vector based on cubemap face index.
// Taken from https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/irmap.hlsl
float3 getSamplingVector(float2 uv, uint slice)
{
    float2 st = 2.0f * float2(uv.x, 1.0f - uv.y) - 1.0f;
    float3 ret;
    switch (slice)
    {
        case 0: ret = float3(1.0f, st.y, -st.x); break;
        case 1: ret = float3(-1.0f, st.y, st.x); break;
        case 2: ret = float3(st.x, 1.0f, -st.y); break;
        case 3: ret = float3(st.x, -1.0f, st.y); break;
        case 4: ret = float3(st.x, st.y, 1.0f); break;
        case 5: ret = float3(-st.x, st.y, -1.0f); break;
    }
    return normalize(ret);
}

static const uint NumSamples = 512;
static const float InvNumSamples = 1.0f / NumSamples;

PS_OUTPUT ps_main(VS_OUTPUT input)
{  
#if 1 // avoid precomputing first mipslice
    if (g_mipSlice == 0)
    {
        PS_OUTPUT output;
        output.o0 = t_skybox.SampleLevel(g_samplerLinear, getSamplingVector(input.uv, 0), 0.0f);
        output.o1 = t_skybox.SampleLevel(g_samplerLinear, getSamplingVector(input.uv, 1), 0.0f);
        output.o2 = t_skybox.SampleLevel(g_samplerLinear, getSamplingVector(input.uv, 2), 0.0f);
        output.o3 = t_skybox.SampleLevel(g_samplerLinear, getSamplingVector(input.uv, 3), 0.0f);
        output.o4 = t_skybox.SampleLevel(g_samplerLinear, getSamplingVector(input.uv, 4), 0.0f);
        output.o5 = t_skybox.SampleLevel(g_samplerLinear, getSamplingVector(input.uv, 5), 0.0f);
        return output;
    }
#endif
    
    float3 prefilteredColor[6] =
    {
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f)
    };
    
    float roughness = float(g_mipSlice + 1) / float(g_mipLevels);
    float alpha = roughness * roughness;
    
    for (uint slice = 0; slice < 6; ++slice)
    {
        // Approximation: Assume zero viewing angle (isotropic reflections).
        float3 N = getSamplingVector(input.uv, slice);
        float3 V = N;
        float weight = 0.0f;
        for (uint i = 0; i < NumSamples; ++i)
        {
            float NdotH;
            float3 H = normalize(sampleGGX(NdotH, i, InvNumSamples, alpha, N));
            float3 L = reflect(-V, H); // Reflect view direction around halfvector, getting incident lighting
            
            NdotH = saturate(NdotH);
            float NdotL = saturate(dot(N, L));
            
            if (NdotL > MATH_EPS)
            {
                float D = D_GGXTrowbridgeReitz(alpha, NdotH);
                float importance = 4.0f / (2.0f * MATH_PI * D) * InvNumSamples;
                float mip = getSamplingMipLevel(g_cubeSize, importance);
                
                prefilteredColor[slice] += t_skybox.SampleLevel(g_samplerLinear, L, mip).rgb * NdotL;
                weight += NdotL;
            }
        }

#if 1 // use WEIGHTED approx
        prefilteredColor[slice] /= weight;
#else
        prefilteredColor[slice] *= InvNumSamples;
#endif
    }
    
    PS_OUTPUT output;
    output.o0 = float4(prefilteredColor[0], 1.0f);
    output.o1 = float4(prefilteredColor[1], 1.0f);
    output.o2 = float4(prefilteredColor[2], 1.0f);
    output.o3 = float4(prefilteredColor[3], 1.0f);
    output.o4 = float4(prefilteredColor[4], 1.0f);
    output.o5 = float4(prefilteredColor[5], 1.0f);
    return output;
}