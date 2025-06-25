#include "CommonRT.hlsl"

// Raytracing output texture, accessed as a UAV
RWTexture2D< float4 > gOutput : register(u0);

[shader("raygeneration")] 
void RayGen() {
    uint2 pixelIndex = DispatchRaysIndex().xy;
    uint2 screenSize = DispatchRaysDimensions().xy;

    float aspect = ((float)screenSize.x) / screenSize.y;
    float2 d = (((pixelIndex.xy + 0.5f) / screenSize.xy) * 2.f - 1.f);

    float2 uv = (pixelIndex + 0.5f) / screenSize;
    float2 ndc = uv * 2.0f - 1.0f;
    ndc.x *= aspect;  // Apply aspect ratio to *NDC*, not to individual axes
    ndc.y *= -1;

    // Construct camera basis
    float3 right = viewMatrix[0].xyz;
    float3 up = viewMatrix[1].xyz;
    float3 forward = viewMatrix[2].xyz;  // Negate if forward points backward

    // Build ray direction in world space
    float3 rayDir = normalize(ndc.x * right + ndc.y * up + forward);

    RayDesc ray;
    ray.Origin = viewPos.xyz;
    ray.Direction = rayDir;
    ray.TMin = 0.001f;
    ray.TMax = 10000;

    RayPayload payload;
    payload.colour = float3(0, 0, 0);
    payload.attenuation = 1;
    payload.depth = 0;
    payload.isInside = false;

    TraceRay(SceneBVH,
        RAY_FLAG_NONE,
        0xFF, // Instance mask
        0,    // Ray contribution to hit group index
        0,    // Multiplier for geometry contribution
        0,    // Miss shader index
        ray,
        payload);

    gOutput[pixelIndex] = float4(payload.colour, 1.0);
}
