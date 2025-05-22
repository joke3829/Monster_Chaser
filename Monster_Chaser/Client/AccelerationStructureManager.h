// ==================================================
// AccelerationStructureManager.h
// 하나의 Scene마다 해당 AS 가지게 한다.
// ==================================================

#pragma once

#include "ResourceManager.h"

extern DXResources g_DxResource;

// AS Manager Class
class CAccelerationStructureManager {
public:
	void Setup(CResourceManager* resourceManager, UINT parameterIndex);

	void SetScene();				// Pass TLAS to Shader
	void UpdateScene(XMFLOAT3& cameraEye);				

	// Add BLAS to BLAS List
	/*void AddBLAS(ID3D12Resource* vertexBuffer, UINT vertexcount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN);*/
	void InitBLAS();
	// BLAS Create
	/*void MakeBLAS(ComPtr<ID3D12Resource>& asResource,
		ID3D12Resource* vertexBuffer, UINT vertexCount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN, bool bOpaque = true);*/
	void MakeBLAS(ComPtr<ID3D12Resource>& resource, std::unique_ptr<Mesh>& mesh);
	// TLAS Create
	void InitTLAS();
	// AccelerationStructure Create
	void MakeAccelerationStructure(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs, 
		ComPtr<ID3D12Resource>& asResource, UINT64* updateScratchSize = nullptr, bool allowUpdate = false);

private:
	CResourceManager* m_pResourceManager{};

	ComPtr<ID3D12Resource> m_TLAS{};						// TLAS는 하나만 사용, BLASList를 토대로 만든다.
	ComPtr<ID3D12Resource> m_tlasUpdataeScratch{};
	ComPtr<ID3D12Resource> m_InstanceBuffer{};				// instance Buffer of TLAS
	D3D12_RAYTRACING_INSTANCE_DESC* m_pInstanceData{};		

	std::vector<ComPtr<ID3D12Resource>> m_vBLASList;		// BLAS

	UINT m_nValidObject{};			// num of Valid Object
	UINT m_nRootParameterIndex{};	// RootParameter Index
	UINT m_nStaticMesh{};
	bool m_bFirst = true;
};

