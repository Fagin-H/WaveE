#include "WaveGeneral.hlsli"

Texture2D m_albedo : register(MATERIAL_SRV_0);
Texture2D m_normal : register(MATERIAL_SRV_1);

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
    float3 worldPos : POSITION;
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

    // Sample the normal texture and transform it into world space
    output.normal = normalize(mul(input.normal, (float3x3) worldMatrix));

    // Pass through UV
    output.texCoord = input.texCoord;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 viewDir = normalize(viewPos.xyz - input.worldPos);
    float3 vertexNormal = normalize(input.normal);
    float3 normalMap = m_normal.Sample(g_samplerLinearWrap, input.texCoord);
    float3 normal = GetNormal(vertexNormal, normalMap, viewDir, input.texCoord);
    float4 colour = m_albedo.Sample(g_samplerLinearWrap, input.texCoord);
    
    float3 lightColour = ambientColor.rgb * ambientColor.a;

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

        lightColour += (diffuse + specular);
    }
    
    
    return colour * float4(lightColour, 1);
}