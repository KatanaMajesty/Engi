#ifndef __ENGI_COMMON_HLSL__
#define __ENGI_COMMON_HLSL__

static const float MATH_PI = 3.141592;
static const float MATH_EPS = 0.0001f;
static const float MATH_PHI = (1.0f + sqrt(5.0f)) / 2.0f;

float3x3 constructTBN(float4x4 transformation, float3 tangent, float3 bitangent, float3 normal)
{
    float3 T = normalize(mul(float4(tangent, 0.0f), transformation).xyz);
    float3 B = normalize(mul(float4(bitangent, 0.0f), transformation).xyz);
    float3 N = normalize(normal);
    return float3x3(T, B, N);
}

float3 applyTBN(float3 normal, float3x3 TBN)
{
    float3 o = normal * 2.0f - 1.0f;
    o = normalize(mul(o, TBN));
    return o;
}

float invLerp(float from, float to, float value)
{
    return (value - from) / (to - from);
}

float remap(float origFrom, float origTo, float targetFrom, float targetTo, float value)
{
    float rel = invLerp(origFrom, origTo, value);
    return lerp(targetFrom, targetTo, rel);
}

float3 convertToOrthogonalBasis(float4x4 transformation, float3 v)
{
    float3 AxisX = normalize(transformation[0].xyz);
    float3 AxisY = normalize(transformation[1].xyz);
    float3 AxisZ = normalize(transformation[2].xyz);
    float3 v1 = v.x * AxisX + v.y * AxisY + v.z * AxisZ;
    return v1;
}

static const float g_xy[] = { -1.0f, -1.0f, 3.0f, -1.0f };
static const float g_uv[] = { 0.0f, 1.0f, 2.0f, 1.0f, 0.0f, -1.0f };

float4 fullscreenTrianglePos(uint vid)
{
    return float4(g_xy[vid + 1], g_xy[vid], 0.0f, 1.0f);
}

float2 fullscreenTriangleUV(uint vid)
{
    return float2(g_uv[(vid * 2)], g_uv[(vid * 2) + 1]);
}

// Fibonacci hemisphere point uniform distribution
float3 fibonacciHemispherePoint(out float NdotV, float i, float n)
{
	float theta = 2.0f * MATH_PI * i / MATH_PHI;
	float phiCos = NdotV = 1.0f - (i + 0.5f) / n;
	float phiSin = sqrt(1.0f - phiCos * phiCos);
	float thetaCos, thetaSin;
    sincos(theta, thetaSin, thetaCos);
	return float3(thetaCos * phiSin, thetaSin * phiSin, phiCos);
}

// Frisvad with z == -1 problem avoidance
void basisFromDir(out float3 right, out float3 up, in float3 dir)
{
    float k = 1.0 / max(1.0 + dir.z, 0.00001);
    float a = dir.y * k;
    float b = dir.y * a;
    float c = -dir.x * a;
    right = float3(dir.z + b, c, -dir.x);
    up = float3(c, 1.0 - b, -dir.y);
}

// Frisvad with z == -1 problem avoidance
float3x3 basisFromDir(float3 dir)
{
    float3x3 rotation;
    rotation[2] = dir;
    basisFromDir(rotation[0], rotation[1], dir);
    return rotation;
}

float3 tangentToWorld(float3 v, float3 N, float3 R, float3 U)
{
    return R * v.x + U * v.y + N * v.z;
}

// Compute Van der Corput radical inverse
// See: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Sphere's closest point calcultaion may return direction pointing beneath surface horizon (dot(N, dir) < 0), 
// use clampDirToHorizon to fix it.
void clampDirToHorizon(inout float3 dir, float3 N, float minNdotL)
{
    float NdotL = dot(N, dir);
    if (NdotL < minNdotL)
    {
        dir = normalize(dir + (minNdotL - NdotL) * N);
    }
}

float calcSolidAngle(float r2, float d2)
{
    float sin2Phi = saturate(r2 / d2);
    float cosPhi = sqrt(1.0f - sin2Phi);
    float h = 1.0f - cosPhi; // unit-sphere's height
    float w = 2.0f * MATH_PI * h;
    return w;
}

float calcSubtendedAngle(float r, float Dist)
{
    // For a spherical object whose actual diameter equals d = 2r, and where 
    // D is the distance to the center of the sphere, the angular diameter can be found by the formula
    // Reference: https://en.wikipedia.org/wiki/Angular_diameter
    float xi = 2.0f * atan(r / Dist);
    return xi;
}

// Reference: "Real Shading in Unreal Engine 4" by Brian Karis, Epic Games
// Reference: "Advances in Lighting and AA, Decima", SIGGRAPH 2017, slide 10
float3 sphereClosestPoint_Karis(float3 L, float r, float3 Reflection)
{
    float LdotR = dot(L, Reflection);
    if (LdotR < 0.0f)
        return L;
    
    float3 centerToRay = LdotR * Reflection - L;
    float3 closestPoint = L + centerToRay * saturate(r / length(centerToRay));
    return closestPoint;
}

uint selectCubeFace(float3 unitDir)
{
    float maxVal = max(abs(unitDir.x), max(abs(unitDir.y), abs(unitDir.z)));
    uint maxIndex = abs(unitDir.x) == maxVal ? 0 : (abs(unitDir.y) == maxVal ? 2 : 4);
    return maxIndex + (asuint(unitDir[maxIndex / 2]) >> 31); // same as:
    // return maxIndex + (unitDir[maxIndex / 2] < 0.f ? 1u : 0u);
}

float2 nonZeroSign(float2 v)
{
    return float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
}

float2 packOctahedron(float3 v)
{
    float2 p = v.xy / (abs(v.x) + abs(v.y) + abs(v.z));
    return v.z <= 0.0 ? (float2(1.0, 1.0) - abs(p.yx)) * nonZeroSign(p) : p;
}

float3 unpackOctahedron(float2 oct)
{
    float3 v = float3(oct, 1.0 - abs(oct.x) - abs(oct.y));
    if (v.z < 0)
        v.xy = (float2(1.0, 1.0) - abs(v.yx)) * nonZeroSign(v.xy);
    return normalize(v);
}


// Linearizes a Z buffer value
float LinearizeDepth(float depth)
{
    const float zFar = 100.0;
    const float zNear = 0.1;

    // bias it from [0, 1] to [-1, 1]
    float l = zNear / (zFar - depth * (zFar - zNear)) * zFar;

    return (l * 2.0) - 1.0;
}

#endif // __ENGI_COMMON_HLSL__