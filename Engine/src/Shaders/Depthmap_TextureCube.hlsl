#include "BufferDefines.hlsli"
#include "LayoutDefines.hlsli"

struct VS_OUTPUT
{
    float4 worldPos : WORLD_POS;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
    float4 modelPos = mul(float4(input.meshPosition, 1.0f), g_meshToModel);
    float4 worldPos = mul(modelPos, input.modelToWorld);
    VS_OUTPUT output;
    output.worldPos = worldPos;
    return output;
}

struct GS_OUTPUT
{
    float4 pos : SV_Position;
    uint arrayslice : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void gs_main(triangle VS_OUTPUT input[3], inout TriangleStream<GS_OUTPUT> output)
{
    for (uint arrayslice = 0; arrayslice < 6; ++arrayslice)
    {
        for (uint i = 0; i < 3; ++i)
        {
            float4x4 viewProj = mul(g_views[arrayslice], g_proj);
            float4 pos = mul(input[i].worldPos, viewProj);
            
            GS_OUTPUT o;
            o.pos = pos;
            o.arrayslice = arrayslice;
            output.Append(o);
        }
        
        output.RestartStrip();
    }
}