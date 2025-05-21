#pragma once
#include "stdafx.h"

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
	bool m_bStart{ true };

	ComPtr<ID3D12Resource> m_DrawBuffer{};
	ComPtr<ID3D12Resource> m_StreamOutputBuffer{};

	ComPtr<ID3D12Resource> m_FilledSizeBuffer{};
	ComPtr<ID3D12Resource> m_ReadBackBuffer{};
	ComPtr<ID3D12Resource> m_UploadBuffer{};
	UINT64* m_Mappedptr{};

	ComPtr<ID3D12Resource> m_VertexBuffer{};
};

class CRaytracingParticle : public CParticle {
public:
protected:
	virtual void BufferReady();

	ComPtr<ID3D12Resource> m_BillboardVertex{};	// 이걸로 만들어진걸

	ComPtr<ID3D12Resource> m_BLAS{};			// BLAS로 만든다.
};
