#include "Camera.h"

using namespace DirectX;

Camera::Camera(XMFLOAT3 position, float movmentSpeed, float mouseSpeed, float fov, float aspectRatio)
	: movmentSpeed(movmentSpeed), mouseSpeed(mouseSpeed), fov(fov), nearClip(0.1f), farClip(100.0f)
{
	// Initalizing intial position
	transform.SetPosition(position);

	// Initalizing view and projection matrix
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

/// <summary>
/// Update view matrix, should happen once per frame
/// </summary>
void Camera::UpdateViewMatrix()
{
	// Cameras position, forward and the global upward direcions
	XMFLOAT3 pos = transform.GetPosition();
	XMFLOAT3 forward = transform.GetForward();
	XMFLOAT3 worldUp = { 0.0f, 1.0f, 0.0f };

	// Getting and storing the view matrix
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&pos), XMLoadFloat3(&forward), XMLoadFloat3(&worldUp));
	XMStoreFloat4x4(&viewMatrix, view);

}

/// <summary>
/// Updates the projection matrix, should happen whenever screen is resized
/// </summary>
/// <param name="aspectRatio"> Aspect ratio of the window</param>
void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	// Getting and storing the projection matrix
	XMMATRIX projection = XMMatrixPerspectiveFovLH(fov, aspectRatio, nearClip, farClip);
	XMStoreFloat4x4(&projectionMatrix, projection);

}

//--------
// Getters
// -------
Transform Camera::GetTransform() { return transform; }
XMFLOAT4X4 Camera::ViewMatrix() { return viewMatrix; }
XMFLOAT4X4 Camera::ProjectionMatrix() { return projectionMatrix; }


void Camera::Update(float deltaTime)
{
	// Camera movement along x,y,z axis
	if (Input::KeyDown('W'))
	{
		transform.MoveRelative(0.0f, 0.0f, movmentSpeed * deltaTime);
	}
	if (Input::KeyDown('S'))
	{
		transform.MoveRelative(0.0f, 0.0f, -movmentSpeed * deltaTime);
	}
	if (Input::KeyDown('D'))
	{
		transform.MoveRelative(movmentSpeed * deltaTime, 0.0f, 0.0f);
	}
	if (Input::KeyDown('A')) 
	{ 
		transform.MoveRelative(-movmentSpeed * deltaTime, 0.0f, 0.0f);
	}
	if (Input::KeyDown(VK_SHIFT)) 
	{ 
		transform.MoveRelative(0.0f, movmentSpeed * deltaTime, 0.0f);
	}
	if (Input::KeyDown(VK_CONTROL)) 
	{
		transform.MoveRelative(0.0f, -movmentSpeed * deltaTime, 0.0f);
	}

	// Camera mouse movement controls
	if (Input::MouseLeftDown())
	{
		float mouseX = Input::GetMouseXDelta() * mouseSpeed;
		float mouseY = Input::GetMouseYDelta() * mouseSpeed;
		 
		transform.Rotate(mouseY, mouseX, 0.0f);	
	}

	UpdateViewMatrix();
}