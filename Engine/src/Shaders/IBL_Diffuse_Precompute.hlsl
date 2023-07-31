#include "Common.hlsli"
#include "SamplerDefines.hlsli"
#include "BRDF.hlsli"

TextureCube<float4> t_skybox : register(t15);

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float2 uv : TEX_COORD;
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

static const uint NumSamples = 2048;

PS_OUTPUT ps_main(VS_OUTPUT input)
{   
    float3 irradiance[6] =
    {
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f),
        float3(0.0f, 0.0f, 0.0f)
    };
    
    for (uint slice = 0; slice < 6; ++slice)
    {
        float3 N = getSamplingVector(input.uv, slice);
        float3x3 toN = basisFromDir(N);
        for (uint i = 0; i < NumSamples; ++i)
        {
            float NdotV;
            float3 tangentSample = fibonacciHemispherePoint(NdotV, i, NumSamples);
            float3 Nv = mul(tangentSample, toN);
            float3 f = (1.0f - F_Schlick(0.04f, NdotV));
            irradiance[slice] += t_skybox.SampleLevel(g_samplerAnisotropic, Nv, 0.0f).rgb * NdotV / MATH_PI * f;
        }
        irradiance[slice] *= 2.0f * MATH_PI / NumSamples;
    }
    
    PS_OUTPUT output;
    output.o0 = float4(irradiance[0], 1.0f);
    output.o1 = float4(irradiance[1], 1.0f);
    output.o2 = float4(irradiance[2], 1.0f);
    output.o3 = float4(irradiance[3], 1.0f);
    output.o4 = float4(irradiance[4], 1.0f);
    output.o5 = float4(irradiance[5], 1.0f);
    return output;
}