#include "Common.hlsli"
#include "IBL.hlsli"
#include "BRDF.hlsli"

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

static const uint NumSamples = 2048;
static const float InvNumSamples = 1.0f / NumSamples;

// References: "Real Shading in Unreal Engine 4" by Brian Karis, Epic Games
// References: https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/spbrdf.hlsl
float2 ps_main(VS_OUTPUT input) : SV_Target0
{
    float NdotV = max(input.uv.x, MATH_EPS); // make sure to avoid divisions by zero and NaNs
    float roughness = 1.0f - input.uv.y;
    
    float3 V;
    V.x = sqrt(1.0f - NdotV * NdotV); // sin
    V.y = 0.0f;
    V.z = NdotV; // cos
    
    float alpha = roughness * roughness;
    
    // We will now pre-integrate Cook-Torrance BRDF for a solid white environment and save results into a 2D LUT.
	// DFG1 & DFG2 are terms of split-sum approximation of the reflectance integral.
	// For derivation see: "Moving Frostbite to Physically Based Rendering 3.0", SIGGRAPH 2014, section 4.9.2.
    // For derivation see: "Real Shading in Unreal Engine 4" by Brian Karis, Epic Games, p.6
    float DFG1 = 0.0f;
    float DFG2 = 0.0f;
    
    for (uint i = 0; i < NumSamples; ++i)
    {
        float2 u = randomHammersley(i, InvNumSamples);
        float3 H = sampleGGX(u.x, u.y, alpha);
        float3 L = reflect(-V, H);
        // float3 L = 2.0f * dot(V, H) * H - V;
        
        float NdotL = saturate(L.z);
        float NdotH = saturate(H.z);
        float VdotH = saturate(dot(V, H));
        
        if (NdotL > MATH_EPS)
        {
            float G = G_SchlickGGX_IBL(alpha, NdotV, NdotL);
            float G_Vis = G * VdotH / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);
            DFG1 += (1.0f - Fc) * G_Vis;
            DFG2 += Fc * G_Vis;
        }
    }
    return float2(DFG1, DFG2) * InvNumSamples;
}