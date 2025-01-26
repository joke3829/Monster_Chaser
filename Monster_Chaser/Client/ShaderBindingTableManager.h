#pragma once

#include "RayTracingPipeline.h"

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
	void Setup(CRayTracingPipeline* pipeline);

	void CreateSBT();
private:
	CRayTracingPipeline* m_pRaytracingPipeline{};
	ComPtr<ID3D12Resource> m_pRayGenTable{};

	ComPtr<ID3D12Resource> m_pMissTable{};

	ComPtr<ID3D12Resource> m_pHitGroupTable{};
};

