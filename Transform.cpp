#include "Transform.h"

using namespace DirectX;

// Setting default values in constructor
Transform::Transform()
	: position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f), dirty(true),
	rightward(1.0f, 0.0f, 0.0f), upward(0.0f, 1.0f, 0.0f), forward(0.0f, 0.0f, 1.0f)
{
	// Storing the identity matrix as the world matrix and inverse matrix
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());

}

/// <summary>
/// Updates the world martix based on any tranformations made to it
/// </summary>
void Transform::UpdateWorldMatrix()
{
	if (dirty)
	{
		// Using SRT to apply transformations
		XMMATRIX world = XMMatrixScaling(scale.x, scale.y, scale.z) * 
			XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) * 
			XMMatrixTranslation(position.x, position.y, position.z);

		// Storing the updated world matrix and its inverse transpose
		XMStoreFloat4x4(&worldMatrix, world);
		XMStoreFloat4x4(&worldInverseTranspose, XMMatrixTranspose(XMMatrixInverse(nullptr, world)));

		// Updating directional vectors
		UpdateDirectionalVectors();

		dirty = false;
	}
}

/// <summary>
/// Updating directional vectors instead of updating them in the getters
/// </summary>
void Transform::UpdateDirectionalVectors()
{
	// Making a rotation quaternion for current rotation
	XMVECTOR rotationQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	// Updating directional vectors
	XMVECTOR rightVector = XMVector3Rotate(XMVectorSet(1, 0, 0, 0), rotationQuat);
	XMVECTOR upVector = XMVector3Rotate(XMVectorSet(0, 1, 0, 0), rotationQuat);
	XMVECTOR forwardVector = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), rotationQuat);

	// Storing vectors inside fields
	XMStoreFloat3(&rightward, rightVector);
	XMStoreFloat3(&upward, upVector);
	XMStoreFloat3(&forward, forwardVector);
}

//--------
// Setters
// -------

// Position setters
void Transform::SetPosition(float x, float y, float z) { position = { x, y, z }; dirty = true; }
void Transform::SetPosition(XMFLOAT3 pos) { position = pos; dirty = true; }

// Rotation setters
void Transform::SetRotation(float pitch, float yaw, float roll) { rotation = { pitch, yaw, roll }; dirty = true; }
void Transform::SetRotation(XMFLOAT3 rot) { rotation = rot; dirty = true;}

// Scale setters
void Transform::SetScale(float x, float y, float z) { scale = { x, y, z }; dirty = true; }
void Transform::SetScale(XMFLOAT3 scl) { scale = scl; dirty = true; }


//--------
// Getters
// -------

// Transform getters
XMFLOAT3 Transform::GetPosition() { return position; }
XMFLOAT3 Transform::GetPitchYawRoll() { return rotation; }
XMFLOAT3 Transform::GetScale() { return scale; }

// Matrix getters
XMFLOAT4X4 Transform::GetWorldMatrix() 
{ 
	UpdateWorldMatrix();
	return worldMatrix; 
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTranspose() { return worldInverseTranspose; }

// Directional vectors getters
XMFLOAT3 Transform::GetRight() { return rightward; }
XMFLOAT3 Transform::GetUp() { return upward; }
XMFLOAT3 Transform::GetForward() { return forward; }


//-------------
// Transformers
//-------------

// Position transformers
void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	dirty = true;
}

void Transform::MoveAbsolute(XMFLOAT3 pos)
{
	MoveAbsolute(pos.x, pos.y, pos.z);
}

// Rotation transformers
void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;
	dirty = true;
	UpdateDirectionalVectors();

}

void Transform::Rotate(XMFLOAT3 rot)
{
	Rotate(rot.x, rot.y, rot.z);
}

// Scale transformers
void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	dirty = true;
}

void Transform::Scale(XMFLOAT3 scl)
{
	Scale(scl.x, scl.y, scl.z);
}

void Transform::MoveRelative(float x, float y, float z)
{
	// Making a rotation quaternion for current rotation
	XMVECTOR rotationQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	// Getting the absolute direction and rotating it by the current rotation
	XMVECTOR relativeTransform = XMVector3Rotate(XMVectorSet(x, y, z, 0), rotationQuat);

	// Storing the relative transform as a float3 so that it can be applied to current position
	XMFLOAT3 move;
	XMStoreFloat3(&move, relativeTransform);

	position.x += move.x;
	position.y += move.y;
	position.z += move.z;

	dirty = true;
}

void Transform::MoveRelative(XMFLOAT3 offset)
{
	// Calling the other move relative method for it to handle the work
	MoveRelative(offset.x, offset.y, offset.z);
}




