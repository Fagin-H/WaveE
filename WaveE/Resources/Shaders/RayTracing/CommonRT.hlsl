#define MAX_LIGHT 4
#define MAX_RECURSION_DEPTH 5

#pragma pack_matrix( row_major )

// ========== Global Resources ==========
RaytracingAccelerationStructure SceneBVH : register(t0);     // Acceleration Structure
TextureCube g_Skybox : register(t1);
ByteAddressBuffer vertices : register(t2);//register(t1, space1);

cbuffer CameraBuffer : register(b0)
{
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix inverseProjectionMatrix;
    float4 viewPos;
    float4 time;
};

struct Light
{
    float4 pos;
    float4 colour; // Alpha used as intensity
};

cbuffer LightBuffer : register(b1)
{
    float4 ambientColour;
    Light lights[MAX_LIGHT];
};

// ========== Payloads ==========
struct RayPayload
{
    float3 colour;        // Accumulated colour
    float attenuation;    // How much colour to contribute
    uint depth;           // Recursion depth
    bool isInside;        // For refraction: inside or outside object
};

// Constants
static const uint FLOAT_SIZE = 4;
static const uint VERTEX_STRIDE = (3 + 2) * FLOAT_SIZE; // normal + uv

struct VertexAttributes
{
    float3 normal;
    float2 uv;
};

// Helper to fetch a vertex from the buffer
VertexAttributes GetVertex(uint vertexIndex)
{
    uint address = vertexIndex * VERTEX_STRIDE;
    VertexAttributes v;

    v.normal   = normalize(asfloat(vertices.Load3(address))); 
    address += 3 * FLOAT_SIZE;

    v.uv       = asfloat(vertices.Load2(address));
    return v;
}

// Interpolate vertex attributes using barycentrics
VertexAttributes InterpolateAttributes(uint primitiveIndex, float3 barycentrics)
{
    uint baseVertex = primitiveIndex * 3;
    VertexAttributes v[3];
    v[0] = GetVertex(baseVertex + 0);
    v[1] = GetVertex(baseVertex + 1);
    v[2] = GetVertex(baseVertex + 2);

    VertexAttributes interpolated;
    interpolated.normal   = normalize(v[0].normal * barycentrics.x + v[1].normal * barycentrics.y + v[2].normal * barycentrics.z);
    interpolated.uv       = v[0].uv * barycentrics.x + v[1].uv * barycentrics.y + v[2].uv * barycentrics.z;

    return interpolated;
}