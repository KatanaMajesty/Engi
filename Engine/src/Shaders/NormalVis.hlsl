#include "BufferDefines.hlsli"
#include "LayoutDefines.hlsli"

struct VS_OUTPUT
{
    float3 worldPos : WORLD_POSITION;
    float4x4 viewProj : WORLD_TO_CLIP;
};

VS_OUTPUT vs_main(VS_INPUT input)
{    
    float4 modelPos = mul(float4(input.meshPosition, 1.0), g_meshToModel);
    
    VS_OUTPUT output;
    output.worldPos = mul(modelPos, input.modelToWorld).xyz;
    output.viewProj = mul(g_views[0], g_proj);
    return output;
}

struct GS_OUTPUT
{
    float4 pos : SV_Position;
};

[maxvertexcount(2)]
void gs_main(
    triangle VS_OUTPUT input[3],
    inout LineStream<GS_OUTPUT> output
)
{
    float3 edge1 = input[1].worldPos - input[0].worldPos;
    float3 edge2 = input[2].worldPos - input[0].worldPos;
    float3 worldNormal = normalize(cross(edge1, edge2));
    float3 normalLen = clamp((length(edge1) + length(edge2)) / 2.0, 0.01, 0.3);
    float3 pos1 = (input[0].worldPos + input[1].worldPos + input[2].worldPos) / 3.0;
    float3 pos2 = pos1 + worldNormal * normalLen;
    
    GS_OUTPUT o1;
    o1.pos = mul(float4(pos1, 1.0), input[0].viewProj);
    output.Append(o1);
    
    GS_OUTPUT o2;
    o2.pos = mul(float4(pos2, 1.0), input[0].viewProj);
    output.Append(o2);
}

float4 ps_main(GS_OUTPUT input) : SV_Target0
{
    return float4(0.0f, 0.8f, 0.2f, 1.0f);
}