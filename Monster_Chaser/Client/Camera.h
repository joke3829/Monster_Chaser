#pragma once

#include "stdafx.h"
#include "GameObject.h"

extern DXResources g_DxResource;

struct CB_CAMERA_INFO {
	XMFLOAT4X4 xmf4x4ViewProj;
	XMFLOAT4X4 xmf4x4InverseViewProj;
	XMFLOAT3 xmf3Eye;
	int bNormalMapping;	// 앞 2byte normal, 뒤 2byte albedo
};

class CCamera {
public:
	void Setup(int nRootParameterIndex);

	void Rotate(int cxDelta, int cyDelta);
	void Move(int arrow, float fElapsedTime);

	void UpdateViewMatrix();
	void SetShaderVariable();

	XMFLOAT3& getEye() { return m_xmf3Eye; }

	void SetTarget(CGameObject* target);
	void SetThirdPersonMode(bool bThirdPerson);
	void SetCameraLength(float fLength) { m_fCameraLength = fLength; }

	void toggleNormalMapping() 
	{ 
		unsigned int fBytes = m_pCameraInfo->bNormalMapping & 0xFFFF0000;
		unsigned int bBytes = m_pCameraInfo->bNormalMapping & 0x0000FFFF;
		fBytes = ~fBytes & 0XFFFF0000;
		m_pCameraInfo->bNormalMapping = fBytes | bBytes;
		//m_pCameraInfo->bNormalMapping = ~m_pCameraInfo->bNormalMapping; 
	}
	void toggleAlbedoColor() 
	{ 
		unsigned int fByte = m_pCameraInfo->bNormalMapping & 0xFFFF0000;
		unsigned int bByte = ~(m_pCameraInfo->bNormalMapping & 0x0000FFFF) & 0x0000FFFF;

		m_pCameraInfo->bNormalMapping = fByte | bByte;
		//m_pCameraInfo->bNormalMapping |= 0x0000; 
	}

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

