#ifndef __SHADERSTRUCTS__ 
#define __SHADERSTRUCTS__
#include "PBRFunctions.hlsli"



//----------------
// Lighting Struct
//----------------

#define MAX_SPECULAR_EXPONENT 256.0f

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

struct Light
{
    int Type;
    float3 Direction;    
    float Range;
    float3 Position;    
    float Intensity;
    float3 Color; 
    float SpotInnerAngle;
    float SpotOuterAngle;
    float2 Padding; 
};


//-----------------------
// Normal Mapping Methods
//-----------------------

float3 ComputeNormalMap(float3 normal, float3 tangent, Texture2D normalMap, SamplerState samplerState, float2 uv)
{
    float3 unpackedNormal = normalMap.Sample(samplerState, uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    
    float3 N = normalize(normal); // Must be normalized here or before
    float3 T = normalize(tangent); // Must be normalized here or before
    T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    
    return mul(unpackedNormal, TBN); // Note multiplication order!
}


//-----------------
// Lighting Methods
//-----------------

float3 Diffuse(float3 lightDirection, float3 normal, float3 surfaceColor)
{
    // Normalize light direction
    lightDirection = normalize(lightDirection);
    
    // Calculate lambertian reflectance with dot product
    float lambertianReflection = saturate(dot(normal, lightDirection));
    
    return surfaceColor * lambertianReflection;
}

float Phong(float3 lightDirection, float3 normal, float3 cameraPosition, float3 pixelWorldPosition, float roughness)
{
    
    lightDirection = normalize(lightDirection);
    
    float3 V = normalize(cameraPosition - pixelWorldPosition);
    float3 R = reflect(-lightDirection, normal);
    
    return pow(max(dot(V, R), 0), (1 - roughness) * MAX_SPECULAR_EXPONENT);
}

float Attenuation(Light light, float3 pixelWorldPosition)
{
    float dist = distance(light.Position, pixelWorldPosition);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}


// ------------------
// Light Type Methods
// ------------------

float3 ComputeDirectionalLighting(Light light, float3 normal, float3 surfaceColor, float3 cameraPosition, float3 pixelWorldPosition, float roughness, float3 specularColor, float metalness)
{
    // Compute light direction 
    float3 lightDirection = normalize(-light.Direction);

    float3 fOut;
    
    // Compute diffuse and specular 
    float3 diffuse = Diffuse(lightDirection, normal, surfaceColor);
    float3 specular = MicrofacetBRDF(normal, lightDirection, cameraPosition, roughness, specularColor, fOut);
    
    // Calculate diffuse with energy conservation, including cutting diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, specular, metalness);
    
    return balancedDiff + specular;
}

float3 ComputePointLighting(Light light, float3 normal, float3 surfaceColor, float3 cameraPosition, float3 pixelWorldPosition, float roughness, float3 specularColor, float metalness)
{
    // Compute light direction to pixel
    float3 lightDirectionToPixel = normalize(light.Position - pixelWorldPosition);

    float3 fOut;

    // Compute diffuse and specular
    float3 diffuse = Diffuse(lightDirectionToPixel, normal, surfaceColor);
    float3 specular = MicrofacetBRDF(normal, lightDirectionToPixel, cameraPosition, roughness, specularColor, fOut);
    
    // Apply attenuation 
    float attenuation = Attenuation(light, pixelWorldPosition);
    
    // Calculate diffuse with energy conservation, including cutting diffuse for metals
    float3 balancedDiff = DiffuseEnergyConserve(diffuse, specular, metalness);
    
    return (balancedDiff + specular) * attenuation;

}

float3 ComputeSpotLighting(Light light, float3 normal, float3 surfaceColor, float3 cameraPosition, float3 pixelWorldPosition, float roughness, float3 specularColor, float metalness)
{
    // Compute light direction 
    float3 lightDirection = normalize(-light.Direction);
    
    // Compute light direction to pixel
    float3 lightDirectionToPixel = normalize(light.Position - pixelWorldPosition);
    
    // Get cos(angle) between pixel and light direction
    float pixelAngle = saturate(dot(lightDirectionToPixel, lightDirection));
    
    // Get cosines of angles and calculate range
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float falloffRange = cosOuter - cosInner;
    
    // Linear falloff over the range, clamp 0-1, apply to light calc
    float spotTerm = saturate((cosOuter - pixelAngle) / falloffRange);
    
    return ComputePointLighting(light, normal, surfaceColor, cameraPosition, pixelWorldPosition, roughness, specularColor, metalness) * spotTerm;
}


// --------------------
// Main Lighting Method
// --------------------

float3 ComputeLighting(Light light, float3 normal, float3 surfaceColor, float3 cameraPosition, float3 pixelWorldPosition, float roughness, float3 specularColor, float metalness)
{
    float3 result = 0.0f;

    // Compute the lighting for whatever type of light it is
    if (light.Type == LIGHT_TYPE_DIRECTIONAL)
    {
        result = ComputeDirectionalLighting(light, normal, surfaceColor, cameraPosition, pixelWorldPosition, roughness, specularColor, metalness);
    }
    else if (light.Type == LIGHT_TYPE_POINT)
    {
        result = ComputePointLighting(light, normal, surfaceColor, cameraPosition, pixelWorldPosition, roughness, specularColor, metalness);
    }
    else if (light.Type == LIGHT_TYPE_SPOT)
    {
        result = ComputeSpotLighting(light, normal, surfaceColor, cameraPosition, pixelWorldPosition, roughness, specularColor, metalness);
    }

    return result * light.Color * light.Intensity;
}


#endif