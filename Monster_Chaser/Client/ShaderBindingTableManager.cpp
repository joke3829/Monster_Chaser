#include "ShaderBindingTableManager.h"

const wchar_t* RayGenShaderNames[] = { L"dd" };
const wchar_t* MissShaderNames[] = { L"", L"" };

void CShaderBindingTableManager::Setup(CRayTracingPipeline* pipeline)
{
	m_pRaytracingPipeline = pipeline;
}

void CShaderBindingTableManager::CreateSBT()
{
	ID3D12StateObjectProperties* properties{};
	m_pRaytracingPipeline->getPipelineState()->QueryInterface(&properties);

	std::vector<void*> raygenIDs;
	std::vector<void*> MissIDs;
	std::vector<void*> HitGroupIDs;

	std::vector<LPCWSTR>& exports = m_pRaytracingPipeline->getExports();
	// Identifier 사용 준비
	for (int i = 0; i < std::size(RayGenShaderNames); ++i) {
		void* raygenID = properties->GetShaderIdentifier(RayGenShaderNames[i]);
		raygenIDs.push_back(raygenID);
	}

	for (int i = 0; i < std::size(MissShaderNames); ++i) {
		void* MissID = properties->GetShaderIdentifier(MissShaderNames[i]);
		MissIDs.push_back(MissID);
	}

	for (int i = 0; i < exports.size(); ++i) {
		void* HitGroupID = properties->GetShaderIdentifier(exports[i]);
		HitGroupIDs.push_back(HitGroupID);
	}

	auto makeBuffer = [&](ComPtr<ID3D12Resource>& buffer, UINT size)
		{
			auto desc = BASIC_BUFFER_DESC;
			desc.Width = size;
			g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&buffer));
		};

	// RaygenTable
	{
		void* tempdata;
		
	}
}