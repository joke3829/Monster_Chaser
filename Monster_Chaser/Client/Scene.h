#pragma once
#include "RayTracingPipeline.h"
#include "ResourceManager.h"
#include "ShaderBindingTableManager.h"
#include "AccelerationStructureManager.h"

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
	void SetUp();

	void Render();
protected:
	void CreateRootSignature();

	ComPtr<ID3D12RootSignature> m_pLocalRootSignature{};
	std::unique_ptr<CRayTracingPipeline> m_pRaytracingPipeline{};
	std::unique_ptr<CResourceManager> m_pResourceManager{};
	std::unique_ptr<CShaderBindingTableManager> m_pShaderBindingTable{};
};