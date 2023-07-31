#include "FractalNoise.hlsli"
#include "BufferDefines.hlsli"
#include "LayoutDefines.hlsli"

struct VS_OUTPUT
{
    float3 modelPosition : MODEL_POSITION;
    float3 modelNormal : MODEL_NORMAL;
    float3 cameraPos : CAMERA_POS;
    float4x4 modelToWorld : INSTANCE_TO_WORLD;
    float3 emission : MESH_EMISSION;
    uint instanceID : INSTANCE_ID;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
    float3 modelAxisX = normalize(g_meshToModel[0].xyz);
    float3 modelAxisY = normalize(g_meshToModel[1].xyz);
    float3 modelAxisZ = normalize(g_meshToModel[2].xyz);
    float3 modelNormal = input.meshNormal.x * modelAxisX + input.meshNormal.y * modelAxisY + input.meshNormal.z * modelAxisZ;
    float3 modelPos = mul(float4(input.meshPosition, 1.0), g_meshToModel).xyz;
    
    VS_OUTPUT output;
    output.modelPosition = modelPos;
    output.modelNormal = modelNormal;
    output.cameraPos = g_viewsInv[0][3].xyz;
    output.modelToWorld = float4x4(input.modelToWorld);
    output.emission = input.emission * input.emissionPow;
    output.instanceID = input.instanceID;
    return output;
}

struct HS_INPUT
{
    uint pointId : SV_OutputControlPointID;
    uint patchId : SV_PrimitiveID;
};

[outputcontrolpoints(3)] // doesn’t have to match InputPatch argument size (3)
[domain("tri")]
[outputtopology("triangle_cw")]
[partitioning("integer")]
[patchconstantfunc("hs_patchConstantFunc")]
VS_OUTPUT hs_main(InputPatch<VS_OUTPUT, 3> patch, HS_INPUT input)
{
    return patch[input.pointId]; // input size matches the number specified control points in the set PATCHLIST Topology
}

struct PATCH_OUT
{
    // 3 outer factors and 1 inner factor specifically for “tri” domain
    float edge[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

PATCH_OUT hs_patchConstantFunc(InputPatch<VS_OUTPUT, 3> input,
    uint patchId : SV_PrimitiveID)
{
    // pos relative to the camera
    float3 viewPos0 = mul(float4(input[0].modelPosition, 1.0), mul(input[0].modelToWorld, g_views[0]));
    float3 viewPos1 = mul(float4(input[1].modelPosition, 1.0), mul(input[1].modelToWorld, g_views[0]));
    float3 viewPos2 = mul(float4(input[2].modelPosition, 1.0), mul(input[2].modelToWorld, g_views[0]));
    float3 edge1 = viewPos1 - viewPos0;
    float3 edge2 = viewPos2 - viewPos0;
    float3 edge3 = viewPos1 - viewPos2;
    
    PATCH_OUT output;
    output.edge[0] = clamp(uint(16.0 * length(edge1)) / length(viewPos0), 1.0, 16.0);
    output.edge[1] = clamp(uint(16.0 * length(edge2)) / length(viewPos1), 1.0, 16.0);
    output.edge[2] = clamp(uint(16.0 * length(edge3)) / length(viewPos2), 1.0, 16.0);
    output.inside = (output.edge[0] + output.edge[1] + output.edge[2]) / 3.0;
    return output;
}

[domain("tri")]
VS_OUTPUT ds_main(PATCH_OUT control, float3 uvw : SV_DomainLocation,
const OutputPatch<VS_OUTPUT, 3> input)
{
    VS_OUTPUT result;
    // manual barycentric interpolation
    result.modelPosition = uvw.x * input[0].modelPosition + uvw.y * input[1].modelPosition + uvw.z * input[2].modelPosition;
    result.modelNormal = uvw.x * input[0].modelNormal + uvw.y * input[1].modelNormal + uvw.z * input[2].modelNormal;
    result.cameraPos = input[0].cameraPos;
    result.modelToWorld = input[0].modelToWorld;
    result.emission = input[0].emission;
    result.instanceID = input[0].instanceID;
    return result;
}

struct GS_OUTPUT
{
    float4 position : SV_Position;
    float3 modelPosition : MODEL_POSITION;
    float3 modelNormal : MODEL_NORMAL;
    float3 cameraPos : CAMERA_POS;
    float3 emission : MESH_EMISSION;
    uint instanceID : INSTANCE_ID;
};

float3 vertexDistortion(float3 pos, float3 normal)
{
    float3 offset = 0.0;
    offset += normal * 0.025 * wave(pos, BLUE_WAVE_INTERVAL, BLUE_WAVE_SPEED, BLUE_WAVE_THICKNESS, true, g_time);
    offset += normal * 0.05 * wave(pos, RED_WAVE_INTERVAL, RED_WAVE_SPEED, RED_WAVE_THICKNESS, true, g_time);
    return offset;
}

[maxvertexcount(3)]
void gs_main(
	triangle VS_OUTPUT input[3],
	inout TriangleStream<GS_OUTPUT> output
)
{
    float3 edge1 = input[1].modelPosition - input[0].modelPosition;
    float3 edge2 = input[2].modelPosition - input[0].modelPosition;
    float3 triangleNormal = normalize(cross(edge1, edge2));
    for (uint i = 0; i < 3; i++)
    {
        float3 modelPos = input[i].modelPosition + vertexDistortion(input[i].modelPosition, triangleNormal);
        float4x4 viewProj = mul(g_views[0], g_proj);
        float4 position = mul(mul(float4(modelPos, 1.0), input[i].modelToWorld), viewProj);
        GS_OUTPUT element;
        element.position = position;
        element.modelNormal = input[i].modelNormal;
        element.modelPosition = modelPos;
        element.cameraPos = input[i].cameraPos;
        element.emission = input[i].emission;
        element.instanceID = input[i].instanceID;
        output.Append(element);
    }
}

// called in pixel shader
float3 colorDistortion(float3 color, float3 cameraPos, float3 pos, float3 normal)
{
    float blueWave = wave(pos, BLUE_WAVE_INTERVAL, BLUE_WAVE_SPEED, BLUE_WAVE_THICKNESS, true, g_time);
    float redWave = wave(pos, RED_WAVE_INTERVAL, RED_WAVE_SPEED, RED_WAVE_THICKNESS, true, g_time);

    float3 toCamera = normalize(cameraPos - pos);
    float contourGlow = pow(1.0 - abs(dot(normal, toCamera)), 2);

    // when contourInterference is 0, ripple effect contourWave is added to contourGlow, otherwise contourWave is 1
    float contourWave = wave(pos, 0.1, 0.1, 0.05, false, g_time);
    float contourInterference = periodIntensity(g_time, 4, 1);
    contourWave = lerp(contourWave, 1.0, contourInterference);
    // when contourWave is 0.0, contourGlow becomes darker, otherwise contourGlow color is plain, without ripple
    contourGlow = lerp(contourGlow / 10, contourGlow, contourWave);

    color = color * min(1, contourGlow + blueWave * 0.5);
    float colorNoise = sqrt(noise4d(float4(pos, frac(g_time)) * 100, 1));
    color *= lerp(colorNoise, 1.0, contourInterference);
    
    color = lerp(color, float3(1.0, 0.0, 0.0), redWave * 0.25);
    return color;
}

struct PS_OUTPUT
{
    float3 emission : SV_Target2;
    uint objectID : SV_Target4;
};

PS_OUTPUT ps_main(GS_OUTPUT input)
{
    float3 emission = colorDistortion(input.emission, input.cameraPos, input.modelPosition, input.modelNormal);
    
    PS_OUTPUT output;
    output.emission = emission;
    output.objectID = input.instanceID;
    return output;
}