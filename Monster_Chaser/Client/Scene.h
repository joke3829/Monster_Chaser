#pragma once
#include "stdafx.h"

extern DXResources g_DxResource;

class CScene {
public:
	virtual void SetUp() {}

	virtual void Render() {};
protected:
	bool m_bRayTracing = false;
	ComPtr<ID3D12RootSignature> m_pGlobalRootSignature{};
};

class CRaytracingScene : public CScene {
public:

protected:
	ComPtr<ID3D12RootSignature> m_pLocalRootSignature{};
};