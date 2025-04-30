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
	virtual void SetUp() {}
	virtual void SetCamera(std::shared_ptr<CCamera>& pCamera) { m_pCamera = pCamera; }

	virtual void UpdateObject(float fElapsedTime) {};
	
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void MouseProcessing(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	virtual void PrepareRender() {};
	virtual void Render() {};
protected:
	bool m_bLockAnimation = false;
	bool m_bLockAnimation1 = false;
	bool m_bDoingCombo = false;
	bool m_bStopAnimaiton = false;
	bool m_bRayTracing = false;
	ComPtr<ID3D12RootSignature> m_pGlobalRootSignature{};
	std::shared_ptr<CCamera> m_pCamera{};
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
	virtual void SetUp() {}
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void MouseProcessing(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(float fElapsedTime) {};

	void UpdateObject(float fElapsedTime);

	void PrepareRender();
	void Render();

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
protected:

	ComPtr<ID3D12RootSignature>							m_pLocalRootSignature{};
	std::unique_ptr<CRayTracingPipeline>				m_pRaytracingPipeline{};
	std::unique_ptr<CResourceManager>					m_pResourceManager{};
	std::unique_ptr<CShaderBindingTableManager>			m_pShaderBindingTable{};
	std::unique_ptr<CAccelerationStructureManager>		m_pAccelerationStructureManager{};

	// ��Ű�� �ִϸ��̼� �� ���ҽ�
	ComPtr<ID3D12RootSignature>							m_pComputeRootSignature{};
	ComPtr<ID3D12PipelineState>							m_pAnimationComputeShader{};
};

class CRaytracingTestScene : public CRaytracingScene {
public:
	void SetUp();
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void MouseProcessing(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

	std::unique_ptr<CHeightMapImage> m_pHeightMap{};
private:
	UCHAR m_PrevKeyBuffer[256] = { 0 }; // ���� Ű ���� ����
};

class CRaytracingMaterialTestScene : public CRaytracingScene {
public:
	void SetUp();
	void ProcessInput(float fElapsedTime);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);

};