#include "WaveGeneral.hlsli"

Texture2D g_skybox : register(MATERIAL_SRV_0);

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    // Transform the vertex position from object space to world space
    float4 worldPosition = mul(float4(input.position, 1.0f), worldMatrix);

    // Transform the vertex position from world space to clip space
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.texCoord = input.texCoord;
    output.texCoord.y = 1 - output.texCoord.y;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_skybox.Sample(g_samplerLinearClamp, input.texCoord);
}