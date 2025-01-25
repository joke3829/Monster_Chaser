#include "ShaderBindingTableManager.h"

void CShaderBindingTableManager::Setup(ID3D12StateObject* pipeline)
{
	m_pRaytracingPIpeline = pipeline;
}

