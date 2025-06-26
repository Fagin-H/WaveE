#include "CommonRT.hlsl"

[shader("miss")]
void Miss(inout RayPayload payload : SV_RayPayload)
{
    float3 position = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 positionNorm = normalize(position);
    positionNorm = clamp(positionNorm + 0.5, 0, 1);

    payload.colour = float3(0.53, 0.81, 0.92) * payload.attenuation * positionNorm.y;
}