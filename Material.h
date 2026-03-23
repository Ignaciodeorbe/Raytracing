#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>
#include <unordered_map>
#include "Camera.h"
#include "Transform.h"

class Material
{
private:

	DirectX::XMFLOAT3 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

	unsigned int albedoIndex;
	unsigned int normalIndex;
	unsigned int metalnessIndex;
	unsigned int roughnessIndex;

public:

	Material(
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline,
		DirectX::XMFLOAT3 tint,
		DirectX::XMFLOAT2 scale,
		DirectX::XMFLOAT2 offset,
		unsigned int albedo,
		unsigned int normal,
		unsigned int roughness,
		unsigned int metalness);

	// --------
	// Getters
	// --------
	DirectX::XMFLOAT3 GetColorTint();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState();

	unsigned int GetAlbedoIndex();
	unsigned int GetNormalIndex();
	unsigned int GetMetalnessIndex();
	unsigned int GetRoughnessIndex();

	// --------
	// Setters
	// --------
	void SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState);
	void SetColorTint(DirectX::XMFLOAT3 tint);
	void SetUVScale(DirectX::XMFLOAT2 scale);
	void SetUVOffset(DirectX::XMFLOAT2 offset);
};