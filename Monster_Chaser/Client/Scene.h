#pragma once
#include "Camera.h"
#include "RayTracingPipeline.h"
#include "ResourceManager.h"
#include "ShaderBindingTableManager.h"
#include "AccelerationStructureManager.h"
#include "stdfxh.h"

extern DXResources g_DxResource;

enum MageAnimationState
{
	IDLE = 0,
	HIT = 1,
	HIT_DEATH = 2,
	BIGHIT = 3,
	BIGHIT_DEATH =4,
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
	DODGE=21,
	C_ATTACK1=22,
	C_ATTACK2 = 23,
	C_ATTACK3 = 24,
	C_ATTACK4 = 25,
	SKILL3_1 = 26,
	SKILL3_2 = 27,
	SKILL3_3 = 28,
	SKILL3_4 = 29,
	SKILL3_5 = 30,
	SKILL2 = 31,
	SKILL1 = 32
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
	bool m_bLockAnimation = false;
	bool m_bLockAnimation1 = false;
	bool m_bStopAnimaiton = false;
	bool m_bDoingCombo = false;

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

template<typename T>
concept HasGameObjectInterface = requires(T t) {
	{ t.getMeshIndex() } -> std::convertible_to<int>;
	{ t.getWorldMatrix() } -> std::convertible_to<XMFLOAT4X4>;
	{ t.GetBoundingOBB() } -> std::convertible_to<BoundingOrientedBox>;
	{ t.GetBoundingSphere() } -> std::convertible_to<BoundingSphere>;
	{ t.getBoundingInfo() } -> std::convertible_to<unsigned short>;
};

template<typename T>
concept HasSkinningObjectInterface = requires(T t) {
	{ t.getObjects() } -> std::convertible_to<std::vector<std::unique_ptr<CGameObject>>&>;
	{ t.getWorldMatrix() } -> std::convertible_to<XMFLOAT4X4>;
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

	template<typename T, typename U>
	requires (HasGameObjectInterface<T> || HasSkinningObjectInterface<T>) && HasSkinningObjectInterface<U>
	bool CheckSphereCollision(const std::vector<std::unique_ptr<T>>& object1, const std::vector < std::unique_ptr<U>>& obejct2); //1차체크

	template<typename T, typename U>
	requires (HasGameObjectInterface<T> || HasSkinningObjectInterface<T>) && HasSkinningObjectInterface<U>
	void CheckOBBCollisions(const std::vector<std::unique_ptr<T>>& object1, const std::vector<std::unique_ptr<U>>& object2); //세부체크

	void TestCollision(const std::vector<std::unique_ptr<CGameObject>>& mapObjects, const std::vector<std::unique_ptr<CSkinningObject>>& characters);

	XMFLOAT3 CalculateCollisionNormal(const BoundingOrientedBox& obb, const BoundingSphere& sphere); //법선 벡터 구하기
	float CalculateDepth(const BoundingOrientedBox& obb, const BoundingSphere& sphere); //침투 깊이 구하기

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