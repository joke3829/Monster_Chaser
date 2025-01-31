#include "ShaderBindingTableManager.h"

// 여기에 셰이더 이름을 사용할 개수 만큼 쓴다.
const wchar_t* RayGenShaderNames[] = { L"dd" };
const wchar_t* MissShaderNames[] = { L"", L"" };

struct LocalRootArg {
	D3D12_GPU_VIRTUAL_ADDRESS CBufferGPUVirtualAddress;		// Material상수버퍼
	D3D12_GPU_VIRTUAL_ADDRESS MeshCBufferGPUVirtualAddress;	// Mesh 상수버퍼	
	// Mesh 정보
	D3D12_GPU_VIRTUAL_ADDRESS VertexBuffer;
	D3D12_GPU_VIRTUAL_ADDRESS ColorsBuffer;
	D3D12_GPU_VIRTUAL_ADDRESS TexCoord0Buffer;
	D3D12_GPU_VIRTUAL_ADDRESS TexCoord1Buffer;
	D3D12_GPU_VIRTUAL_ADDRESS NormalsBuffer;
	D3D12_GPU_VIRTUAL_ADDRESS TangentBuffer;
	D3D12_GPU_VIRTUAL_ADDRESS BiTangentBuffer;
	D3D12_GPU_VIRTUAL_ADDRESS IndexBuffer;

	D3D12_GPU_DESCRIPTOR_HANDLE AlbedoMap;
	D3D12_GPU_DESCRIPTOR_HANDLE SpecularMap;
	D3D12_GPU_DESCRIPTOR_HANDLE NormalMap;
	D3D12_GPU_DESCRIPTOR_HANDLE MetallicMap;
	D3D12_GPU_DESCRIPTOR_HANDLE EmissionMap;
	D3D12_GPU_DESCRIPTOR_HANDLE DetailAlbedoMap;
	D3D12_GPU_DESCRIPTOR_HANDLE DetailNormalMap;
};

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

	// HitGroupTable
	{
		m_nHitGroupSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + sizeof(LocalRootArg);
		m_nHitGroupStride = Align(m_nHitGroupSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		UINT nVaildMeshes{};
		std::vector<std::unique_ptr<CGameObject>>& vObjects = m_pResourceManager->getGameObjectList();
		std::vector<std::unique_ptr<Mesh>>& vMeshes = m_pResourceManager->getMeshList();
		std::vector<std::unique_ptr<CTexture>>& vTextures = m_pResourceManager->getTextureList();

		for (std::unique_ptr<CGameObject>& object : vObjects) {
			int n = object->getMeshIndex();
			if (vMeshes[n]->getHasVertex()) {
				if (vMeshes[n]->getHasSubmesh())
					nVaildMeshes += vMeshes[n]->getSubMeshCount();
				else
					nVaildMeshes += 1;
			}
		}

		makeBuffer(m_pHitGroupTable, m_nHitGroupStride * nVaildMeshes * exports.size());
		void* ptrStride{};
		void* ptrtemp{};
		m_pHitGroupTable->Map(0, nullptr, &ptrStride);
		ptrtemp = ptrStride;
		int nRecords{};
		for (std::unique_ptr<CGameObject>& object : vObjects) {
			int n = object->getMeshIndex();
			std::vector<Material>& vMaterials = object->getMaterials();
			if (vMeshes[n]->getHasVertex()) {
				object->SetHitGroupIndex(nRecords);	// object의 hitgroup index 설정
				if (vMeshes[n]->getHasSubmesh()) {	// 서브 메시를 가질 때(인덱스를 가질 때)
					for (int i = 0; i < vMeshes[n]->getSubMeshCount(); ++i) {	// i == object의 mesh 인덱스
						for (int j = 0; j < HitGroupIDs.size(); ++j) {			// j == IDs 인덱스
							LocalRootArg args{};
							args.CBufferGPUVirtualAddress = object->getCbuffer(i)->GetGPUVirtualAddress();
							args.MeshCBufferGPUVirtualAddress = object->getMeshCBuffer()->GetGPUVirtualAddress();
							// 정점
							args.VertexBuffer = vMeshes[n]->getVertexBuffer()->GetGPUVirtualAddress();
							// 컬러
							if (vMeshes[n]->getHasColor())
								args.ColorsBuffer = vMeshes[n]->getColorsBuffer()->GetGPUVirtualAddress();
							else
								args.ColorsBuffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
							// Tex0
							if (vMeshes[n]->getHasTex0())
								args.TexCoord0Buffer = vMeshes[n]->getTexCoord0Buffer()->GetGPUVirtualAddress();
							else
								args.TexCoord0Buffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
							// Tex1
							if (vMeshes[n]->getHasTex1())
								args.TexCoord1Buffer = vMeshes[n]->getTexCoord1Buffer()->GetGPUVirtualAddress();
							else
								args.TexCoord1Buffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
							// Normal
							if (vMeshes[n]->getHasNormal())
								args.NormalsBuffer = vMeshes[n]->getNormalsBuffer()->GetGPUVirtualAddress();
							else
								args.NormalsBuffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
							// Tangent
							if (vMeshes[n]->getHasTangent())
								args.TangentBuffer = vMeshes[n]->getTangentsBuffer()->GetGPUVirtualAddress();
							else
								args.TangentBuffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
							// BiTangent
							if (vMeshes[n]->getHasBiTangent())
								args.BiTangentBuffer = vMeshes[n]->getBiTangentsBuffer()->GetGPUVirtualAddress();
							else
								args.BiTangentBuffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
							// Index
							args.IndexBuffer = vMeshes[n]->getIndexBuffer(i)->GetGPUVirtualAddress();

							// AlbedoMap
							if (vMaterials[i].m_bHasAlbedoMap)
								args.AlbedoMap = vTextures[vMaterials[i].m_nAlbedoMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
							else
								args.AlbedoMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
							// SpecularMap
							if (vMaterials[i].m_bHasSpecularMap)
								args.SpecularMap = vTextures[vMaterials[i].m_nSpecularMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
							else
								args.SpecularMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
							// NormalMap
							if (vMaterials[i].m_bHasNormalMap)
								args.NormalMap = vTextures[vMaterials[i].m_nNormalMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
							else
								args.NormalMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
							// MetallicMap
							if (vMaterials[i].m_bHasMetallicMap)
								args.MetallicMap = vTextures[vMaterials[i].m_nMetallicMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
							else
								args.MetallicMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
							// EmissionMap
							if (vMaterials[i].m_bHasEmissionMap)
								args.EmissionMap = vTextures[vMaterials[i].m_nEmissionMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
							else
								args.EmissionMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
							// DetailAlbedoMap
							if (vMaterials[i].m_bHasDetailAlbedoMap)
								args.DetailAlbedoMap = vTextures[vMaterials[i].m_nDetailAlbedoMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
							else
								args.DetailAlbedoMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
							// DetailNormalMap
							if (vMaterials[i].m_bHasDetailNormalMap)
								args.DetailNormalMap = vTextures[vMaterials[i].m_nDetailNormalMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
							else
								args.DetailNormalMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();

							memcpy(ptrtemp, HitGroupIDs[j], D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
							ptrtemp = static_cast<char*>(ptrtemp) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
							memcpy(ptrtemp, &args, sizeof(LocalRootArg));
							ptrStride = static_cast<char*>(ptrStride) + m_nHitGroupStride;
							ptrtemp = ptrStride;

							++nRecords;
						}
					}
				}
				else {	// 정점 정보는 있으나 인덱스가 없을 때
					for (int j = 0; j < HitGroupIDs.size(); ++j) {
						LocalRootArg args{};
						args.CBufferGPUVirtualAddress = object->getCbuffer(0)->GetGPUVirtualAddress();
						args.MeshCBufferGPUVirtualAddress = object->getMeshCBuffer()->GetGPUVirtualAddress();
						// 정점
						args.VertexBuffer = vMeshes[n]->getVertexBuffer()->GetGPUVirtualAddress();
						// 컬러
						if (vMeshes[n]->getHasColor())
							args.ColorsBuffer = vMeshes[n]->getColorsBuffer()->GetGPUVirtualAddress();
						else
							args.ColorsBuffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
						// Tex0
						if (vMeshes[n]->getHasTex0())
							args.TexCoord0Buffer = vMeshes[n]->getTexCoord0Buffer()->GetGPUVirtualAddress();
						else
							args.TexCoord0Buffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
						// Tex1
						if (vMeshes[n]->getHasTex1())
							args.TexCoord1Buffer = vMeshes[n]->getTexCoord1Buffer()->GetGPUVirtualAddress();
						else
							args.TexCoord1Buffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
						// Normal
						if (vMeshes[n]->getHasNormal())
							args.NormalsBuffer = vMeshes[n]->getNormalsBuffer()->GetGPUVirtualAddress();
						else
							args.NormalsBuffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
						// Tangent
						if (vMeshes[n]->getHasTangent())
							args.TangentBuffer = vMeshes[n]->getTangentsBuffer()->GetGPUVirtualAddress();
						else
							args.TangentBuffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
						// BiTangent
						if (vMeshes[n]->getHasBiTangent())
							args.BiTangentBuffer = vMeshes[n]->getBiTangentsBuffer()->GetGPUVirtualAddress();
						else
							args.BiTangentBuffer = m_pd3dNullBuffer->GetGPUVirtualAddress();
						// Index
						args.IndexBuffer = m_pd3dNullBuffer->GetGPUVirtualAddress();

						// AlbedoMap
						if (vMaterials[0].m_bHasAlbedoMap)
							args.AlbedoMap = vTextures[vMaterials[0].m_nAlbedoMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
						else
							args.AlbedoMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
						// SpecularMap
						if (vMaterials[0].m_bHasSpecularMap)
							args.SpecularMap = vTextures[vMaterials[0].m_nSpecularMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
						else
							args.SpecularMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
						// NormalMap
						if (vMaterials[0].m_bHasNormalMap)
							args.NormalMap = vTextures[vMaterials[0].m_nNormalMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
						else
							args.NormalMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
						// MetallicMap
						if (vMaterials[0].m_bHasMetallicMap)
							args.MetallicMap = vTextures[vMaterials[0].m_nMetallicMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
						else
							args.MetallicMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
						// EmissionMap
						if (vMaterials[0].m_bHasEmissionMap)
							args.EmissionMap = vTextures[vMaterials[0].m_nEmissionMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
						else
							args.EmissionMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
						// DetailAlbedoMap
						if (vMaterials[0].m_bHasDetailAlbedoMap)
							args.DetailAlbedoMap = vTextures[vMaterials[0].m_nDetailAlbedoMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
						else
							args.DetailAlbedoMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();
						// DetailNormalMap
						if (vMaterials[0].m_bHasDetailNormalMap)
							args.DetailNormalMap = vTextures[vMaterials[0].m_nDetailNormalMapIndex]->getView()->GetGPUDescriptorHandleForHeapStart();
						else
							args.DetailNormalMap = m_pd3dNullBufferView->GetGPUDescriptorHandleForHeapStart();

						memcpy(ptrtemp, HitGroupIDs[j], D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
						ptrtemp = static_cast<char*>(ptrtemp) + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
						memcpy(ptrtemp, &args, sizeof(LocalRootArg));
						ptrStride = static_cast<char*>(ptrStride) + m_nHitGroupStride;
						ptrtemp = ptrStride;

						++nRecords;
					}
				}
			}
		}
		m_pHitGroupTable->Unmap(0, nullptr);
	}
}