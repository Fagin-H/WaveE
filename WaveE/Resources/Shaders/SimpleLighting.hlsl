#include "WaveGeneral.hlsli"

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float3 worldPos : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    // Transform the vertex position from object space to world space
    float4 worldPosition = mul(float4(input.position, 1.0f), worldMatrix);
    output.worldPos = worldPosition.xyz;

    // Transform the vertex position from world space to clip space
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Pass through normal and color
    output.normal = mul((float3x3) worldMatrix, input.normal);
    output.color = input.color;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(viewPos.xyz - input.worldPos);
    
    float3 finalColor = ambientColor.rgb * ambientColor.a * input.color.rgb;

    for (int i = 0; i < MAX_LIGHT; ++i)
    {
        float3 lightDir = normalize(lights[i].pos.xyz - input.worldPos);
        float3 reflectDir = reflect(-lightDir, normal);

        // Diffuse lighting
        float diff = max(dot(normal, lightDir), 0.0);
        float3 diffuse = diff * lights[i].colour.rgb * lights[i].colour.a;

        // Specular lighting
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        float3 specular = spec * lights[i].colour.rgb * lights[i].colour.a;

        finalColor += (diffuse + specular) * input.color.rgb;
    }

    return float4(finalColor, 1);
}