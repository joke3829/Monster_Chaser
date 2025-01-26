#pragma once

#include "RayTracingPipeline.h"

// ===============================================================================
// 바인딩 테이블을 만들기 위해선 StateObject를 이용해서 Identifier를 가져와야하고
// HitGroup에 Local Root Argument를 넣어줘야 하는데 그러려면 이 Manager한테
// InstanceID로 인덱싱 된 RootArgument가 필요? 
// 또한 HitGroup에 따라서 TLAS를 만들 때 HitGroupIndex를 넣어줘야한다.
// Instance를 관리하는 무언가가 필요한데;;;;;
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

