#include "CommonRT.hlsl"

// Resources
cbuffer GlassBuffer : register(b2)//register(b0, space1)
{
    float4 indexOfRefraction;
};

float FresnelReflectAmount(float n1, float n2, float3 normal, float3 incident)
{
    // Schlick aproximation
    float r0 = (n1 - n2) / (n1 + n2);
    r0 *= r0;
    float cosX = -dot(normal, incident);
    if (n1 > n2)
    {
        float n = n1 / n2;
        float sinT2 = n * n * (1.0 - cosX * cosX);
            // Total internal reflection
        if (sinT2 > 1.0)
            return 1.0;
        cosX = sqrt(1.0 - sinT2);
    }
    float x = 1.0 - cosX;
    float ret = r0 + (1.0 - r0) * x * x * x * x * x;
 
    return ret;
}

[shader("closesthit")]
void ClosestHit_Glass(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const uint primitiveIndex = PrimitiveIndex();
    const float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    VertexAttributes attr = InterpolateAttributes(primitiveIndex, barycentrics);

    float3 position = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 viewDir = normalize(viewPos.xyz - position);
    float3 vertexNormal = attr.normal;
    float3 normal = vertexNormal; // To world space
    if(payload.isInside)
    {
        normal *= -1;
    }
    
    float3 I = WorldRayDirection();
    float n1 = payload.isInside ? indexOfRefraction.x : 1;
    float n2 = payload.isInside ? 1 : indexOfRefraction.x;

    // Fresnel term for reflection/refraction weighting
    float fresnel = FresnelReflectAmount(n1, n2, normal, I);
    fresnel = clamp(fresnel, 0, 1);
    float3 reflectedcolour = float3(0,0,0);
    float3 refractedcolour = float3(0,0,0);
    float3 refractDir;
    RayPayload refractPayload;
    if (payload.depth < MAX_RECURSION_DEPTH)
    {
        RayPayload newPayload;
        newPayload.attenuation = fresnel;
        newPayload.depth = payload.depth + 1;
        newPayload.isInside = payload.isInside;

        RayDesc reflectRay;
        reflectRay.Direction = reflect(I, normal);
        reflectRay.Origin = position + 0.001 * reflectRay.Direction;
        reflectRay.TMin = 0.001;
        reflectRay.TMax = 10000;

        TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, reflectRay, newPayload);
        reflectedcolour = newPayload.colour;

        // Refraction
        refractDir = refract(I, normal, n1 / n2);
        if (length(refractDir) > 0.0001f)
        {
            refractPayload.attenuation = (1.0 - fresnel);
            refractPayload.depth = payload.depth + 1;
            refractPayload.isInside = !payload.isInside;

            RayDesc refractRay;
            refractRay.Direction = refractDir;
            refractRay.Origin = position + 0.001 * refractDir;
            refractRay.TMin = 0.001;
            refractRay.TMax = 10000;

            TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, refractRay, refractPayload);
            refractedcolour = refractPayload.colour;
        }
    }

    payload.colour = (reflectedcolour + refractedcolour) * payload.attenuation;
}