#pragma once
#include "Camera.h"
#include "RayTracingPipeline.h"
#include "PlayableCharacter.h"
#include "ShaderBindingTableManager.h"
#include "AccelerationStructureManager.h"
#include "UIObject.h"
#include "TextManager.h"
#include "stdfxh.h"
#include "Monster.h"
#include "ObjectManager.h"

extern DXResources g_DxResource;

class CScene {
public:
	virtual ~CScene() {}
	virtual void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr) { m_pOutputBuffer = outputBuffer; }
	virtual void SetCamera(std::shared_ptr<CCamera>& pCamera) { m_pCamera = pCamera; }
	virtual void CreateRTVDSV();
	virtual void CreateOrthoMatrixBuffer();

	virtual void UpdateObject(float fElapsedTime) {};

	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	virtual void PrepareRender() {}
	virtual void Render() {}
	virtual void PostProcess() {}

	//virtual void TextRender() {}

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
	//std::shared_ptr<CTextManager>			m_pTextManager{};
	short m_nNextScene = -1;
};

// ==================================================================================

class TitleScene : public CScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr);

	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void CreateRootSignature();
	void CreatePipelineState();

	void UpdateObject(float fElapsedTime);
	void Render();
protected:
	std::unique_ptr<CResourceManager>		m_pResourceManager{};

	std::vector<std::unique_ptr<UIObject>>	m_vTitleUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vRoomSelectUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vInRoomUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vSelectCUIs;
	// Title variables
	float									wOpacity = 1.0f;
	float									startTime{};
	// Room Select variables
	int										peopleindex{};


	// InRoom variables
	short									local_uid{};
	short									currentRoom{};
	std::array<short, 3>				userJob{ JOB_MAGE, JOB_MAGE, JOB_MAGE };
	std::array<bool, 3>						userReadyState{};
	short									readyUIIndex{};
	short									backUIIndex{};

	short prevJob{};
	short CUIindex{};
};

// ==================================================================================

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
	virtual void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr) {}
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	virtual void UpdateObject(float fElapsedTime);

	void PrepareRender();
	virtual void Render();
	virtual void PostProcess() {};

	template<typename T, typename U>
		requires (HasGameObjectInterface<T> || HasSkinningObjectInterface<T>) && HasSkinningObjectInterface<U>
	bool CheckSphereCollision(const std::vector<std::unique_ptr<T>>& object1, const std::vector < std::unique_ptr<U>>& obejct2); //1차체크

	template<typename T, typename U>
		requires (HasGameObjectInterface<T> || HasSkinningObjectInterface<T>) && HasSkinningObjectInterface<U>
	void CheckOBBCollisions(const std::vector<std::unique_ptr<T>>& object1, const std::vector<std::unique_ptr<U>>& object2); //세부체크

	void TestCollision(const std::vector<std::unique_ptr<CGameObject>>& mapObjects, const std::vector<std::unique_ptr<CSkinningObject>>& characters);
	void TestShootCollision(const std::vector<std::unique_ptr<CProjectile>>& Objects, const std::vector<std::unique_ptr<CSkinningObject>>& characters);

	XMFLOAT3 CalculateCollisionNormal(const BoundingOrientedBox& obb, const BoundingSphere& sphere); //법선 벡터 구하기
	float CalculateDepth(const BoundingOrientedBox& obb, const BoundingSphere& sphere); //침투 깊이 구하기

	//void CreateRootSignature();
	void CreateComputeRootSignature();
	void CreateComputeShader();

	virtual void PrepareTerrainTexture() {}
protected:

	//ComPtr<ID3D12RootSignature>							m_pLocalRootSignature{};
	std::shared_ptr<CRayTracingPipeline>				m_pRaytracingPipeline{};
	std::unique_ptr<CResourceManager>					m_pResourceManager{};
	std::unique_ptr<CShaderBindingTableManager>			m_pShaderBindingTable{};
	std::unique_ptr<CAccelerationStructureManager>		m_pAccelerationStructureManager{};

	// Animation Tool
	ComPtr<ID3D12RootSignature>							m_pComputeRootSignature{};
	ComPtr<ID3D12PipelineState>							m_pAnimationComputeShader{};
};

class CRaytracingGameScene : public CRaytracingScene {
public:
	virtual void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr) {}
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	virtual void UpdateObject(float fElapsedTime) {}

	void CreateUIRootSignature();
	void CreateUIPipelineState();

	void AttackCollision(const std::vector<std::unique_ptr<CPlayableCharacter>>& targets, const std::vector<std::unique_ptr<CPlayableCharacter>>& attackers, int flag);
	void ShootCollision(const std::vector<std::unique_ptr<CPlayableCharacter>>& targets, const std::vector<std::unique_ptr<CPlayableCharacter>>& attackers, int flag);
	void AutoDirection(const std::vector<std::unique_ptr<CPlayableCharacter>>& attacker, const std::vector<std::unique_ptr<CPlayableCharacter>>& targets);

	void CreateMageCharacter();
	void CreateWarriorCharacter();
	void CreatePriestCharacter();

	virtual void Render() {}
	void PostProcess();
protected:
	std::vector<std::unique_ptr<CPlayableCharacter>>	m_vPlayers{};
	std::unique_ptr<CPlayer>							m_pPlayer{};

	std::vector<std::unique_ptr<CPlayableCharacter>>	m_vMonsters{};
	std::vector<std::unique_ptr<CMonster>>				m_pMonsters{};

	ComPtr<ID3D12RootSignature>					m_UIRootSignature{};

	// InGame UI ====================================================================
	bool m_bUIOnOff = true;

	int cItem{};

	std::array<std::vector<std::unique_ptr<UIObject>>, 3> m_vPlayersStatUI{};
	std::vector<std::unique_ptr<UIObject>>	m_vItemUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vSkillUIs;

	short m_numUser = 3;						// replace	Player.size()
	short m_local_id = 0;
	short user_job[3] = { JOB_MAGE, JOB_WARRIOR, JOB_HEALER };

	std::array<bool, 3>	m_BuffState{};	// replace
	std::array<float, 3> maxHPs;		// replace
	std::array<float, 3> cHPs;			// replace

	std::array<float, 3> coolTime{};
	std::array<float, 3> curCTime{};
	std::array<float, 3> skillCost{};

	float maxMPs[3] = { 100, 100, 100 };			// replace
	float cMPs[3] = { 100, 37, 78 };			// replace

	UINT m_nGold = 1500;

	size_t ItemNumTextIndex;

	void PlayerUISetup(short job);		// player job need
	// ===============================================================================
};

// real use scene
class CRaytracingWinterLandScene : public CRaytracingGameScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void Create_Gorhorrid();

	void UpdateObject(float fElapsedTime);
	void Render();
	void PrepareTerrainTexture();

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
	std::unique_ptr<CHeightMapImage> m_pRoadTerrain{};
	std::unique_ptr<CHeightMapImage> m_pCollisionHMap{};
protected:
	unsigned int								m_nSkyboxIndex{};

	std::vector<std::unique_ptr<UIObject>>		m_vUIs{};
	float										startTime{};
	float										wOpacity = 1.0f;

	// terrainDescriptor
	ComPtr<ID3D12DescriptorHeap>						m_pTerrainDescriptor{};
	ComPtr<ID3D12Resource>								m_pTerrainCB{};

	int m_nMonsterNum{};
};

// real use scene
class CRaytracingCaveScene : public CRaytracingGameScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void Create_Limadon();
	void Create_Fulgurodonte();
	void Create_Occisodonte();
	void Create_Crassorrid();

	void UpdateObject(float fElapsedTime);
	void Render();

	//void TextRender();

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
	std::unique_ptr<CHeightMapImage> m_pCollisionHMap{};
protected:
	unsigned int								m_nSkyboxIndex{};

	InGameState									m_nState{};

	std::vector<std::unique_ptr<UIObject>>		m_vUIs{};
	float										startTime{};
	float										wOpacity = 1.0f;

	int m_nMonsterNum{};
};

// real use scene
class CRaytracingETPScene : public CRaytracingGameScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void Create_Feroptere();
	void Create_Pistriptere();
	void Create_RostrokarckLarvae();
	void Create_Xenokarce();

	void UpdateObject(float fElapsedTime);
	void Render();
	void PrepareTerrainTexture();

	//void TextRender();

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
	std::unique_ptr<CHeightMapImage> m_pRoadTerrain{};
	std::unique_ptr<CHeightMapImage> m_pCollisionHMap{};
protected:
	unsigned int								m_nSkyboxIndex{};

	InGameState									m_nState{};

	std::vector<std::unique_ptr<UIObject>>		m_vUIs{};
	float										startTime{};
	float										wOpacity = 1.0f;

	// terrainDescriptor
	ComPtr<ID3D12DescriptorHeap>						m_pTerrainDescriptor{};
	ComPtr<ID3D12Resource>								m_pTerrainCB{};

	int m_nMonsterNum{};
};