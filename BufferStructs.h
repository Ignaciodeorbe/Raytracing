#pragma once
#include <DirectXMath.h>
#include "Lights.h"



struct VertexShaderExternalData {
	DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 worldInverseTranspose;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

};

struct PixelShaderExternalData
{
	unsigned int albedoIndex;
	unsigned int normalIndex;
	unsigned int roughnessIndex;
	unsigned int metalnessIndex;

	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;

	DirectX::XMFLOAT3 cameraPosition;

	int lightCount;
	Light lights[6];
};

// Root constants for bindless resources
struct RayTracingDrawData
{
	unsigned int SceneDataConstantBufferIndex;
	unsigned int EntityDataDescriptorIndex;
	unsigned int SceneTLASDescriptorIndex;
	unsigned int OutputUAVDescriptorIndex;
};
// Overall scene data for ray tracing (constant buffer)
struct RayTracingSceneData
{
	DirectX::XMFLOAT4X4 InverseViewProjection;
	DirectX::XMFLOAT3 CameraPosition;
	unsigned int RaysPerPixel;
};
// Per-entity information (geom, material, etc.)
// - Multiple sets of these will be stored in a structured buffer
// - Materials *could* be separated out into their own buffer
// to cut down on data repetition
struct RayTracingEntityData
{
	DirectX::XMFLOAT4 Color;
	unsigned int VertexBufferDescriptorIndex;
	unsigned int IndexBufferDescriptorIndex;
	float pad[2];
};
