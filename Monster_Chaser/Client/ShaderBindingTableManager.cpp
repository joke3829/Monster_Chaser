#include "ShaderBindingTableManager.h"

// 여기에 셰이더 이름을 사용할 개수 만큼 쓴다.
const wchar_t* RayGenShaderNames[] = { L"dd" };
const wchar_t* MissShaderNames[] = { L"", L"" };

void CShaderBindingTableManager::Setup(CRayTracingPipeline* pipeline, CResourceManager* manager)
{
	m_pRaytracingPipeline = pipeline;
	m_pResourceManager = manager;

	// null 버퍼와 nulltexture
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = 1;			// 여기 의심

	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dNullBuffer.GetAddressOf()));

	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dNullTexture.GetAddressOf()));

	D3D12_DESCRIPTOR_HEAP_DESC descriptorDesc{};
	descriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorDesc.NumDescriptors = 1;
	descriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	g_DxResource.device->CreateDescriptorHeap(&descriptorDesc, IID_PPV_ARGS(m_pd3dNullBufferView.GetAddressOf()));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;

	g_DxResource.device->CreateShaderResourceView(m_pd3dNullTexture.Get(), &srvDesc, m_pd3dNullBufferView->GetCPUDescriptorHandleForHeapStart());
}

void CShaderBindingTableManager::CreateSBT()
{
	ID3D12StateObjectProperties* properties{};
	m_pRaytracingPipeline->getPipelineState()->QueryInterface(&properties);

	std::vector<void*> raygenIDs;		// Raygen은 하나만 쓰는거라 생각하지만 일단은 vector로 사용
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
		makeBuffer(m_pRayGenTable, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * raygenIDs.size());
		m_pRayGenTable->Map(0, nullptr, &tempdata);
		for (int i = 0; i < raygenIDs.size(); ++i) {
			memcpy(tempdata, raygenIDs[i], D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			tempdata = static_cast<char*>(tempdata) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		}
		m_pRayGenTable->Unmap(0, nullptr);
	}

	// MissTable
	{
		void* tempdata;
		makeBuffer(m_pMissTable, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT * MissIDs.size());
		m_pMissTable->Map(0, nullptr, &tempdata);
		for (int i = 0; i < MissIDs.size(); ++i) {
			memcpy(tempdata, MissIDs[i], D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			tempdata = static_cast<char*>(tempdata) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		}
		m_pMissTable->Unmap(0, nullptr);
	}
}