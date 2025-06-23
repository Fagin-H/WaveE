#include "CommonRT.hlsl"

// Raytracing output texture, accessed as a UAV
RWTexture2D< float4 > gOutput : register(u0);

[shader("raygeneration")] 
void RayGen() {
    uint2 pixelIndex = DispatchRaysIndex().xy;
    uint2 screenSize = DispatchRaysDimensions().xy;

    float2 uv = (float2(pixelIndex) + 0.5f) / float2(screenSize);
    uv = uv * 2.0f - 1.0f;  // [-1,1] range

    // Generate camera ray direction in view space
    float4 ndc = float4(uv, 1, 1);
    float4 viewDir = mul(inverseProjectionMatrix, ndc);
    viewDir /= viewDir.w;
    float3 rayDir = normalize(viewDir.xyz);

    // Transform to world space
    rayDir = mul((float3x3)viewMatrix, rayDir);

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
