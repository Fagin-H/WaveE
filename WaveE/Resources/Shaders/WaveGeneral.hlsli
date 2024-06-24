#pragma once
#include "SlotMacros.hlsli"

#define MAX_LIGHT 4

SamplerState g_samplerPointWrap : register(DEFAULT_SAMPLER_0);
SamplerState g_samplerLinearWrap : register(DEFAULT_SAMPLER_1);
SamplerState g_samplerPointClamp : register(DEFAULT_SAMPLER_2);
SamplerState g_samplerLinearClamp : register(DEFAULT_SAMPLER_3);

struct Light
{
    float4 pos;
    float4 colour; // Alpha used as intensity
};

cbuffer CameraBuffer : register(FRAME_CBV_0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
    float4 viewPos;
    float4 time;
};

cbuffer LightBuffer : register(FRAME_CBV_1)
{
    float4 ambientColor;
    Light lights[MAX_LIGHT];
};

cbuffer ObjectBuffer : register(DRAW_CBV_0)
{
    matrix worldMatrix;
};

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
};

// Function to calculate the cotangent frame
float3x3 CalculateTangentFrame(float3 vertexNormal, float3 position, float2 texcoord)
{
    // Get edge vectors of the pixel triangle
    float3 dp1 = ddx(position);
    float3 dp2 = ddy(position);
    float2 duv1 = ddx(texcoord);
    float2 duv2 = ddy(texcoord);
    
    // Solve the linear system
    float3 dp2perp = cross(dp2, vertexNormal);
    float3 dp1perp = cross(vertexNormal, dp1);
    float3 tangent = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 bitangent = dp2perp * duv1.y + dp1perp * duv2.y;
    
    // Construct a scale-invariant frame
    float invmax = rsqrt(max(dot(tangent, tangent), dot(bitangent, bitangent)));
    return float3x3(tangent * invmax, bitangent * invmax, vertexNormal);
}

// Function to format and perturb the normal using a normal map
float3 GetNormal(float3 vertexNormal, float3 normalMap, float3 viewDirection, float2 texcoord)
{
    normalMap = normalMap * 2 - 1;
    
    normalMap.y = -normalMap.y;
    
    float3x3 TBN = CalculateTangentFrame(vertexNormal, -viewDirection, texcoord);
    return normalize(mul(TBN, normalMap));
}