#include "Camera.hpp"

WMTS::Camera::Camera()
{

}

DirectX::XMMATRIX WMTS::Camera::GetViewMatrix()
{
    DirectX::XMVECTOR posVec = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR lookVec = DirectX::XMLoadFloat3(&look);
    DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);

    return DirectX::XMMatrixLookAtLH(posVec, lookVec, upVec);
}

DirectX::XMMATRIX WMTS::Camera::GetProjectionMatrix()
{
    return DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
}

void WMTS::Camera::MoveBackward(float distance)
{
    MoveFoward(-distance);
}

void WMTS::Camera::MoveFoward(float distance)
{
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&look);
    DirectX::XMVECTOR scaledDir = DirectX::XMVectorScale(dir, distance);
    DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(pos, scaledDir);
    DirectX::XMStoreFloat3(&position, newPos);
}

void WMTS::Camera::MoveRight(float distance)
{
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&right);
    DirectX::XMVECTOR scaledDir = DirectX::XMVectorScale(dir, distance);
    DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(pos, scaledDir);
    DirectX::XMStoreFloat3(&position, newPos);
}

void WMTS::Camera::MoveLeft(float distance)
{
    MoveRight(-distance);
}

void WMTS::Camera::MoveUp(float distance)
{
    // Load the XMFLOAT3 position and up into XMVECTORs for computation
    DirectX::XMVECTOR posVector = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR upVector = DirectX::XMLoadFloat3(&up);

    // Scale the up direction by the desired distance
    DirectX::XMVECTOR scaledUp = DirectX::XMVectorScale(upVector, distance);

    // Add the scaled direction to the current position
    DirectX::XMVECTOR newPos = DirectX::XMVectorAdd(posVector, scaledUp);

    // Store the result back into the XMFLOAT3 position
    DirectX::XMStoreFloat3(&position, newPos);
}

void WMTS::Camera::MoveDown(float distance)
{
    MoveUp(-distance);
}

void WMTS::Camera::RotateY(float angle)
{
    yaw += angle;
    if (yaw > 360.0f) {
        yaw -= 360.0f;
    }
    else if (yaw < 0.0f) {
        yaw += 360.0f;
    }

}

void WMTS::Camera::RotateX(float angle)
{
    pitch += angle;
    pitch = std::clamp(pitch, -89.0f, 89.0f);
}

void WMTS::Camera::RotateZ(float angle)
{
    roll += angle;
    roll = std::clamp(roll, -360.0f, 360.0f);
}

void WMTS::Camera::UpdateCameraVectors()
{
    // Calculate the new front vector
    DirectX::XMFLOAT3 front{};
    front.x = cos(DirectX::XMConvertToRadians(yaw)) * cos(DirectX::XMConvertToRadians(pitch));
    front.y = sin(DirectX::XMConvertToRadians(pitch));
    front.z = sin(DirectX::XMConvertToRadians(yaw)) * cos(DirectX::XMConvertToRadians(pitch));

    DirectX::XMVECTOR frontVec = DirectX::XMLoadFloat3(&front);
    DirectX::XMVECTOR lookVec = DirectX::XMVector3Normalize(frontVec);

    // Also re-calculate the right and up vector
    DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMVECTOR rightVec = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(lookVec, worldUp));
    DirectX::XMVECTOR upVec = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(rightVec, lookVec));

    // New code to account for roll
    DirectX::XMMATRIX rollMatrix = DirectX::XMMatrixRotationAxis(lookVec, DirectX::XMConvertToRadians(roll));
    upVec = DirectX::XMVector3TransformNormal(upVec, rollMatrix);
    rightVec = DirectX::XMVector3TransformNormal(rightVec, rollMatrix);

    // Store results back into your class members (if they are of type XMFLOAT3)
    DirectX::XMStoreFloat3(&look, lookVec);
    DirectX::XMStoreFloat3(&right, rightVec);
    DirectX::XMStoreFloat3(&up, upVec);
}