#include "CommonRT.hlsl"

// Resources
Texture2D g_albedo : register(t0, space1);
Texture2D g_normalMap : register(t1, space1);

[shader("closesthit")]
void ClosestHit_LitObject(inout RayPayload payload, in Attributes attribs)
{
    const uint primitiveIndex = PrimitiveIndex();
    const float3 barycentrics = float3(1.0 - attribs.texcoord.x - attribs.texcoord.y, attribs.texcoord.x, attribs.texcoord.y);
    
    VertexAttributes attr = InterpolateAttributes(primitiveIndex, barycentrics);

    float3 position = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 viewDir = normalize(viewPos.xyz - position);
    uint2 screenSize = DispatchRaysDimensions().xy;
    int2 coord = floor(attribs.texcoord * screenSize);
    float4 albedocolour = g_albedo.Load(int3(coord, 0));
    float3 vertexNormal = attr.normal;
    float3 normal = normalize(mul((float3x3)viewMatrix, vertexNormal)); // To world space

    float3 lightcolour = ambientColour.rgb * ambientColour.a;

    for (int i = 0; i < MAX_LIGHT; ++i)
    {
        float3 lightDir = normalize(lights[i].pos.xyz - position);
        float3 reflectDir = reflect(-lightDir, normal);

        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);
        float3 diffuse = diff * lights[i].colour.rgb * lights[i].colour.a;

        // Specular
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        float3 specular = spec * lights[i].colour.rgb * lights[i].colour.a;

        lightcolour += diffuse + specular;
    }

    payload.colour = albedocolour.rgb * lightcolour * payload.attenuation;
}