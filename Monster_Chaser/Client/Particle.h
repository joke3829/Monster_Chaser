#pragma once
#include "stdafx.h"
#include "GameObject.h"

extern DXResources g_DxResource;

class CParticle {
public:
	CParticle();
	CParticle(UINT maxVertex);

	XMFLOAT3 getPosition();

	void setPosition(XMFLOAT3& pos);

	virtual void Render() {}
protected:
	virtual void BufferReady();

	XMFLOAT4X4 m_WorldMatrix{};
	ComPtr<ID3D12Resource> m_WorldBuffer{};

	UINT m_nMaxVertex{};
	UINT m_nCurrentVertex{};
	bool m_bStart{ true };

	ComPtr<ID3D12Resource> m_DrawBuffer{};
	ComPtr<ID3D12Resource> m_StreamOutputBuffer{};

	ComPtr<ID3D12Resource> m_FilledSizeBuffer{};
	ComPtr<ID3D12Resource> m_ReadBackBuffer{};
	ComPtr<ID3D12Resource> m_UploadBuffer{};
	UINT64* m_Mappedptr{};

	ComPtr<ID3D12Resource> m_VertexBuffer{};

	Material m_material;
};

class CRaytracingParticle : public CParticle {
public:
	CRaytracingParticle();
	CRaytracingParticle(UINT maxVertex);

protected:
	void BufferReady();

	void InitBLAS();
	void ReBuildBLAS();

	ComPtr<ID3D12Resource> m_MeshCB{};
	ComPtr<ID3D12Resource> m_MaterialCB{};

	// use 2 slot
	ComPtr<ID3D12Resource> m_BillboardVertex{};	// 이걸로 만들어진걸
	ComPtr<ID3D12Resource> m_TexCoord0Buffer{};

	ComPtr<ID3D12Resource> m_BLAS{};			// BLAS로 만든다.
	ComPtr<ID3D12Resource> m_ScratchBuffer{};
};
