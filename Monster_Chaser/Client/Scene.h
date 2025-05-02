#pragma once
#include "Camera.h"
#include "RayTracingPipeline.h"
#include "ResourceManager.h"
#include "ShaderBindingTableManager.h"
#include "AccelerationStructureManager.h"
#include "stdfxh.h"

extern DXResources g_DxResource;

enum MoveAnimationState
{
	IDLE = 0,
	WALK_FORWARD = 5,
	WALK_LEFT_UP = 6,
	WALK_RIGHT_UP = 7,
	WALK_LEFT = 8,
	WALK_RIGHT = 9,
	WALK_BACKWARD = 10,
	WALK_LEFT_DOWN = 11,
	WALK_RIGHT_DOWN = 12,
	RUN_FORWARD = 13,
	RUN_LEFT_UP = 14,
	RUN_RIGHT_UP = 15,
	RUN_LEFT = 16,
	RUN_RIGHT = 17,
	RUN_BACKWARD = 18,
	RUN_LEFT_DOWN = 19,
	RUN_RIGHT_DOWN = 20,
};

class CScene {
public:
	virtual void SetUp(ComPtr<ID3D12Resource>& outputBuffer) {}
	virtual void SetCamera(std::shared_ptr<CCamera>& pCamera) { m_pCamera = pCamera; }

	virtual void UpdateObject(float fElapsedTime) {};
	
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	virtual void PrepareRender() {};
	virtual void Render() {};

	virtual void PrepareTerrainTexture() {}
protected:
	bool m_LockAnimation = false;
	bool m_LockAnimation1 = false;
	bool m_StopAnimaiton = false;

	bool								m_bRayTracing = false;
	ComPtr<ID3D12RootSignature>			m_pGlobalRootSignature{};
	std::shared_ptr<CCamera>			m_pCamera{};

	POINT oldCursor;
	bool m_bHold = false;
};

enum TitleState{Title, RoomSelect, InRoom, GoLoading};

class TitleScene : public CScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);

	//void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void CreateRootSignature();
	void CreateRenderTargetView();

	void UpdateObject(float fElapsedTime);
	void Render();
protected:
	TitleState							m_nState = Title;

	ComPtr<ID3D12Resource>				m_pOutputBuffer{};
	ComPtr<ID3D12DescriptorHeap>		m_RederTargetView{};
	ComPtr<ID3D12PipelineState>			m_UIPipelineState{};
	std::unique_ptr<CResourceManager>	m_pResourceManager{};
};

class CRaytracingScene : public CScene {
public:
	virtual void SetUp(ComPtr<ID3D12Resource>& outputBuffer) {}
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	virtual void UpdateObject(float fElapsedTime);

	void PrepareRender();
	virtual void Render();

	void CreateRootSignature();
	void CreateComputeRootSignature();
	void CreateComputeShader();

	virtual void PrepareTerrainTexture() {}
protected:

	ComPtr<ID3D12RootSignature>							m_pLocalRootSignature{};
	std::unique_ptr<CRayTracingPipeline>				m_pRaytracingPipeline{};
	std::unique_ptr<CResourceManager>					m_pResourceManager{};
	std::unique_ptr<CShaderBindingTableManager>			m_pShaderBindingTable{};
	std::unique_ptr<CAccelerationStructureManager>		m_pAccelerationStructureManager{};

	// Animation Tool
	ComPtr<ID3D12RootSignature>							m_pComputeRootSignature{};
	ComPtr<ID3D12PipelineState>							m_pAnimationComputeShader{};

	// terrainDescriptor
	ComPtr<ID3D12DescriptorHeap>						m_pTerrainDescriptor{};
	ComPtr<ID3D12Resource>								m_pTerrainCB{};
};

class CRaytracingTestScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
private:
	UCHAR m_PrevKeyBuffer[256] = { 0 }; // PrevKey
};

class CRaytracingMaterialTestScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void Render();
	void PrepareTerrainTexture();
};

// real use scene
class CRaytracingWinterLandScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void UpdateObject(float fElapsedTime);
	void Render();
	void PrepareTerrainTexture();

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
};