#include "Material.h"
using namespace DirectX;
using Microsoft::WRL::ComPtr;

// Constructor
Material::Material(
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline,
    DirectX::XMFLOAT3 tint,
    DirectX::XMFLOAT2 scale,
    DirectX::XMFLOAT2 offset,
    unsigned int albedo,
    unsigned int normal,
    unsigned int roughness,
    unsigned int metalness)
    :
    pipelineState(pipeline),
    colorTint(tint),
    uvScale(scale),
    uvOffset(offset),
    albedoIndex(albedo),
    normalIndex(normal),
    roughnessIndex(roughness),
    metalnessIndex(metalness)
{
}


// --------
// Getters
// --------
XMFLOAT3 Material::GetColorTint() { return colorTint; }
XMFLOAT2 Material::GetUVScale() { return uvScale; }
XMFLOAT2 Material::GetUVOffset() { return uvOffset; }
ComPtr<ID3D12PipelineState> Material::GetPipelineState() { return pipelineState; }

unsigned int Material::GetAlbedoIndex() { return albedoIndex; }
unsigned int Material::GetNormalIndex() { return normalIndex; }
unsigned int Material::GetMetalnessIndex() { return metalnessIndex; }
unsigned int Material::GetRoughnessIndex() { return roughnessIndex; }


// --------
// Setters
// --------
void Material::SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState) { this->pipelineState = pipelineState; }
void Material::SetColorTint(XMFLOAT3 tint) { colorTint = tint; }
void Material::SetUVScale(XMFLOAT2 scale) { uvScale = scale; }
void Material::SetUVOffset(XMFLOAT2 offset) { uvOffset = offset; }