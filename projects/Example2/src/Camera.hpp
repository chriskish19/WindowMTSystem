#pragma once
#include <DirectXMath.h>
#include <algorithm>


namespace WMTS{
    class Camera {
    public:
        Camera();

        DirectX::XMMATRIX GetViewMatrix();
        DirectX::XMMATRIX GetProjectionMatrix();

        void MoveBackward(float distance);
        void MoveFoward(float distance);
        void MoveRight(float distance);
        void MoveLeft(float distance);
        void MoveUp(float distance);
        void MoveDown(float distance);
        void RotateY(float angle);
        void RotateX(float angle);
        void RotateZ(float angle);

        void UpdateCameraVectors();
    
        float GetMovementSpeed() { return movementSpeed; }
        float GetRotateSpeed() { return rotateSpeed; }
        void SetAspectRatio(float ratio) { aspect = ratio; }
    private:  
        

        DirectX::XMFLOAT3 position{ 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 up{ 0.0f, 1.0f, 0.0f };
        DirectX::XMFLOAT3 right{ 1.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 look{ 0.0f, 0.0f, 1.0f };

        const float nearZ{0.1f};
        const float farZ{1000.0f};
        float aspect{};
        const float fovY{ DirectX::XM_PIDIV4 };
        float pitch{0.0f}, yaw{0.0f};
        const float movementSpeed{ 5.0f };
        const float rotateSpeed{ 20.0f };
        float roll{};
    };
}