#pragma once
#include <DirectXMath.h>


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