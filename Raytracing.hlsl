
#define PI 3.141592654f

// === Structs ===

// Layout of data in the vertex buffer
struct Vertex
{
    float3 localPosition;
    float2 uv;
    float3 normal;
    float3 tangent;
};

// Payload for rays (data that is "sent along" with each ray during raytrace)
// Note: This should be as small as possible, and must match our C++ size definition
struct RayPayload
{
    float3 color;
    uint RecursionDepth;
    uint RayPerPixelIndex;
};

struct ShadowPayload
{
    bool Hit;
};

// Note: We'll be using the built-in BuiltInTriangleIntersectionAttributes struct
// for triangle attributes, so no need to define our own.  It contains a single float2.

struct SceneData
{
    matrix InverseViewProjection;
    float3 CameraPosition;
    uint RaysPerPixel;
};
struct EntityData
{
    float4 Color;
    uint VertexBufferDescriptorIndex;
    uint IndexBufferDescriptorIndex;
    float pad[2];
};



// === Constant buffers ===

cbuffer DrawData : register(b0)
{
    uint SceneDataConstantBufferIndex;
    uint EntityDataDescriptorIndex;
    uint SceneTLASDescriptorIndex;
    uint OutputUAVDescriptorIndex;
};

// Random functions
float rand(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

float2 rand2(float2 uv)
{
    return float2(
		rand(uv),
		rand(uv.yx));
}

float3 RandomVector(float u0, float u1)
{
    float a = u0 * 2 - 1;
    float b = sqrt(1 - a * a);
    float phi = 2.0f * PI * u1;
    float x = b * cos(phi);
    float y = b * sin(phi);
    float z = a;
    return float3(x, y, z);
}

float3 RandomCosineWeightedHemisphere(float u0, float u1, float3 unitNormal)
{
    float a = u0 * 2 - 1;
    float b = sqrt(1 - a * a);
    float phi = 2.0f * PI * u1;
    float x = unitNormal.x + b * cos(phi);
    float y = unitNormal.y + b * sin(phi);
    float z = unitNormal.z + a;
    return float3(x, y, z);
}



// === Helpers ===

// Barycentric interpolation of data from the triangle's vertices
Vertex InterpolateVertices(uint triangleIndex, float2 barycentrics)
{
	// Get the data for this entity
    StructuredBuffer<EntityData> ed =
		ResourceDescriptorHeap[EntityDataDescriptorIndex];
    EntityData thisEntity = ed[InstanceIndex()];
	
	// Get the geometry buffers
    StructuredBuffer<uint> IndexBuffer =
		ResourceDescriptorHeap[thisEntity.IndexBufferDescriptorIndex];
    StructuredBuffer<Vertex> VertexBuffer =
		ResourceDescriptorHeap[thisEntity.VertexBufferDescriptorIndex];
	
	// Grab the 3 indices for this triangle
    uint firstIndex = triangleIndex * 3;
    uint indices[3];
    indices[0] = IndexBuffer[firstIndex + 0];
    indices[1] = IndexBuffer[firstIndex + 1];
    indices[2] = IndexBuffer[firstIndex + 2];

	// Grab the 3 corresponding vertices
    Vertex verts[3];
    verts[0] = VertexBuffer[indices[0]];
    verts[1] = VertexBuffer[indices[1]];
    verts[2] = VertexBuffer[indices[2]];
	
	// Calculate the barycentric data for vertex interpolation
    float3 barycentricData = float3(
		1.0f - barycentrics.x - barycentrics.y,
		barycentrics.x,
		barycentrics.y);
	
	// Loop through the barycentric data and interpolate
    Vertex finalVert = (Vertex) 0;
    for (uint i = 0; i < 3; i++)
    {
        finalVert.localPosition += verts[i].localPosition * barycentricData[i];
        finalVert.uv += verts[i].uv * barycentricData[i];
        finalVert.normal += verts[i].normal * barycentricData[i];
        finalVert.tangent += verts[i].tangent * barycentricData[i];
    }
    return finalVert;
}


// Calculates an origin and direction from the camera for specific pixel indices
RayDesc CalcRayFromCamera(float2 rayIndices, float3 camPos, float4x4 invVP)
{
	// Offset to the middle of the pixel
    float2 pixel = rayIndices + 0.5f;
    float2 screenPos = pixel / DispatchRaysDimensions().xy * 2.0f - 1.0f;
    screenPos.y = -screenPos.y;

	// Unproject the coords
    float4 worldPos = mul(invVP, float4(screenPos, 0, 1));
    worldPos.xyz /= worldPos.w;

	// Set up the ray
    RayDesc ray;
    ray.Origin = camPos.xyz;
    ray.Direction = normalize(worldPos.xyz - ray.Origin);
    ray.TMin = 0.01f;
    ray.TMax = 1000.0f;
    return ray;
}


// === Shaders ===

// Ray generation shader - Launched once for each ray we want to generate
// (which is generally once per pixel of our output texture)
[shader("raygeneration")]
void RayGen()
{
	
	// Grab the constant buffer
    ConstantBuffer<SceneData> cb =
		ResourceDescriptorHeap[SceneDataConstantBufferIndex];
	
	// Get the ray indices
    uint2 rayIndices = DispatchRaysIndex().xy;
	
    float3 totalColor = float3(0, 0, 0);
	
    for (int n = 0; n < cb.RaysPerPixel; n++)
    {
        float2 adjustedIndices = (float2) rayIndices;
        float ray01 = (float) n / cb.RaysPerPixel;
        adjustedIndices += rand2(rayIndices.xy * ray01);
		
		// Calculate the ray from the camera through a particular
		// pixel of the output buffer using this shader's indices
        RayDesc ray = CalcRayFromCamera(
		rayIndices,
		cb.CameraPosition,
		cb.InverseViewProjection);

		// Set up the payload for the ray
		// This initializes the struct to all zeros
        RayPayload payload = (RayPayload) 0;
        payload.color = float3(1, 1, 1);
        payload.RecursionDepth = 0;
        payload.RayPerPixelIndex = n;
	
		// Perform the ray trace for this ray
        RaytracingAccelerationStructure SceneTLAS = ResourceDescriptorHeap[SceneTLASDescriptorIndex];
        TraceRay(
			SceneTLAS,
			RAY_FLAG_NONE,
			0xFF,
			0,
			0,
			0,
			ray,
			payload);
		
        totalColor += payload.color;
    }

	

	// Set the final color of the buffer (gamma corrected)
    RWTexture2D<float4> OutputColor =
		ResourceDescriptorHeap[OutputUAVDescriptorIndex];
	
	// Set the final color of the buffer
    OutputColor[rayIndices] = float4(pow(totalColor / cb.RaysPerPixel, 1.0f / 2.2f), 1);
}


// Miss shader - What happens if the ray doesn't hit anything?
[shader("miss")]
void Miss(inout RayPayload payload)
{
	// Nothing was hit, so return black for now.
	// Ideally this is where we would do skybox stuff!
    float3 upColor = float3(0.3f, 0.5f, 0.95f);
    float3 downColor = float3(1, 1, 1);

	// Interpolate based on the direction of the ray
    float interpolation = dot(normalize(WorldRayDirection()), float3(0, 1, 0)) * 0.5f + 0.5f;
    float3 skyColor = lerp(downColor, upColor, interpolation);
	
	// Alter the payload color by the sky color
    payload.color *= skyColor;
}


[shader("miss")]
void ShadowMiss(inout ShadowPayload payload)
{
    payload.Hit = false;
}

[shader("closesthit")]
void ShadowClosestHit(inout ShadowPayload payload, BuiltInTriangleIntersectionAttributes attr)
{
    payload.Hit = true; 
}



// Closest hit shader - Runs the first time a ray hits anything
[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes hitAttributes)
{
	// If max recursion is reached then the light source wasnt hit
    if (payload.RecursionDepth == 10)
    {
        payload.color = float3(0, 0, 0);
        return;
    }
	//// Get the interpolated vertex data
	//Vertex interpolatedVert = InterpolateVertices(
	//	PrimitiveIndex(), 
	//	hitAttributes.barycentrics);
	
	// Get the data for this entity
    StructuredBuffer<EntityData> entityDataBuffer =
		ResourceDescriptorHeap[EntityDataDescriptorIndex];
    EntityData thisEntity = entityDataBuffer[InstanceIndex()];

	// Use the resulting data to set the final color
	// Note: Here is where we would do actual shading!
    payload.color *= thisEntity.Color.rgb;
	
    Vertex hit = InterpolateVertices(PrimitiveIndex(), hitAttributes.barycentrics);
    float3 normal_WS = normalize(mul(hit.normal, (float3x3) ObjectToWorld4x3()));
	
	

    // sun direction
    float3 directionToSun = normalize(float3(1, 1, -1));

    float NdotL = dot(normal_WS, directionToSun);

    if (NdotL > 0)
    {
        float3 hitPos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();

        // shadow ray
        RayDesc shadowRay;
        shadowRay.Direction = directionToSun;
        shadowRay.Origin = hitPos + normal_WS * 0.01f;
        shadowRay.TMin = 0.001f;
        shadowRay.TMax = 1000.0f;

        // Shadow payload
        ShadowPayload shadowPayload;
        shadowPayload.Hit = false;

        // Trace shadow ray
        RaytracingAccelerationStructure SceneTLAS = ResourceDescriptorHeap[SceneTLASDescriptorIndex];
        TraceRay(
        SceneTLAS,
        RAY_FLAG_NONE,
        0xFF,
        1, 
        0,
        1, 
        shadowRay,
        shadowPayload);

        if (!shadowPayload.Hit)
        {
            payload.color *= NdotL;
        }
    }
    
    

	
    float2 pixelUV = (float2) DispatchRaysIndex().xy / DispatchRaysDimensions().xy;
    float2 rng = rand2(pixelUV * (payload.RecursionDepth + 1) + payload.RayPerPixelIndex + RayTCurrent());
	
    float3 refl = reflect(WorldRayDirection(), normal_WS);
    float3 randomBounce = RandomCosineWeightedHemisphere(rand(rng), rand(rng.yx), normal_WS);
    float3 dir = normalize(lerp(refl, randomBounce, thisEntity.Color.a));
	
    RayDesc ray;
    ray.Origin = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    ray.Direction = dir;
    ray.TMin = 0.0001f;
    ray.TMax = 1000.0f;
	
	// Recursive ray trace
    payload.RecursionDepth++;
    RaytracingAccelerationStructure SceneTLAS = ResourceDescriptorHeap[SceneTLASDescriptorIndex];
    TraceRay(
		SceneTLAS,
		RAY_FLAG_NONE,
		0xFF, 0, 0, 0, // Mask and offsets
		ray,
		payload);
}