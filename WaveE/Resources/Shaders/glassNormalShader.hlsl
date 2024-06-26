#include "WaveGeneral.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normalVS : NORMAL_SS;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    // Transform the vertex position from object space to world space
    float4 worldPosition = mul(float4(input.position, 1.0f), worldMatrix);

    // Transform the vertex position from world space to clip space
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.normalVS = mul(float4(input.normal, 0.0f), worldMatrix).xyz;
    output.normalVS = mul(float4(output.normalVS, 0.0f), viewMatrix).xyz;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(normalize(input.normalVS), 1);
}