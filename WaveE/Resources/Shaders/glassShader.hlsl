#include "WaveGeneral.hlsli"

cbuffer GlassConstantData : register(GLOBAL_CBV_0)
{
    float4 colourAbsorption;
    float4 useInternalReflections;
    float2 screenRes;
    float refractionIndex;
    float maxItterations;
};

Texture2D g_mainDiffuse : register(GLOBAL_SRV_0);
Texture2D g_mainDepth : register(GLOBAL_SRV_1);
Texture2D g_backDepth : register(GLOBAL_SRV_2);
Texture2D g_frountDepth : register(GLOBAL_SRV_3);
Texture2D g_backNormal : register(GLOBAL_SRV_4);
Texture2D g_frountNormal : register(GLOBAL_SRV_5);

struct PSInput
{
    float4 position : SV_POSITION; //<< This attribute must exist.
    float3 normal : NORMAL;
    float3 normalVS : NORMAL_SS;
    float2 texCoord : UV;
    float3 vsPosition : VS_POS;
};

PSInput VSMain(VSInput input)
{
PSInput output;

    // Transform the vertex position from object space to world space
    float4 worldPosition = mul(float4(input.position, 1.0f), worldMatrix);

    // Transform the vertex position from world space to clip space
    output.vsPosition = mul(worldPosition, viewMatrix).xyz;
    output.position = mul(float4(output.vsPosition, 1), projectionMatrix);

    // Sample the normal texture and transform it into world space
    output.normal = normalize(mul(input.normal, (float3x3) worldMatrix));

    output.normalVS = mul(float4(input.normal, 0.0f), worldMatrix).xyz;
    output.normalVS = mul(float4(output.normalVS, 0.0f), viewMatrix).xyz;

    // Pass through UV
    output.texCoord = input.texCoord;

    return output;
}

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

float3 FindIntersectionWithDepth(float3 rayPosTS, float3 rayDirTS)
{
    int MAX_ITERATION = maxItterations;
    float MAX_THICKNESS = 0.1;
        
    float xScale = (1 - rayPosTS.x) / rayDirTS.x;
    if (xScale < 0)
    {
        xScale = -rayPosTS.x / rayDirTS.x;
    }
    float yScale = (1 - rayPosTS.y) / rayDirTS.y;
    if (yScale < 0)
    {
        yScale = -rayPosTS.y / rayDirTS.y;
    }
    float zScale = (1 - rayPosTS.z) / rayDirTS.z;
    if (zScale < 0)
    {
        zScale = -rayPosTS.z / rayDirTS.z;
    }
    
    float scale = min(min(xScale, yScale), zScale);
    float3 vReflectionEndPosInTS = rayPosTS + rayDirTS * scale;
    
    float3 dp = vReflectionEndPosInTS - rayPosTS;
    int2 startScreenPos = int2(rayPosTS.xy * screenRes.xy);
    int2 endPosScreenPos = int2(vReflectionEndPosInTS.xy * screenRes.xy);
    int2 dp2 = endPosScreenPos - startScreenPos;
    int max_dist = max(max(abs(dp2.x), abs(dp2.y)), 1);
    max_dist = min(max_dist, MAX_ITERATION);
    dp /= max_dist;
        
    float4 rayPosInTS = float4(rayPosTS.xyz + dp, 0);
    float4 vRayDirInTS = float4(dp.xyz, 0);
    
    [loop] for (int i = 0; i < max_dist; i++)
    {
        float depth = g_mainDepth.Sample(g_samplerPointClamp, rayPosInTS.xy).x;
        float thickness = rayPosInTS.z - depth;
        if (thickness >= 0 && thickness < MAX_THICKNESS)
        {
            float3 shiftBackFactor = float3(-vRayDirInTS.xy, 0) * min(i, 2);
            //rayPosInTS.z = depth;
            return rayPosInTS.xyz + shiftBackFactor;
        }
        rayPosInTS += vRayDirInTS;
    }
    
    return rayPosInTS;
}

float4 FindIntersectionWithDepthIternal(float3 rayPosTS, float3 rayDirTS)
{
    int MAX_ITERATION = maxItterations;
    float MAX_THICKNESS = 0.1;
        
    float xScale = (1 - rayPosTS.x) / rayDirTS.x;
    if (xScale < 0)
    {
        xScale = -rayPosTS.x / rayDirTS.x;
    }
    float yScale = (1 - rayPosTS.y) / rayDirTS.y;
    if (yScale < 0)
    {
        yScale = -rayPosTS.y / rayDirTS.y;
    }
    float zScale = (1 - rayPosTS.z) / rayDirTS.z;
    if (zScale < 0)
    {
        zScale = -rayPosTS.z / rayDirTS.z;
    }
    
    float scale = min(min(xScale, yScale), zScale);
    float3 vReflectionEndPosInTS = rayPosTS + rayDirTS * scale;
    
    float3 dp = vReflectionEndPosInTS - rayPosTS;
    int2 startScreenPos = int2(rayPosTS.xy * screenRes.xy);
    int2 endPosScreenPos = int2(vReflectionEndPosInTS.xy * screenRes.xy);
    int2 dp2 = endPosScreenPos - startScreenPos;
    int max_dist = max(max(abs(dp2.x), abs(dp2.y)), 1);
    max_dist = min(max_dist, MAX_ITERATION);
    dp /= max_dist;
        
    float4 rayPosInTS = float4(rayPosTS.xyz + dp, 0);
    float4 vRayDirInTS = float4(dp.xyz, 0);
    
    [loop]
    for (int i = 0; i < max_dist; i++)
    {
        rayPosInTS += vRayDirInTS;
        
        float depthB = g_backDepth.Sample(g_samplerPointClamp, rayPosInTS.xy).x;
        float depthF = g_frountDepth.Sample(g_samplerPointClamp, rayPosInTS.xy).x;
            
        if (rayPosInTS.z > depthB || rayPosInTS.z < depthF)
        {
            if (rayPosInTS.z > depthB)
            {
                rayPosInTS.z = depthB;
                return float4(rayPosInTS.xyz, 1);
            }
            else
            {
                rayPosInTS.z = depthF;
                return float4(rayPosInTS.xyz, -1);
            }
        }
    }
    
    return float4(rayPosInTS.xyz, 0);
}

float3 ViewSpaceToClipSpaceRay(float3 posVS, float3 directionVS)
{
    float3 startPosVS;
    float3 endPosVS;
    
    if (directionVS.z > 0)
    {
        startPosVS = posVS - directionVS * 100;
        endPosVS = posVS;
    }
    else
    {
        startPosVS = posVS;
        endPosVS = posVS + directionVS * 100;
    }
    float4 startPosCS = mul(float4(startPosVS, 1), projectionMatrix);
    startPosCS.xyz /= startPosCS.w;
    float4 endPosCS = mul(float4(endPosVS, 1), projectionMatrix);
    endPosCS.xyz /= endPosCS.w;

    float3 directionCS = normalize(endPosCS.xyz - startPosCS.xyz);
    
    return directionCS;
}

float4 PSMain(PSInput input) : SV_TARGET
{ 
    float indexOfRefraction = 1 / refractionIndex;
    float3 refractColour = float3(0, 0, 0);
    float3 reflectColour = float3(0, 0, 0);
    
    float3 normalVS = normalize(input.normalVS);
    float3 rayPosTS = float3(((input.position.xy + 0.5) / screenRes.xy), input.position.z);
    float3 rayPosClipSpace = rayPosTS;
    rayPosClipSpace.xy = rayPosClipSpace.xy * 2 - 1;
    rayPosClipSpace.y *= -1;
    
    float4 rayPosVS = float4(input.vsPosition, 1);
    
    // Ray from camera to fragment
    float3 incidentDirectionVS = normalize(rayPosVS.xyz);
    float reflectedToRefractedRatio = FresnelReflectAmount(indexOfRefraction, 1, normalVS, incidentDirectionVS);
    
    
    float3 refractedRayDirVS = refract(incidentDirectionVS, normalVS, indexOfRefraction);
    
    
    bool skipRefract = false;
    if (length(refractedRayDirVS) < 0.5)
    {
        skipRefract = true;
    }
    
    float3 reflectedRayDirVS = reflect(incidentDirectionVS, normalVS);
    float3 reflectedRayDirCS = ViewSpaceToClipSpaceRay(rayPosVS.xyz, reflectedRayDirVS);
    
    // Find reflected light colour
    float3 reflectedRayDirTS = reflectedRayDirCS.xyz;
    reflectedRayDirTS.xy *= float2(0.5, -0.5);
    float3 foundPosReflectedTS = FindIntersectionWithDepth(rayPosTS.xyz, reflectedRayDirTS);
    reflectColour = g_mainDiffuse.Sample(g_samplerLinearClamp, foundPosReflectedTS.xy).xyz;
    
    //Find Refracted light colour
    bool foundRefracted = false;
    if (!skipRefract)
    {       
        float3 refractedRayDirCS = ViewSpaceToClipSpaceRay(rayPosVS.xyz, refractedRayDirVS);
        float3 refractedRayDirTS = refractedRayDirCS.xyz;
        refractedRayDirTS.xy *= float2(0.5, -0.5);
        
        // Find intersection with back face of self
        float4 foundPosRefractedTS = FindIntersectionWithDepthIternal(rayPosTS.xyz, refractedRayDirTS);
        foundRefracted = foundPosRefractedTS.w != 0;
        
        if (foundRefracted)
        {
            float4 foundPosRefractedVS;
            // Find ray refracted out of object
            float3 foundPosRefractedCS = foundPosRefractedTS.xyz;
            foundPosRefractedCS.xy -= float2(0.5, 0.5);
            foundPosRefractedCS.xy /= float2(0.5, -0.5);
        
            foundPosRefractedVS = mul(float4(foundPosRefractedCS, 1), inverseProjectionMatrix);
            foundPosRefractedVS.xyz /= foundPosRefractedVS.w;
            float distTraveled = length(foundPosRefractedVS.xyz - rayPosVS.xyz);
            
            float3 foundNormalVS;
            if (foundPosRefractedTS.w > 0)
            {
                foundNormalVS = -normalize((g_backNormal.Sample(g_samplerLinearClamp, foundPosRefractedTS.xy).xyz));
            }
            else
            {
                foundNormalVS = -normalize((g_frountNormal.Sample(g_samplerLinearClamp, foundPosRefractedTS.xy).xyz));
            }
            float3 doubleRefractedRayDirVS = refract(refractedRayDirVS, foundNormalVS, 1 / indexOfRefraction);
            float reflectedToRefractedRatio2 = FresnelReflectAmount(1 / indexOfRefraction, 1, foundNormalVS, refractedRayDirVS);
            float3 foundPosRefractedReflectedTS;
            if (length(doubleRefractedRayDirVS) < 0.5)
            {
                foundRefracted = false;
            }
            else
            {
                float3 doubleRefractedRayDirCS = ViewSpaceToClipSpaceRay(foundPosRefractedVS.xyz, doubleRefractedRayDirVS);
            
                float3 doubleRefractedRayDirTS = doubleRefractedRayDirCS.xyz;
                doubleRefractedRayDirTS.xy *= float2(0.5, -0.5);
                
                foundPosRefractedReflectedTS = FindIntersectionWithDepth(foundPosRefractedTS.xyz, doubleRefractedRayDirTS);
            }
            
            if (foundRefracted)
            {
                refractColour = g_mainDiffuse.Sample(g_samplerLinearClamp, foundPosRefractedReflectedTS.xy).xyz;
                float3 absorb = exp(-colourAbsorption.xyz * distTraveled * colourAbsorption.w);
                refractColour *= absorb;
            }
            if (useInternalReflections.x > 0)
            {
                // Find ray reflected internally
            
                float3 refractedReflectedRayDirVS = reflect(refractedRayDirVS, foundNormalVS);
                float3 refractedReflectedRayDirCS = ViewSpaceToClipSpaceRay(foundPosRefractedVS.xyz, refractedReflectedRayDirVS);
                float3 refractedReflectedRayDirTS = refractedReflectedRayDirCS.xyz;
                refractedReflectedRayDirTS.xy *= float2(0.5, -0.5);
        
                float4 foundPosRefractedReflectedTS = FindIntersectionWithDepthIternal(foundPosRefractedTS.xyz, refractedReflectedRayDirTS);
                bool foundRefractedRefracted = foundPosRefractedReflectedTS.w != 0;
            
                if (foundRefractedRefracted)
                {
                    // Find ray refracted out of object
                    float4 foundPosRefractedReflectedVS;
                    float3 foundPosRefractedReflectedCS = foundPosRefractedReflectedTS.xyz;
                    foundPosRefractedReflectedCS.xy -= float2(0.5, 0.5);
                    foundPosRefractedReflectedCS.xy /= float2(0.5, -0.5);
        
                    foundPosRefractedReflectedVS = mul(float4(foundPosRefractedReflectedCS, 1), inverseProjectionMatrix);
                    foundPosRefractedReflectedVS.xyz /= foundPosRefractedReflectedVS.w;
                    distTraveled += length(foundPosRefractedVS.xyz - foundPosRefractedReflectedVS.xyz);
            
                    float3 foundNormalRefractedReflectedVS;
                    if (foundPosRefractedReflectedTS.w > 0)
                    {
                        foundNormalRefractedReflectedVS = -normalize((g_backNormal.Sample(g_samplerLinearClamp, foundPosRefractedReflectedTS.xy).xyz));
                    }
                    else
                    {
                        foundNormalRefractedReflectedVS = -normalize((g_frountNormal.Sample(g_samplerLinearClamp, foundPosRefractedReflectedTS.xy).xyz));
                    }
                    float3 refractedReflectedRefractedRayDirVS = refract(refractedReflectedRayDirVS, foundNormalRefractedReflectedVS, 1 / indexOfRefraction);
                
                
                    if (length(refractedReflectedRefractedRayDirVS) < 0.5)
                    {
                        foundRefractedRefracted = false;
                    }
                    else
                    {
                        float3 refractedReflectedRefractedRayDirCS = ViewSpaceToClipSpaceRay(foundPosRefractedReflectedVS.xyz, refractedReflectedRefractedRayDirVS);
            
                        float3 refractedReflectedRefractedRayDirTS = refractedReflectedRefractedRayDirCS.xyz;
                        refractedReflectedRefractedRayDirTS.xy *= float2(0.5, -0.5);
                
                        foundPosRefractedReflectedTS.xyz = FindIntersectionWithDepth(foundPosRefractedReflectedTS.xyz, refractedReflectedRefractedRayDirTS);
                    }
                    if (foundRefractedRefracted)
                    {
                        float3 absorb = exp(-colourAbsorption.xyz * distTraveled * colourAbsorption.w);
                        if (foundRefracted)
                        {
                            refractColour = refractColour * (1 - reflectedToRefractedRatio2) + g_mainDiffuse.Sample(g_samplerLinearClamp, foundPosRefractedReflectedTS.xy).xyz * reflectedToRefractedRatio2 * absorb;
                        }
                        else
                        {
                            refractColour = g_mainDiffuse.Sample(g_samplerLinearClamp, foundPosRefractedReflectedTS.xy).xyz * absorb;
                        }
                        foundRefracted = true;
                    }
            }
            }

        }
        
    }
    
    // Calculate final colour
    float3 colour;
    if(foundRefracted)
    {
        colour = reflectColour * reflectedToRefractedRatio + refractColour * (1 - reflectedToRefractedRatio);
    }
    else
    {
        colour = reflectColour;
    }
    
    return float4(colour, 1);
}