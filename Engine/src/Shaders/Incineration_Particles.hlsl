
#include "BufferDefines.hlsli"
#include "SamplerDefines.hlsli"

struct VS_INPUT
{
    uint vertexID : SV_VertexID;
    uint instanceID : SV_InstanceID;
};

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 emissive : EMISSIVE;
};

static const float2 g_billboardVertices[] =
{
    float2(-1.0, -1.0),
    float2(-1.0, 1.0),
    float2(1.0, 1.0),
    float2(1.0, -1.0),
};

static const float2 g_billboardTexCoords[] =
{
    float2(0.0, 1.0),
    float2(0.0, 0.0),
    float2(1.0, 0.0),
    float2(1.0, 1.0),
};

struct IncinerationParticle
{
    float3 worldPos;
    float3 velocity;
    float3 emissive;
    float lifetime;
    uint parentInstanceID;
};

Buffer<uint> b_incinerationRangeBuffer : register(t0);
StructuredBuffer<IncinerationParticle> b_incinerationParticleBuffer : register(t1);

#define MAX_PARTICLES 2048
#define BUFFER_INDEX_NUM_PARTICLES 0
#define BUFFER_INDEX_OFFSET 1
#define BUFFER_INDEX_NUM_EXPIRED 2

VS_OUTPUT vs_main(VS_INPUT input)
{
    static const float BillboardSize = 0.01;
    float3 cameraRight = g_viewsInv[0][0].xyz;
    float3 cameraUp = g_viewsInv[0][1].xyz;
    float3 cameraFront = g_viewsInv[0][2].xyz;
    float3 cameraPos = g_viewsInv[0][3].xyz;
    float2 delta = BillboardSize * g_billboardVertices[input.vertexID % 4];
    
    // We get a pos of billboards vertex
    uint index = (b_incinerationRangeBuffer[BUFFER_INDEX_OFFSET] + input.instanceID) % MAX_PARTICLES;
    IncinerationParticle particle = b_incinerationParticleBuffer[index];
    float3 particleWorldPos = particle.worldPos;
    float3 pos = particleWorldPos + (delta.x * cameraRight) + (delta.y * cameraUp);
    
    float4x4 viewProj = mul(g_views[0], g_proj);
    
    VS_OUTPUT output;
    output.pos = mul(float4(pos, 1.0), viewProj);
    output.emissive = particle.emissive;
    
    return output;
}

float4 ps_main(VS_OUTPUT input) : SV_Target0
{
    return float4(input.emissive, 1.0);
}

// =================
// COMPUTE incineration buffer update
// =================
RWBuffer<uint> u_rangeBuffer : register(u0);

#define MAX_PARTICLES 2048

[numthreads(1, 1, 1)]
void cs_main(uint3 ThreadID : SV_DispatchThreadID)
{
    uint expired = u_rangeBuffer[2];
    uint numParticles = u_rangeBuffer[0] - expired;
    u_rangeBuffer[0] = numParticles;
    u_rangeBuffer[1] = (u_rangeBuffer[1] + expired) % MAX_PARTICLES;
    u_rangeBuffer[2] = 0;
    
    // DrawIndexedInstancedIndirect
    u_rangeBuffer[3] = 6;
    u_rangeBuffer[4] = numParticles;
    u_rangeBuffer[5] = 0;
    u_rangeBuffer[6] = 0;
    u_rangeBuffer[7] = 0;
    
    uint rem = numParticles % 16;
    uint numGroups = numParticles / 16;
    u_rangeBuffer[8] = rem != 0 ? numGroups + 1 : numGroups; // DispatchIndirect draw call 1st param should be equal to number of particles
    u_rangeBuffer[9] = 1; // DispatchIndirect draw call 2nd threadGroupsY
    u_rangeBuffer[10] = 1; // DispatchIndirect draw call 3rd threadGroupsZ
}