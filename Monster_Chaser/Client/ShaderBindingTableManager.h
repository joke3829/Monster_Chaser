#pragma once

#include "RayTracingPipeline.h"
#include "ResourceManager.h"

// ===============================================================================
// ���ε� ���̺��� ����� ���ؼ� StateObject�� �̿��ؼ� Identifier�� �����;��ϰ�
// HitGroup�� Local Root Argument�� �־���� �ϴµ� �׷����� �� Manager����
// InstanceID�� �ε��� �� RootArgument�� �ʿ�? 
// ���� HitGroup�� ���� TLAS�� ���� �� HitGroupIndex�� �־�����Ѵ�.
// Instance�� �����ϴ� ���𰡰� �ʿ��ѵ�;;;;;
// ===============================================================================

extern DXResources g_DxResource;

class CShaderBindingTableManager {
public:
	void Setup(CRayTracingPipeline* pipeline, CResourceManager* manager);

	void CreateSBT();

	ID3D12Resource* getRayGenTable() const;
	ID3D12Resource* getMissTable() const;
	ID3D12Resource* getHitGroupTable() const;

	UINT64 getHitGroupSize() const;
	UINT64 getHitGroupStride() const;
private:
	CRayTracingPipeline* m_pRaytracingPipeline{};
	CResourceManager* m_pResourceManager{};
	ComPtr<ID3D12Resource> m_pRayGenTable{};

	ComPtr<ID3D12Resource> m_pMissTable{};

	ComPtr<ID3D12Resource> m_pHitGroupTable{};
	UINT64 m_nHitGroupSize{};
	UINT64 m_nHitGroupStride{};

	ComPtr<ID3D12Resource> m_pd3dNullBuffer{};
	ComPtr<ID3D12Resource> m_pd3dNullTexture{};
	ComPtr<ID3D12DescriptorHeap> m_pd3dNullBufferView;
};

