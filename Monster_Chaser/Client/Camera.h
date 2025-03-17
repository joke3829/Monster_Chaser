#pragma once

#include "stdafx.h"
#include "GameObject.h"

extern DXResources g_DxResource;

struct CB_CAMERA_INFO {
	XMFLOAT4X4 xmf4x4ViewProj;
	XMFLOAT4X4 xmf4x4InverseViewProj;
	XMFLOAT3 xmf3Eye;
};

class CCamera {
public:
	void Setup(int nRootParameterIndex);

	void Rotate(int cxDelta, int cyDelta);
	void Move(int arrow, float fElapsedTime);

	void UpdateViewMatrix();
	void SetShaderVariable();

	void SetTarget(CGameObject* target);
	void SetThirdPersonMode(bool bThirdPerson);
	void SetCameraLength(float fLength) { m_fCameraLength = fLength; }

protected:
	bool m_bThirdPerson = false;
	CGameObject* m_pTarget = nullptr;


	ComPtr<ID3D12Resource> m_pd3dCameraBuffer{};
	CB_CAMERA_INFO* m_pCameraInfo{};

	XMFLOAT3 m_xmf3Eye{ 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_xmf3At;
	XMFLOAT3 m_xmf3Up{ 0.0f, 1.0f, 0.0f };

	XMFLOAT3 m_xmf3Dir{ 0.0f, 0.0f, 1.0f };
	XMFLOAT3 m_xmf3Offset{ 0.0f, 0.0f, -1.0f };

	XMFLOAT4X4 m_xmf4x4View;
	XMFLOAT4X4 m_xmf4x4Proj;
	
	float m_fFOV = 60.0f;
	float m_fAspect = 960.0f / 540.0f;
	float m_fNear = 0.1f;
	float m_fFar = 1000.0f;

	float m_fLimitcy{};
	float m_fCameraLength{ 30.0f };		// 카메라 거리

	int m_nRootParameterIndex{};
};

