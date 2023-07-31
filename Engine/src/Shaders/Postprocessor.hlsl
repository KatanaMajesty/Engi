#include "Postprocessor.hlsli"

static const float g_xy[] = { -1.0f, -1.0f, 3.0f, -1.0f };
static const float g_uv[] = { 0.0f, 1.0f, 2.0f, 1.0f, 0.0f, -1.0f };

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float2 uv : TEX_COORD;
};

VS_OUTPUT vs_main(uint vid : SV_VertexID)
{
    VS_OUTPUT output;
    output.pos = float4(g_xy[vid + 1], g_xy[vid], 0.0f, 1.0f);
    output.uv = float2(g_uv[(vid * 2)], g_uv[(vid * 2) + 1]);
    return output;
}

float4 ps_main(VS_OUTPUT input) : SV_Target0
{
    float3 color = t_hdrBuffer.Sample(g_activeSampler, input.uv).xyz;
    color *= computeExposure(g_ev100);
// we only use 1 tonemapper, thus we dont need to check for anything else
#ifdef TONEMAPPER_ACES
    color = ACESFitted(color);
#endif
    color = correctGamma(color, g_gamma);
    
    // If FXAA
    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));
    return float4(color, luma);
}