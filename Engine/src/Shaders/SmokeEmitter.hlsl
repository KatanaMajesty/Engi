#include "BufferDefines.hlsli"
#include "SamplerDefines.hlsli"
#include "Lighting.hlsli"
#include "IBL.hlsli"

Texture2DArray<float4> t_mvea : register(t21);
Texture2DArray<float3> t_dbf : register(t22);
Texture2DArray<float3> t_rlu : register(t23);
Texture2D<float> t_depth : register(t24);

struct VS_INPUT
{
    uint vertexID : SV_VertexID;
    float3 position : BILLBOARD_POSITION;
    float3 color : BILLBOARD_COLOR;
    float initialSize : BILLBOARD_INITIAL_SIZE;
    float deathSize : BILLBOARD_DEATH_SIZE;
    float time : BILLBOARD_TIME;
    float lifetime : BILLBOARD_LIFETIME;
    float initialRotation : BILLBOARD_INITIAL_ROTATION;
    float rotation : BILLBOARD_ROTATION;
};

struct VS_OUTPUT
{
    float3 color : BILLBOARD_COLOR;
    float time : BILLBOARD_TIME;
    float lifetime : BILLBOARD_LIFETIME;
    float2 texCoords : BILLBOARD_TEXCOORDS;
    float3x3 basis : BILLBOARD_BASIS;
    float3 worldPos : BILLBOARD_WORLD_POS;
    float3 cameraPos : CAMERA_POS;
    float3 normal : BILLBOARD_NORMAL;
    float4 fragpos : BILLBOARD_CLIP_POS;
    float4 pos : SV_Position;
    float frameFraction : FRAME_FRACTION;
    uint frameIndex : FRAME_INDEX;
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

// Rodrigues' rotation formula rotates v by an angle theta around vector k by decomposing it into its components parallel and perpendicular to k
float3 Rodrigues_Rotation(float3 pos, float3 around, float3 NAxis, float angle)
{
    float3 toPos = pos - around;
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);
    float3 rotatedPoint = toPos * cosTheta + cross(NAxis, toPos) * sinTheta + NAxis * dot(NAxis, toPos) * (1.0 - cosTheta);
    return around + rotatedPoint;
}

// TODO: Remove temp hardcore
static const uint g_numTextures = 64;

VS_OUTPUT vs_main(VS_INPUT input)
{
    float dA = saturate(input.time / input.lifetime);
    float size = input.initialSize + (input.deathSize - input.initialSize) * smoothstep(0.0, 1.0, dA);
    
    float3 cameraRight = g_viewsInv[0][0].xyz;
    float3 cameraUp = g_viewsInv[0][1].xyz;
    float3 cameraFront = g_viewsInv[0][2].xyz;
    float3 cameraPos = g_viewsInv[0][3].xyz;
    float2 delta = size * g_billboardVertices[input.vertexID % 4];
    
    // We get a pos, then we apply Rodrigues' rotation of initial rotation and then rotate it by the angle increasing by time
    float3 pos = input.position + (delta.x * cameraRight) + (delta.y * cameraUp);
    pos = Rodrigues_Rotation(pos, input.position, cameraFront, input.initialRotation);
    pos = Rodrigues_Rotation(pos, input.position, cameraFront, lerp(0.0f, input.rotation, dA));
    
    VS_OUTPUT output;
    
    float4x4 viewProj = mul(g_views[0], g_proj);
    output.pos = mul(float4(pos, 1.0), viewProj);
    output.fragpos = output.pos;
    output.worldPos = pos;
    output.basis = float3x3(cameraRight, cameraUp, cameraFront);
    
    output.cameraPos = cameraPos;
    output.texCoords = g_billboardTexCoords[input.vertexID % 4];
    output.normal = normalize(-cameraFront);
    
    output.time = input.time;
    output.lifetime = input.lifetime;
    output.color = input.color;
    
    float timePerFrame = input.lifetime / g_numTextures; // 64 is the amount of textures in texture atlas
    float frameFraction = fmod(input.time, timePerFrame) / timePerFrame;
    output.frameFraction = frameFraction;
    output.frameIndex = uint(dA * g_numTextures);
    return output;
}

float Smoke_AlphaAttenuation(float spawnFraction, float deathFraction, float alpha, float lifetime, float time)
{
    float p1 = lifetime * spawnFraction; // first (spawnFraction * 100)% of lifetime
    float dL1 = saturate(time / p1);
    float retAlpha = lerp(0.0, alpha, dL1);
    
    float p2 = lifetime * deathFraction; // last (deathFraction * 100)% of lifetime
    float dL2 = 1.0 - saturate(max(time - (lifetime - p2), 0.0) / p2);
    retAlpha = lerp(0.0, retAlpha, dL2);
    return retAlpha;
}

static const float g_smokeThickness = 0.8;
static const float g_motionVectorScale = 0.0015;

// Linearize depth value for reversed z-buffer without farPlane value
float LinearizeDepth_ForInfinity(float d, float zNear)
{
    return zNear / d;
}

void Process_6WayLightmap(inout float3 color, in float3 L, in float3 Lr, in float3x3 basis, in float3 lightmapRLU, in float3 lightmapDBF)
{
    float3 px = basis[0];
    float3 nx = -px;
    color += Lr * lightmapRLU.r * max(0.0001, dot(px, L));
    color += Lr * lightmapRLU.g * max(0.0001, dot(nx, L));
        
    float3 py = basis[1];
    float3 ny = -py;
    color += Lr * lightmapRLU.b * max(0.0001, dot(py, L));
    color += Lr * lightmapDBF.r * max(0.0001, dot(ny, L));
        
    float3 pz = basis[2];
    float3 nz = -pz;
    color += Lr * lightmapDBF.g * max(0.0001, dot(pz, L));
    color += Lr * lightmapDBF.b * max(0.0001, dot(nz, L));
}

void GetLinearDepthValues(out float depth, out float sceneDepth, in float4 fragpos)
{
    float3 projCoords = fragpos.xyz / fragpos.w;
    float2 uv = float2(projCoords.x * 0.5 + 0.5, projCoords.y * -0.5 + 0.5);
    depth = LinearizeDepth_ForInfinity(projCoords.z, 0.1);
    sceneDepth = LinearizeDepth_ForInfinity(t_depth.Sample(g_samplerLinear, uv).r, 0.1);
}

float Smoke_SurfaceAttenuation(in float depth, in float sceneDepth)
{
    float dz = saturate(sceneDepth - depth);
    float depthAlpha = smoothstep(0.0, 1.0, saturate(dz / g_smokeThickness));
    return depthAlpha;
}

void Smoke_CameraAttenuation(inout float depthAlpha, in float depth)
{
    // Attenuation factor for the smoke dissolution near the camera
    float eps = 0.2;
    float invEps = 1.0 / eps;
    float attenuationFactor = saturate((depth - eps) * invEps);
    depthAlpha = lerp(0.0, depthAlpha, attenuationFactor);
}

float4 ps_main(VS_OUTPUT input) : SV_Target0
{
    float3 N = normalize(input.normal);
    
    float2 mv0 = 2.0 * t_mvea.Sample(g_activeSampler, float3(input.texCoords, input.frameIndex)).rg - 1.0; // current frame motion-vector
    float2 mv1 = 2.0 * t_mvea.Sample(g_activeSampler, float3(input.texCoords, input.frameIndex + 1)).rg - 1.0; // next frame motion-vector
    mv0.y = -mv0.y;
    mv1.y = -mv1.y;

    float time = input.frameFraction; // goes from 0.0 to 1.0 between two sequential frames
    
    float2 uv0 = input.texCoords; // texture sample uv for the current frame
    uv0 -= mv0 * g_motionVectorScale * time; // if MV points in some direction, then UV flows in opposite
    
    float2 uv1 = uv0; // texture sample uv for the next frame
    uv1 -= mv1 * g_motionVectorScale * (time - 1.0); // if MV points in some direction, then UV flows in opposite

    // ----------- sample textures -----------
    float3 uvw0 = float3(uv0, input.frameIndex);
    float3 uvw1 = float3(uv1, input.frameIndex + 1);
    float2 emissionAlpha0 = t_mvea.Sample(g_activeSampler, uvw0).ba;
    float2 emissionAlpha1 = t_mvea.Sample(g_activeSampler, uvw1).ba;

    // .x - right, .y - left, .z - up
    float3 lightmapRLU0 = t_rlu.Sample(g_activeSampler, uvw0).rgb;
    float3 lightmapRLU1 = t_rlu.Sample(g_activeSampler, uvw1).rgb;

    // .x - down, .y - back, .z - front
    float3 lightmapDBF0 = t_dbf.Sample(g_activeSampler, uvw0).rgb;
    float3 lightmapDBF1 = t_dbf.Sample(g_activeSampler, uvw1).rgb;

    // ----------- lerp values -----------
    float2 emissionAlpha = lerp(emissionAlpha0, emissionAlpha1, time);
    float3 lightmapRLU = lerp(lightmapRLU0, lightmapRLU1, time);
    float3 lightmapDBF = lerp(lightmapDBF0, lightmapDBF1, time);
    
    float3 lightColor = 0.0;
    for (uint lightIndex = 0; lightIndex < g_numDirLights; ++lightIndex)
    {
        ENGI_DirectionalLight light = b_dirLights[lightIndex];
        float3 L = normalize(-light.direction);
        
        Process_6WayLightmap(lightColor, L, light.ambient, input.basis, lightmapRLU, lightmapDBF);
    }
    
    for (lightIndex = 0; lightIndex < g_numPointLights; ++lightIndex)
    {
        ENGI_PointLight light = b_pointLights[lightIndex];
        float3 L = light.position - input.worldPos;
        float r = light.radius;
        float d2 = getDistance2(L);
        float3 Lr = calcSolidAngle(r * r, d2) * light.intensity * light.color;
        
        Process_6WayLightmap(lightColor, normalize(L), Lr, input.basis, lightmapRLU, lightmapDBF);
    }
    
    for (lightIndex = 0; lightIndex < g_numSpotLights; ++lightIndex)
    {
        ENGI_SpotLight light = b_spotLights[lightIndex];
        float3 L = light.position - input.worldPos;
        
        float3 cookieAttenuation = 1.0f;
        if (light.useSpotlightMask)
        {
            float sin = sqrt(1.0f - pow(light.outerCutoff, 2.0f));
            float tan = sin / light.outerCutoff;
            cookieAttenuation = getCookieAttenuation(tan, input.worldPos, light.worldToLocal);
        }
        
        float r = light.radius;
        float d2 = getDistance2(L);
        float cutoffIntensity = getSpotlightIntensity(normalize(L), normalize(light.direction), light.cutoff, light.outerCutoff);
        float3 Lr = light.color * light.intensity * cookieAttenuation * cutoffIntensity * calcSolidAngle(r * r, d2);
        
        Process_6WayLightmap(lightColor, normalize(L), Lr, input.basis, lightmapRLU, lightmapDBF);
    }
    
    float depth = 0.0;
    float sceneDepth = 0.0;
    GetLinearDepthValues(depth, sceneDepth, input.fragpos);
    float depthAlpha = Smoke_SurfaceAttenuation(depth, sceneDepth);
    
#if 1
    Smoke_CameraAttenuation(depthAlpha, depth);
#endif
    
    float alpha = Smoke_AlphaAttenuation(0.1, 0.2, emissionAlpha.g, input.lifetime, input.time) * depthAlpha;
    float3 color = lightColor * input.color * emissionAlpha.r;
    
    if (g_useIBL)
    {
        color += addDiffuseReflection(input.color, N, 0.0);
    }
    
    return float4(color, alpha);
}