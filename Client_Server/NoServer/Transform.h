//-----------------------------------------------------------------------------
// File: Transform.h
// 오브젝트의 위치, 회전, 스케일과 같은 변환 데이터를 관리하는 클래스
// 
// 추가적인 기능은 개발 진행에 따라 확장 예정
//-----------------------------------------------------------------------------
#include "Component.h"
#include "stdafx.h"

class Transform : public Component
{
public:
    Transform()
        : m_Position(0.0f, 0.0f, 0.0f),
        m_Rotation(0.0f, 0.0f, 0.0f),
        m_Scale(1.0f, 1.0f, 1.0f),
        m_IsDirty(true) //true면 업데이트하고 반환
    {
        UpdateWorldMatrix();
    }

    virtual ~Transform() = default;

private:
    XMFLOAT3 m_Position;
    XMFLOAT3 m_Rotation;
    XMFLOAT3 m_Scale;

    XMFLOAT4X4 m_WorldMatrix;
    XMFLOAT4X4 m_ParentMatrix;
    bool m_IsDirty;

public:
    const XMFLOAT3& GetPosition() const { return m_Position; }
    const XMFLOAT3& GetRotation() const { return m_Rotation; }
    const XMFLOAT3& GetScale() const { return m_Scale; }
    const XMFLOAT4X4& GetWorldMatrix()
    {
        if (m_IsDirty) //true 라면 업데이트하고 반환
            UpdateWorldMatrix();
        return m_WorldMatrix;
    }

    void SetPosition(const XMFLOAT3& position)
    {
        m_Position = position;
        m_IsDirty = true;
    }

    void SetRotation(const XMFLOAT3& rotation)
    {
        m_Rotation = rotation;
        m_IsDirty = true;
    }

    void SetScale(const XMFLOAT3& scale)
    {
        m_Scale = scale;
        m_IsDirty = true;
    }

    void Translate(const XMFLOAT3& delta)
    {
        m_Position.x += delta.x;
        m_Position.y += delta.y;
        m_Position.z += delta.z;
        m_IsDirty = true;
    }

    void Rotate(const XMFLOAT3& delta)
    {
        m_Rotation.x += delta.x;
        m_Rotation.y += delta.y;
        m_Rotation.z += delta.z;
        m_IsDirty = true;
    }

    void Scale(const XMFLOAT3& factor)
    {
        m_Scale.x *= factor.x;
        m_Scale.y *= factor.y;
        m_Scale.z *= factor.z;
        m_IsDirty = true;
    }

    void SetPMatrix(XMFLOAT4X4& Parent)
    {
        m_ParentMatrix = Parent;
    }

    XMFLOAT3 GetPosition() { return m_Position; }
    XMFLOAT3 GetLook()
    {
        XMFLOAT3 look(m_WorldMatrix._31, m_WorldMatrix._32, m_WorldMatrix._33);
        return look;
    }

private:
    void UpdateWorldMatrix()
    {
        XMMATRIX scaleMatrix = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
        XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
        XMMATRIX translationMatrix = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

        XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

        XMStoreFloat4x4(&m_WorldMatrix, worldMatrix);
        m_IsDirty = false; //업데이트하고 false로 바꿔주기
    }
};
