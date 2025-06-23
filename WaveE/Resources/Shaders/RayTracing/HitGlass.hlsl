#include "CommonRT.hlsl"

// Resources
Texture2D g_normalMap : register(t0, space1);

cbuffer CameraBuffer : register(b0, space1)
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
void ClosestHit_Glass(inout RayPayload payload, in Attributes attribs)
{
    const uint primitiveIndex = PrimitiveIndex();
    const float3 barycentrics = float3(1.0 - attribs.texcoord.x - attribs.texcoord.y, attribs.texcoord.x, attribs.texcoord.y);
    VertexAttributes attr = InterpolateAttributes(primitiveIndex, barycentrics);

    float3 viewDir = normalize(viewPos.xyz - attr.position);
    float3 vertexNormal = attr.normal;
    float3 normal = normalize(mul((float3x3)viewMatrix, vertexNormal)); // To world space

    // Handle normal flipping when inside the object
    float3 I = -WorldRayDirection();
    float n1 = payload.isInside ? indexOfRefraction.x : 1;
    float n2 = payload.isInside ? 1 : indexOfRefraction.x;

    // Fresnel term for reflection/refraction weighting
    float fresnel = FresnelReflectAmount(n1, n2, normal, I);

    float3 reflectedcolour = float3(0,0,0);
    float3 refractedcolour = float3(0,0,0);

    if (payload.depth < MAX_RECURSION_DEPTH)
    {
        RayPayload newPayload;
        newPayload.attenuation = payload.attenuation * fresnel;
        newPayload.depth = payload.depth + 1;
        newPayload.isInside = payload.isInside;

        RayDesc reflectRay;
        reflectRay.Direction = reflect(I, normal);
        reflectRay.Origin = attr.position + 0.001 * reflectRay.Direction;
        reflectRay.TMin = 0.001;
        reflectRay.TMax = 10000;

        TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, reflectRay, newPayload);
        reflectedcolour = newPayload.colour;

        // Refraction
        float3 refractDir = refract(I, normal, n1 / n2);
        if (length(refractDir) > 0.0001f)
        {
            RayPayload refractPayload;
            refractPayload.attenuation = payload.attenuation * (1.0 - fresnel);
            refractPayload.depth = payload.depth + 1;
            refractPayload.isInside = !payload.isInside;

            RayDesc refractRay;
            refractRay.Direction = refractDir;
            refractRay.Origin = attr.position - 0.001 * refractDir;
            refractRay.TMin = 0.001;
            refractRay.TMax = 10000;

            TraceRay(SceneBVH, RAY_FLAG_NONE, 0xFF, 0, 0, 0, refractRay, refractPayload);
            refractedcolour = refractPayload.colour;
        }
        else
        {
            // Total internal reflection fallback → only reflection happens
            refractedcolour = float3(0,0,0);
        }
    }
    payload.colour = (reflectedcolour + refractedcolour) * payload.attenuation;
}