#pragma once

#include "Input.h"
#include <DirectXMath.h>
#include "Transform.h"

class Camera
{
private:
	// Variables
	Transform transform;

	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	float fov;
	float nearClip;
	float farClip;
	float movmentSpeed;
	float mouseSpeed;
	
public:
	Camera(DirectX::XMFLOAT3 position, float movmentSpeed, float mouseSpeed, float fov, float aspectRatio);


	//--------
	// Methods
	// -------
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);
	void Update(float deltaTime);

	//--------
	// Getters
	// -------
	Transform GetTransform();
	DirectX::XMFLOAT4X4 ViewMatrix();
	DirectX::XMFLOAT4X4 ProjectionMatrix();
};
