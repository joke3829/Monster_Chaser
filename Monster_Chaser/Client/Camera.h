#pragma once

#include "stdafx.h"

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

	void UpdateViewMatrix();
	void SetShaderVariable();
protected:
	ComPtr<ID3D12Resource> m_pd3dCameraBuffer{};
	CB_CAMERA_INFO* m_pCameraInfo{};

	XMFLOAT3 m_xmf3Eye{ 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_xmf3At;
	XMFLOAT3 m_xmf3Up{ 0.0f, 1.0f, 0.0f };

	XMFLOAT3 m_xmf3Dir{ 0.0f, 0.0f, 1.0f };

	XMFLOAT4X4 m_xmf4x4View;
	XMFLOAT4X4 m_xmf4x4Proj;
	
	float m_fFOV = 90.0f;
	float m_fAspect = 960.0f / 540.0f;
	float m_fNear = 0.1f;
	float m_fFar = 1000.0f;

	int m_nRootParameterIndex{};
};

