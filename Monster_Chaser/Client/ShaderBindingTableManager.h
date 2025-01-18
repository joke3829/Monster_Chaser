#pragma once

#include "stdafx.h"

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

private:
	ComPtr<ID3D12Resource> m_pRayGenTable{};

	ComPtr<ID3D12Resource> m_pMissTable{};

	ComPtr<ID3D12Resource> m_pHitGroupTable{};
};

