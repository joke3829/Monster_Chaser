// ==================================================
// AccelerationStructureManager.h
// �ϳ��� Scene���� �ش� AS ������ �Ѵ�.
// ==================================================

#pragma once

#include "ResourceManager.h"

extern DXResources g_DxResource;

// AS�� �ѹ��� �����ϴ� Ŭ����
class CAccelerationStructureManager {
public:
	void Setup(CResourceManager* resourceManager, UINT parameterIndex);

	void SetScene();				// ���̴��� TLAS�� �Ѱ��ش�.
	void UpdateScene(XMFLOAT3& cameraEye);				// instance ������ �޾� ��Ĺ� ����� ������Ʈ ������

	// BLASList�� BLAS�� �߰��Ѵ�.
	/*void AddBLAS(ID3D12Resource* vertexBuffer, UINT vertexcount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN);*/
	void InitBLAS();
	// BLAS�� �����Ѵ�.
	/*void MakeBLAS(ComPtr<ID3D12Resource>& asResource,
		ID3D12Resource* vertexBuffer, UINT vertexCount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN, bool bOpaque = true);*/
	void MakeBLAS(ComPtr<ID3D12Resource>& resource, std::unique_ptr<Mesh>& mesh);
	// TLAS�� �����Ѵ�.
	void InitTLAS();
	// AccelerationStructure�� �����.
	void MakeAccelerationStructure(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs, 
		ComPtr<ID3D12Resource>& asResource, UINT64* updateScratchSize = nullptr, bool allowUpdate = false);

private:
	CResourceManager* m_pResourceManager{};

	ComPtr<ID3D12Resource> m_TLAS{};						// TLAS�� �ϳ��� ���, BLASList�� ���� �����.
	ComPtr<ID3D12Resource> m_tlasUpdataeScratch{};
	ComPtr<ID3D12Resource> m_InstanceBuffer{};				// TLAS�� �� instance ����
	D3D12_RAYTRACING_INSTANCE_DESC* m_pInstanceData{};		// 

	std::vector<ComPtr<ID3D12Resource>> m_vBLASList;		// BLAS

	UINT m_nValidObject{};			// ��ȿ�� ������Ʈ(�ν��Ͻ�) ����
	UINT m_nRootParameterIndex{};	// ��Ʈ �Ķ���� �ε���
	UINT m_nStaticMesh{};
	bool m_bFirst = true;
};

