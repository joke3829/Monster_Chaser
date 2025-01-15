// ===========================================
// BLASManager.h
// BLAS를 관리하는 Class를 선언
// ===========================================
#pragma once
#include "stdafx.h"

class CBLASManager {
public:
	void AddBLAS(ID3D12Resource* vertexBuffer, UINT vertexcount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN);

	void MakeBLAS(ComPtr<ID3D12Resource>& asResource,
		ID3D12Resource* vertexBuffer, UINT vertexCount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
		ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN);
private:
	std::vector < ComPtr<ID3D12Resource>> m_vBLASList;
};

