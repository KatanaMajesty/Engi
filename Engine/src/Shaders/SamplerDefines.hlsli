#ifndef __ENGI_SAMPLERS_HLSL__
#define __ENGI_SAMPLERS_HLSL__

SamplerState g_activeSampler : register(s0);
SamplerState g_samplerLinear : register(s1);
SamplerState g_samplerTrilinear : register(s2);
SamplerState g_samplerAnisotropic : register(s3);
SamplerComparisonState g_samplerShadow : register(s4);
SamplerState g_samplerNearest : register(s5);
SamplerState g_samplerLinearClamp : register(s6);

#endif // __ENGI_SAMPLERS_HLSL__