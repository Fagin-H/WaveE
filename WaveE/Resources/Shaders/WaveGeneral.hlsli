#pragma once
#include "SlotMacros.hlsli"

#define MAX_LIGHT 4

sampler g_samplerPointWrap : register(DEFAULT_SAMPLER_0);
sampler g_samplerLinearWrap : register(DEFAULT_SAMPLER_1);
sampler g_samplerPointClamp : register(DEFAULT_SAMPLER_2);
sampler g_samplerLinearClamp : register(DEFAULT_SAMPLER_3);

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
    float4 color : COLOR;
};