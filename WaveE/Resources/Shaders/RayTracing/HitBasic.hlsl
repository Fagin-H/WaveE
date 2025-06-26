#include "CommonRT.hlsl"

// Resources
Texture2D g_albedo : register(t3);//register(t0, space1);

[shader("closesthit")]
void ClosestHit_LitObject(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const uint primitiveIndex = PrimitiveIndex();
    const float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    
    VertexAttributes attr = InterpolateAttributes(primitiveIndex, barycentrics);

    float3 position = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 viewDir = normalize(viewPos.xyz - position);
    uint width;
	uint height;
	g_albedo.GetDimensions(width, height);
    float4 albedocolour = g_albedo.Load(int3(attr.uv * float2(width, height), 0));
    float3 vertexNormal = attr.normal;
    float3 normal = vertexNormal; // To world space

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