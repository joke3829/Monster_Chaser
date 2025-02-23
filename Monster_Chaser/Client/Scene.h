#pragma once
#include "Camera.h"
#include "RayTracingPipeline.h"
#include "ResourceManager.h"
#include "ShaderBindingTableManager.h"
#include "AccelerationStructureManager.h"
#include "stdfxh.h"

extern DXResources g_DxResource;

class CScene {
public:
	virtual void SetUp() {}
	virtual void SetCamera(std::shared_ptr<CCamera>& pCamera) { m_pCamera = pCamera; }

	virtual void UpdateObject(float fElapsedTime) {};

	virtual void PrepareRender() {};
	virtual void Render() {};
protected:
	bool m_bRayTracing = false;
	ComPtr<ID3D12RootSignature> m_pGlobalRootSignature{};
	std::shared_ptr<CCamera> m_pCamera{};
};

class CRaytracingScene : public CScene {
public:
	void SetUp();
	void UpdateObject(float fElapsedTime);

	void PrepareRender();
	void Render();
protected:
	void CreateRootSignature();
	void CreateComputeRootSignature();
	void CreateComputeShader();

	ComPtr<ID3D12RootSignature> m_pLocalRootSignature{};
	std::unique_ptr<CRayTracingPipeline> m_pRaytracingPipeline{};
	std::unique_ptr<CResourceManager> m_pResourceManager{};
	std::unique_ptr<CShaderBindingTableManager> m_pShaderBindingTable{};
	std::unique_ptr<CAccelerationStructureManager> m_pAccelerationStructureManager{};

	// 스키닝 애니메이션 용 리소스
	ComPtr<ID3D12RootSignature> m_pComputeRootSignature{};
	ComPtr<ID3D12PipelineState> m_pAnimationComputeShader{};
};