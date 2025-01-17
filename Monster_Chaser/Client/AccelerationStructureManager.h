// ==================================================
// AccelerationStructureManager.h
//
// ==================================================

#pragma once

#include "stdafx.h"

extern DXResources g_DxResource;

// AS�� �ѹ��� �����ϴ� Ŭ����
class CAccelerationStructureManager {
public:
	void SetScene();				// ���̴��� TLAS�� �Ѱ��ش�.
	void UpdateScene();				// instance ������ �޾� ��Ĺ� ����� ������Ʈ ������

	

	void AddBLAS(ID3D12Resource* vertexBuffer, UINT vertexcount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN);

	void MakeBLAS(ComPtr<ID3D12Resource>& asResource,
		ID3D12Resource* vertexBuffer, UINT vertexCount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN);

	void MakeTLAS();

	void MakeAccelerationStructure(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs, ComPtr<ID3D12Resource>& asResource, UINT64* updateScratchSize = nullptr);

private:
	ComPtr<ID3D12Resource> m_TLAS{};						// TLAS�� �ϳ��� ���, BLASList�� ���� �����.
	ComPtr<ID3D12Resource> m_InstanceBuffer{};				// TLAS�� �� instance ����
	UINT64* m_pUpdateScracthSize{};

	std::vector<ComPtr<ID3D12Resource>> m_vBLASList;		// BLAS
	UINT m_nInstances;				// �ν��Ͻ� ����

	UINT m_nRootParameterIndex{};	// ��Ʈ �Ķ���� �ε���
};
