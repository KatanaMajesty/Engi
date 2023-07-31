#include "LayoutDefines.hlsli"
#include "BufferDefines.hlsli"
#include "Common.hlsli"
#include "SamplerDefines.hlsli"
#include "TextureDefines.hlsli"

Texture2D<float> t_dissolutionTexture : register(t20);

struct IncinerationParticle
{
    float3 worldPos;
    float3 velocity;
    float3 emissive;
    float lifetime;
    uint parentInstanceID;
};

RWBuffer<uint> u_incinerationRangeBuffer : register(u5);
RWStructuredBuffer<IncinerationParticle> u_incinerationParticleBuffer : register(u6);

#define BUFFER_INDEX_NUM_PARTICLES 0
#define BUFFER_INDEX_OFFSET 1
#define BUFFER_INDEX_NUM_EXPIRED 2

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 worldNormal : MESH_NORMAL;
    float3 modelPos : MODEL_POS;
    float2 texCoords : TEXCOORD;
    float3 color : MESH_COLOR;
    float3 emission : MESH_EMISSION;
    float3x3 TBN : TBN_MAT;
    float time : TIME;
    float3 iSpherePos : INCINERATION_SPHERE_POS;
    float iSphereRadius : INCINERATION_SPHERE_RADIUS;
    uint instanceID : INSTANCE_ID;
};

static const float g_incinerationTime = 5.0;

#define INCINERATION_PER_VERTEX 128

void Incineration_SpawnParticle(float3 worldPos, float3 worldNormal, float3 emission, uint instanceID)
{
    uint incinerationID;
    InterlockedAdd(u_incinerationRangeBuffer[BUFFER_INDEX_NUM_PARTICLES], 1, incinerationID);
    if (incinerationID >= 2048)
    {
        InterlockedAdd(u_incinerationRangeBuffer[BUFFER_INDEX_NUM_PARTICLES], -1);
        return;
    }
    
    uint incinerationOffset = u_incinerationRangeBuffer[BUFFER_INDEX_OFFSET];
    incinerationID = (incinerationOffset + incinerationID) % 2048; // 2048 is max num of particles
    
    IncinerationParticle particle;
    particle.worldPos = worldPos;
    particle.velocity = worldNormal; // Just multiply normal by some constant to get nicer visual effect
    particle.emissive = emission;
    particle.lifetime = 0.0;
    particle.parentInstanceID = instanceID;
    
    u_incinerationParticleBuffer[incinerationID] = particle;
}

VS_OUTPUT vs_main(VS_INPUT input, uint vid : SV_VertexID)
{
    float3 worldNormal = convertToOrthogonalBasis(input.modelToWorld, convertToOrthogonalBasis(g_meshToModel, input.meshNormal));
    float3 modelPos = mul(float4(input.meshPosition, 1.0f), g_meshToModel).xyz;
    float3 worldPos = mul(float4(modelPos, 1.0f), input.modelToWorld).xyz;
    
    float4x4 viewProj = mul(g_views[0], g_proj);
    float4 pos = mul(float4(worldPos, 1.0f), viewProj);
    
    float3 emission = input.emission * input.emissionPow;
    
    VS_OUTPUT output;
    output.pos = pos;
    output.worldNormal = worldNormal;
    output.modelPos = modelPos;
    output.texCoords = input.meshTexCoords;
    
    output.color = input.color;
    output.emission = emission;
    
    output.TBN = constructTBN(input.modelToWorld, input.meshTangent, input.meshBitangent, worldNormal);
    output.time = input.time;
    
    // Incineration sphere radii
    float invIncinerationTime = 1.0 / g_incinerationTime;
    float prevFrameTime = max(0.0, input.time - input.timestep);
    float iSphereRadius = lerp(0.0, input.sphereRadiusMax, saturate(input.time * invIncinerationTime));
    float iSpherePrevFrameRadius = lerp(0.0, input.sphereRadiusMax, saturate(prevFrameTime * invIncinerationTime));
    
    // Incineration
    float incinerationDistance = length(modelPos - input.sphereOrigin);
    if (incinerationDistance < iSphereRadius && incinerationDistance > iSpherePrevFrameRadius)
    {
        if (vid % INCINERATION_PER_VERTEX == 0)
        {
            Incineration_SpawnParticle(worldPos, worldNormal, emission, input.instanceID);
        }
    }
    
    output.iSpherePos = input.sphereOrigin;
    output.iSphereRadius = iSphereRadius;
    
    output.instanceID = input.instanceID;
    return output;
}

struct PS_OUTPUT
{
    float4 albedo : SV_Target0;
    float4 normals : SV_Target1;
    float3 emission : SV_Target2;
    float2 roughnessMetalness : SV_Target3;
    uint objectID : SV_Target4;
};

PS_OUTPUT ps_main(VS_OUTPUT input)
{
    float3 albedo = input.color;
    if (g_useAlbedoTexture)
    {
        albedo = processAlbedoMap(g_activeSampler, input.texCoords).rgb;
    }
    
     // Highlight color for dissolved pixels
    float3 incinerationEmition = input.emission;
        
    // Attenuation factor for the highlight over time
    float eps = 0.05;
    float radiusDelta = input.iSphereRadius * eps;
    float innerSphereRadius = input.iSphereRadius - radiusDelta;
    
    float incinerationDistance = length(input.modelPos - input.iSpherePos);
    if (incinerationDistance < innerSphereRadius)
        discard;
    
    float attenuationFactor = saturate(smoothstep(input.iSphereRadius, innerSphereRadius, incinerationDistance));
    float3 interpolatedEmission = incinerationEmition * attenuationFactor;
    
    float3 N = normalize(input.worldNormal);
    float3 Ng = N;
    if (g_useNormalMap)
    {
        Ng = processNormalMap(g_activeSampler, input.texCoords, input.TBN);
    }
    
    float roughness = g_roughness;
    if (g_useRoughnessMap)
    {
        roughness = processRoughnessMap(g_activeSampler, input.texCoords).r;
    }
    
    float metalness = g_metallic;
    if (g_useMetalnessMap)
    {
        metalness = processMetalnessMap(g_activeSampler, input.texCoords).r;
    }
    
    PS_OUTPUT output;
    output.emission = interpolatedEmission;
    output.albedo = float4(albedo, 1.0);
    output.normals = float4(packOctahedron(N), packOctahedron(Ng));
    output.roughnessMetalness = float2(roughness, metalness);
    output.objectID = input.instanceID;
    
    return output;
}

#define PARTICLE_MAX_LIFETIME 5.0

Texture2D<float4> t_gbufferNormals : register(t22);
Texture2D<uint> t_gbufferObjectID : register(t25);
Texture2D<float> t_depthBuffer : register(t26);

static const float g_gravity = -9.81; // m/s^2

RWBuffer<uint> u_rangeBuffer : register(u0);
RWStructuredBuffer<IncinerationParticle> u_particleBuffer : register(u1);

void queryGBufferDimensions(out float width, out float height)
{
    t_gbufferObjectID.GetDimensions(width, height);
}

[numthreads(16, 1, 1)]
void cs_main(uint3 ThreadID : SV_DispatchThreadID)
{
    // Dont process on this thread as it is out of range
    if (ThreadID.x > u_rangeBuffer[BUFFER_INDEX_NUM_PARTICLES])
        return;
    
    uint index = (u_rangeBuffer[BUFFER_INDEX_OFFSET] + ThreadID.x) % 2048;
    
    IncinerationParticle p = u_particleBuffer[index];
    p.lifetime += g_timestep;
    if (p.lifetime > PARTICLE_MAX_LIFETIME)
    {
        InterlockedAdd(u_rangeBuffer[BUFFER_INDEX_NUM_EXPIRED], 1);
        return;
    }
    
    float4x4 viewProj = mul(g_views[0], g_proj);
    
    float3 newpos = p.worldPos + p.velocity * g_timestep;
    float4 particleClip = mul(float4(newpos, 1.0), viewProj);
    particleClip /= particleClip.w;

    float2 uv = particleClip.xy * float2(0.5, -0.5) + 0.5;
        
    float sceneDepth = t_depthBuffer.SampleLevel(g_samplerNearest, uv, 0.0);
    if (particleClip.z < sceneDepth)
    {
        float viewportWidth;
        float viewportHeight;
        queryGBufferDimensions(viewportWidth, viewportHeight);
        
        float2 pos = uv * float2(viewportWidth, viewportHeight);
        
        // We dont want to collide with own instance
        uint objectID = t_gbufferObjectID.Load(int3(pos, 0));
        if (objectID != p.parentInstanceID)
        {
            // Reflect normal, reduce speed
            float3 surfaceNormal = unpackOctahedron(t_gbufferNormals.SampleLevel(g_samplerNearest, uv, 0.0).zw);
            p.velocity = reflect(p.velocity, surfaceNormal) * 0.2;
        }
    }
    else
    {
        p.velocity += float3(0.0, g_gravity, 0.0) * g_timestep;
    }
    
    p.worldPos += p.velocity * g_timestep;
    u_particleBuffer[index] = p;
}