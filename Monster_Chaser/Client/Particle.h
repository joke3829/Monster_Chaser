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
	virtual void setMaterial(Material& material) { m_material = material; }

	virtual ID3D12Resource* getVertexBuffer() {}
	virtual ID3D12Resource* getTexCoordBuffer() {}

	virtual ID3D12Resource* getBLAS() {}

	virtual void UpdateObject();

	virtual void Render() {}
	virtual void PostProcess();
protected:
	virtual void BufferReady();

	virtual void OnePath() {}
	virtual void TwoPath() {}

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

	ComPtr<ID3D12PipelineState> m_OnePathPipeline{};
	ComPtr<ID3D12PipelineState> m_TwoPathPipeline{};
	Material m_material;
};

class CRaytracingParticle : public CParticle {
public:
	CRaytracingParticle();
	CRaytracingParticle(UINT maxVertex);

	ID3D12Resource* getVertexBuffer() { return m_BillboardVertex.Get(); }
	ID3D12Resource* getTexCoordBuffer() { return m_TexCoord0Buffer.Get(); }

	ID3D12Resource* getBLAS() { return m_BLAS.Get(); }
	ID3D12Resource* getMeshCB() { return m_MeshCB.Get(); }
	ID3D12Resource* getMaterialCB() { return m_MaterialCB.Get(); }

	void setMaterial(Material& material);

	void Render();
protected:
	void BufferReady();

	void OnePath();
	void TwoPath();

	void InitBLAS();
	void ReBuildBLAS();

	ComPtr<ID3D12Resource> m_MeshCB{};
	ComPtr<ID3D12Resource> m_MaterialCB{};

	// use 2 slot
	ComPtr<ID3D12Resource> m_BillboardVertex{};	// �̰ɷ� ���������
	ComPtr<ID3D12Resource> m_TexCoord0Buffer{};

	ComPtr<ID3D12Resource> m_BLAS{};			// BLAS�� �����.
	ComPtr<ID3D12Resource> m_ScratchBuffer{};
};
