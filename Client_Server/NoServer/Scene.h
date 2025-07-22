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

enum TitleState{Title, RoomSelect, InRoom, SelectC, GoLoading};

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
	TitleState								m_nState = Title;
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
	std::array<short, 10>					userPerRoom{ 1, 0, 0, 3, 2, 2, 3, 0, 2, 1 };

	// InRoom variables
	short									local_uid{};
	short									currentRoom{};
	std::array<short, 3>					userJob{ JOB_MAGE, JOB_MAGE, JOB_MAGE };
	std::array<bool, 3>						userReadyState{};
	short									readyUIIndex{};
	short									backUIIndex{};

	short prevJob{};
	short CUIindex{};
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
	bool CheckSphereCollision(const std::vector<std::unique_ptr<T>>& object1, const std::vector < std::unique_ptr<U>>& obejct2); 

	template<typename T, typename U>
	requires (HasGameObjectInterface<T> || HasSkinningObjectInterface<T>) && HasSkinningObjectInterface<U>
	void CheckOBBCollisions(const std::vector<std::unique_ptr<T>>& object1, const std::vector<std::unique_ptr<U>>& object2); 

	void TestCollision(const std::vector<std::unique_ptr<CGameObject>>& mapObjects, const std::vector<std::unique_ptr<CSkinningObject>>& characters);
	void TestShootCollision(const std::vector<std::unique_ptr<CProjectile>>& Objects, const std::vector<std::unique_ptr<CSkinningObject>>& characters);

	XMFLOAT3 CalculateCollisionNormal(const BoundingOrientedBox& obb, const BoundingSphere& sphere);
	float CalculateDepth(const BoundingOrientedBox& obb, const BoundingSphere& sphere);

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
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void CreateUIRootSignature();
	void CreateUIPipelineState();

	void CreateMageCharacter();

	void UpdateObject(float fElapsedTime);
	void Render();
	void PrepareTerrainTexture();

	//void TextRender();

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
	std::unique_ptr<CHeightMapImage> m_pCollisionHMap{};
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

	// InGame UI ====================================================================
	std::array<std::vector<std::unique_ptr<UIObject>>, 3>	m_vStatusUIs{};
	std::vector<std::unique_ptr<UIObject>>	m_vItemUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vSkillUIs;
	std::unique_ptr<UIObject>				m_pShopUI;

	short m_numUser = 1;						// replace
	std::array<size_t, 3>				m_buffpixelHeight{};
	std::array<std::array<bool, 3>, 3>	m_BuffState{};	// replace
	std::array<float, 3> maxHPs;		// replace
	std::array<float, 3> cHPs;			// replace

	short cItem = 0;		// replace server var
	bool itemUse{};			// replace server var

	std::array<float, 3> coolTime{};
	std::array<float, 3> curCTime{};

	float maxMP = 100;			// replace
	float cMP = 100;			// replace

	UINT m_nGold = 1500;

	size_t ItemNumTextIndex;
	size_t GoldTextIndex;
	size_t itemNum[4] = {10, 10, 10, 10};

	bool m_bOpenShop = false;

	void PlayerUISetup(short job);		// player job need
	// ===============================================================================
};

// real use scene
class CRaytracingCaveScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void CreateUIRootSignature();
	void CreateUIPipelineState();

	void CreateMageCharacter();

	void UpdateObject(float fElapsedTime);
	void Render();

	//void TextRender();

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
	std::unique_ptr<CHeightMapImage> m_pCollisionHMap{};
protected:
	std::vector<std::unique_ptr<CPlayableCharacter>>	m_vPlayers{};
	std::unique_ptr<CPlayer>							m_pPlayer{};

	unsigned int								m_nSkyboxIndex{};

	ComPtr<ID3D12RootSignature>					m_UIRootSignature{};
	InGameState									m_nState{};

	std::vector<std::unique_ptr<UIObject>>		m_vUIs{};
	float										startTime{};
	float										wOpacity = 1.0f;

	// InGame UI ====================================================================
	std::array<std::vector<std::unique_ptr<UIObject>>, 3>	m_vStatusUIs{};
	std::vector<std::unique_ptr<UIObject>>	m_vItemUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vSkillUIs;
	std::unique_ptr<UIObject>				m_pShopUI;

	short m_numUser = 1;						// replace
	std::array<size_t, 3>				m_buffpixelHeight{};
	std::array<std::array<bool, 3>, 3>	m_BuffState{};	// replace
	std::array<float, 3> maxHPs;		// replace
	std::array<float, 3> cHPs;			// replace

	short cItem = 0;		// replace server var
	bool itemUse{};			// replace server var

	std::array<float, 3> coolTime{};
	std::array<float, 3> curCTime{};

	float maxMP = 100;			// replace
	float cMP = 100;			// replace

	UINT m_nGold = 1500;

	size_t ItemNumTextIndex;
	size_t GoldTextIndex;
	size_t itemNum[4] = { 10, 10, 10, 10 };

	bool m_bOpenShop = false;

	void PlayerUISetup(short job);		// player job need
	// ===============================================================================
};

// real use scene
class CRaytracingETPScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline = nullptr);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void CreateUIRootSignature();
	void CreateUIPipelineState();

	void CreateMageCharacter();

	void UpdateObject(float fElapsedTime);
	void Render();
	void PrepareTerrainTexture();

	//void TextRender();

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

	// InGame UI ====================================================================
	std::array<std::vector<std::unique_ptr<UIObject>>, 3>	m_vStatusUIs{};
	std::vector<std::unique_ptr<UIObject>>	m_vItemUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vSkillUIs;
	std::unique_ptr<UIObject>				m_pShopUI;

	short m_numUser = 1;						// replace
	std::array<size_t, 3>				m_buffpixelHeight{};
	std::array<std::array<bool, 3>, 3>	m_BuffState{};	// replace
	std::array<float, 3> maxHPs;		// replace
	std::array<float, 3> cHPs;			// replace

	short cItem = 0;		// replace server var
	bool itemUse{};			// replace server var

	std::array<float, 3> coolTime{};
	std::array<float, 3> curCTime{};

	float maxMP = 100;			// replace
	float cMP = 100;			// replace

	UINT m_nGold = 1500;

	size_t ItemNumTextIndex;
	size_t GoldTextIndex;
	size_t itemNum[4] = { 10, 10, 10, 10 };

	bool m_bOpenShop = false;

	void PlayerUISetup(short job);		// player job need
	// ===============================================================================
};


class CRaytracingParticleTestScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void UpdateObject(float fElapsedTime);
	void Render();
	void PostProcess();
	void TextRender();

	void CreateParticleRS();
	void CreateOnePath();
	void CreateTwoPath();

	ComPtr<ID3D12PipelineState> m_OnePathPS{};
	ComPtr<ID3D12PipelineState> m_TwoPathPS{};
	ComPtr<ID3D12RootSignature> m_ParticleRS{};

	UINT m_nSkyboxIndex{};

	bool m_bHold = false;
	POINT oldCursor;
};




class UITestScene : public CScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);

	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void CreateRootSignature();
	void CreatePipelineState();

	void UpdateObject(float fElapsedTime);
	void Render();
protected:
	std::unique_ptr<CResourceManager>		m_pResourceManager{};

	std::array<std::vector<std::unique_ptr<UIObject>>, 3>	m_vStatusUIs{};
	std::vector<std::unique_ptr<UIObject>>	m_vItemUIs;
	std::vector<std::unique_ptr<UIObject>>	m_vSkillUIs;
	std::unique_ptr<UIObject>				m_background;
	std::unique_ptr<UIObject>				m_pShopUI;

	short m_numUser = 1;
	std::array<size_t, 3>				m_buffpixelHeight{};
	std::array<std::array<bool, 3>, 3>	m_BuffState{};
	std::array<float, 3> maxHPs;
	std::array<float, 3> cHPs;

	short cItem = 0;
	bool itemUse{};

	std::array<float, 3> coolTime{};
	std::array<float, 3> curCTime{};

	float maxMP = 100;
	float cMP = 100;
};

class CRaytracingCollisionTestScene : public CRaytracingScene {
public:
	void SetUp(ComPtr<ID3D12Resource>& outputBuffer);
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	void CreateUIRootSignature();
	void CreateUIPipelineState();

	void CreateMageCharacter();
	void CreateWarriorCharacter();
	void CreatePriestCharacter();

	void Create3StageBoss(); //스테이지 별로 하나씩 만들어서 모든 몬스터 한번에 하도록 설정하면 될 듯 - 보스는 마지막 인자 true, 일반은 false

	void AttackCollision(const std::vector<std::unique_ptr<CPlayableCharacter>>& players, const std::vector<std::unique_ptr<CPlayableCharacter>>& monsters);

	void UpdateObject(float fElapsedTime);
	void Render();
	void PrepareTerrainTexture();

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
protected:
	std::vector<std::unique_ptr<CPlayableCharacter>>	m_vPlayers{};
	std::unique_ptr<CPlayer>							m_pPlayer{};

	std::vector<std::unique_ptr<CPlayableCharacter>>	m_vMonsters{};
	std::unique_ptr<CMonster>							m_pMonster{};

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