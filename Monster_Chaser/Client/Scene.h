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
	
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	virtual void PrepareRender() {};
	virtual void Render() {};
protected:
	bool m_PressKey = false;
	bool m_bRayTracing = false;
	ComPtr<ID3D12RootSignature> m_pGlobalRootSignature{};
	std::shared_ptr<CCamera> m_pCamera{};
};

class CRaytracingScene : public CScene {
public:
	virtual void SetUp() {}
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	void UpdateObject(float fElapsedTime);

	void PrepareRender();
	void Render();

	void CreateRootSignature();
	void CreateComputeRootSignature();
	void CreateComputeShader();
protected:

	ComPtr<ID3D12RootSignature> m_pLocalRootSignature{};
	std::unique_ptr<CRayTracingPipeline> m_pRaytracingPipeline{};
	std::unique_ptr<CResourceManager> m_pResourceManager{};
	std::unique_ptr<CShaderBindingTableManager> m_pShaderBindingTable{};
	std::unique_ptr<CAccelerationStructureManager> m_pAccelerationStructureManager{};

	// 스키닝 애니메이션 용 리소스
	ComPtr<ID3D12RootSignature> m_pComputeRootSignature{};
	ComPtr<ID3D12PipelineState> m_pAnimationComputeShader{};
};

class CRaytracingTestScene : public CRaytracingScene {
public:
	void SetUp();
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
private:
	UCHAR m_PrevKeyBuffer[256] = { 0 }; // 이전 키 상태 저장
};