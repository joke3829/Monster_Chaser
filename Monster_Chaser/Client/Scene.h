#pragma once
#include "Camera.h"
#include "RayTracingPipeline.h"
#include "ResourceManager.h"
#include "ShaderBindingTableManager.h"
#include "AccelerationStructureManager.h"
#include "stdfxh.h"

extern DXResources g_DxResource;

constexpr int MAX_LIGHTS = 64;

struct LIGHT {
	XMFLOAT4							m_xmf4Ambient;
	XMFLOAT4							m_xmf4Diffuse;
	XMFLOAT4							m_xmf4Specular;
	XMFLOAT3							m_xmf3Position;
	float 								m_fFalloff;
	XMFLOAT3							m_xmf3Direction;
	float 								m_fTheta;				//cos(m_fTheta)
	XMFLOAT3							m_xmf3Attenuation;
	float								m_fPhi;					//cos(m_fPhi)
	bool								m_bEnable;
	int									m_nType;
	float								m_fRange;
	float								padding;
};

struct LIGHTS {
	LIGHT			m_pLights[MAX_LIGHTS];
	int				m_nLights;
};

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
	bool								m_bRayTracing = false;
	ComPtr<ID3D12RootSignature>			m_pGlobalRootSignature{};
	std::shared_ptr<CCamera>			m_pCamera{};
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

	ComPtr<ID3D12RootSignature>							m_pLocalRootSignature{};
	std::unique_ptr<CRayTracingPipeline>				m_pRaytracingPipeline{};
	std::unique_ptr<CResourceManager>					m_pResourceManager{};
	std::unique_ptr<CShaderBindingTableManager>			m_pShaderBindingTable{};
	std::unique_ptr<CAccelerationStructureManager>		m_pAccelerationStructureManager{};

	// 스키닝 애니메이션 용 리소스
	ComPtr<ID3D12RootSignature>							m_pComputeRootSignature{};
	ComPtr<ID3D12PipelineState>							m_pAnimationComputeShader{};
};

class CRaytracingTestScene : public CRaytracingScene {
public:
	void SetUp();
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
};

class CRaytracingMaterialTestScene : public CRaytracingScene {
public:
	void SetUp();
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
};