#pragma once
#include "Camera.h"
#include "RayTracingPipeline.h"
#include "PlayableCharacter.h"
#include "ShaderBindingTableManager.h"
#include "AccelerationStructureManager.h"
#include "UIObject.h"
#include "stdfxh.h"

extern DXResources g_DxResource;

class CScene {
public:
	virtual ~CScene() {}
	virtual void SetUp(ComPtr<ID3D12Resource>& outputBuffer) { m_pOutputBuffer = outputBuffer; }
	virtual void SetCamera(std::shared_ptr<CCamera>& pCamera) { m_pCamera = pCamera; }
	virtual void CreateRTVDSV();
	virtual void CreateOrthoMatrixBuffer();

	virtual void UpdateObject(float fElapsedTime) {};
	
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	virtual void PrepareRender() {};
	virtual void Render() {};

	short getNextSceneNumber() const { return m_nNextScene; }

	virtual void PrepareTerrainTexture() {}
protected:
	ComPtr<ID3D12Resource>				m_pOutputBuffer{};
	ComPtr<ID3D12DescriptorHeap>		m_RTV{};
	ComPtr<ID3D12PipelineState>			m_UIPipelineState{};
	ComPtr<ID3D12Resource>				m_pDepthStencilBuffer{};
	ComPtr<ID3D12DescriptorHeap>		m_DSV{};

	bool								m_bRayTracing = false;
	ComPtr<ID3D12RootSignature>			m_pGlobalRootSignature{};
	std::shared_ptr<CCamera>			m_pCamera{};

	ComPtr<ID3D12Resource>					m_cameraCB{};
	short m_nNextScene = -1;
};

enum TitleState{Title, RoomSelect, InRoom, GoLoading};

class TitleScene : public CScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);

	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void CreateRootSignature();
	void CreatePipelineState();

	void UpdateObject(float fElapsedTime);
	void Render();
protected:
	TitleState								m_nState = Title;
	std::unique_ptr<CResourceManager>		m_pResourceManager{};

	std::vector<std::unique_ptr<UIObject>>	m_vTitleUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vRoomSelectUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vInRoomUIs;

	// Title variables
	float									wOpacity = 1.0f;
	float									startTime{};
	// Room Select variables
	int										peopleindex{};
	std::array<short, 10>					userPerRoom{ 1, 0, 0, 3, 2, 2, 3, 0, 2, 1 };

	// InRoom variables
	short									local_uid{};
	short									currentRoom{};
	std::array<short, 3>					userJob{ JOB_MAGE, JOB_MAGE, JOB_MAGE };
	std::array<bool, 3>						userReadyState{};
	short									readyUIIndex{};
	short									backUIIndex{};
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
	bool CheckSphereCollision(const std::vector<std::unique_ptr<T>>& object1, const std::vector < std::unique_ptr<U>>& obejct2);

	template<typename T, typename U>
	requires (HasGameObjectInterface<T> || HasSkinningObjectInterface<T>) && HasSkinningObjectInterface<U>
	void CheckOBBCollisions(const std::vector<std::unique_ptr<T>>& object1, const std::vector<std::unique_ptr<U>>& object2);

	void TestCollision(const std::vector<std::unique_ptr<CGameObject>>& mapObjects, const std::vector<std::unique_ptr<CSkinningObject>>& characters);
	void TestShootCollision(const std::vector<std::unique_ptr<CProjectile>>& Objects, const std::vector<std::unique_ptr<CSkinningObject>>& characters);

	XMFLOAT3 CalculateCollisionNormal(const BoundingOrientedBox& obb, const BoundingSphere& sphere); //법선 벡터 구하기
	float CalculateDepth(const BoundingOrientedBox& obb, const BoundingSphere& sphere); //침투 깊이 구하기

	bool CheckTriangleSphereCollision(const XMFLOAT3* tri, const BoundingSphere& sphere, XMFLOAT3& outNormal, float& outDepth);

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
};

class CRaytracingTestScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
};

class CRaytracingMaterialTestScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void Render();
};

enum InGameState{IS_LOADING, IS_GAMING, IS_FINISH};

// real use scene
class CRaytracingWinterLandScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void CreateUIRootSignature();
	void CreateUIPipelineState();

	void CreateMageCharacter();
	void CreateWarriorCharacter();

	void UpdateObject(float fElapsedTime);
	void Render();
	void PrepareTerrainTexture();

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
protected:
	std::vector<std::unique_ptr<CPlayableCharacter>>	m_vPlayers{};
	std::unique_ptr<CPlayer>							m_pPlayer{};
	
	unsigned int								m_nSkyboxIndex{};

	ComPtr<ID3D12RootSignature>					m_UIRootSignature{};
	InGameState									m_nState{};

	std::vector<std::unique_ptr<UIObject>>		m_vUIs{};
	float										startTime{};
	float										wOpacity = 1.0f;

	// terrainDescriptor
	ComPtr<ID3D12DescriptorHeap>						m_pTerrainDescriptor{};
	ComPtr<ID3D12Resource>								m_pTerrainCB{};
};