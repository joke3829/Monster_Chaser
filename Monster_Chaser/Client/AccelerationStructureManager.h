// ==================================================
// AccelerationStructureManager.h
// 하나의 Scene마다 해당 AS 가지게 한다.
// ==================================================

#pragma once

#include "stdafx.h"

extern DXResources g_DxResource;

// AS를 한번에 관리하는 클래스
class CAccelerationStructureManager {
public:
	void SetScene();				// 셰이더에 TLAS를 넘겨준다.
	void UpdateScene();				// instance 정보를 받아 행렬및 장면을 업데이트 해주자

	

	void AddBLAS(ID3D12Resource* vertexBuffer, UINT vertexcount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN);

	void MakeBLAS(ComPtr<ID3D12Resource>& asResource,
		ID3D12Resource* vertexBuffer, UINT vertexCount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN);

	void MakeTLAS();

	void MakeAccelerationStructure(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs, ComPtr<ID3D12Resource>& asResource, UINT64* updateScratchSize = nullptr);

private:
	ComPtr<ID3D12Resource> m_TLAS{};						// TLAS는 하나만 사용, BLASList를 토대로 만든다.
	ComPtr<ID3D12Resource> m_InstanceBuffer{};				// TLAS에 들어갈 instance 버퍼
	ComPtr<ID3D12Resource> m_TLASUpdateScratch{};			// 
	D3D12_RAYTRACING_INSTANCE_DESC* m_pInstanceData{};		// 

	std::vector<ComPtr<ID3D12Resource>> m_vBLASList;		// BLAS
	UINT m_nInstances;				// 인스턴스 개수

	UINT m_nRootParameterIndex{};	// 루트 파라미터 인덱스
};

