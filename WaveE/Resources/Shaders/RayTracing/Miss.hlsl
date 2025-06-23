#include "CommonRT.hlsl"

[shader("miss")]
void Miss(inout RayPayload payload : SV_RayPayload)
{
    payload.colour = float3(0.53, 0.81, 0.92) * payload.attenuation;
}