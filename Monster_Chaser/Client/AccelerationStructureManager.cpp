#include "AccelerationStructureManager.h"

void CAccelerationStructureManager::Setup(CResourceManager* resourceManager, UINT parameterIndex)
{
	m_pResourceManager = resourceManager;
	m_nRootParameterIndex = parameterIndex;
}

void CAccelerationStructureManager::SetScene()
{
	g_DxResource.cmdList->SetComputeRootShaderResourceView(m_nRootParameterIndex, m_TLAS->GetGPUVirtualAddress());
}

void CAccelerationStructureManager::UpdateScene()
{
	std::vector<std::unique_ptr<CGameObject>>& objects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	int i = m_nStaticMesh;
	if (m_bFirst) {
		for (std::unique_ptr<CGameObject>& object : objects) {
			int n = object->getMeshIndex();
			if (n != -1) {
				if (meshes[n]->getHasVertex()) {
					m_pInstanceData[i].AccelerationStructure = m_vBLASList[n]->GetGPUVirtualAddress();
					m_pInstanceData[i].InstanceContributionToHitGroupIndex = object->getHitGroupIndex();
					m_pInstanceData[i].InstanceID = i;
					m_pInstanceData[i].InstanceMask = 1;
					m_pInstanceData[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
					auto* ptr = reinterpret_cast<XMFLOAT3X4*>(&m_pInstanceData[i].Transform);
					XMStoreFloat3x4(ptr, XMLoadFloat4x4(&object->getWorldMatrix()));
					++i;
				}
			}
		}
		m_nStaticMesh = i;
		m_bFirst = false;
	}
	for (std::unique_ptr<CSkinningObject>& Skinning : m_pResourceManager->getSkinningObjectList()) {
		std::vector<ComPtr<ID3D12Resource>>& skinningBLASs = Skinning->getBLAS();
		std::vector<std::shared_ptr<Mesh>>& sMeshes = Skinning->getMeshes();
		for (std::unique_ptr<CGameObject>& object : Skinning->getObjects()) {
			int n = object->getMeshIndex();
			if (n != -1) {
				if (sMeshes[n]->getHasVertex()) {
					m_pInstanceData[i].AccelerationStructure = skinningBLASs[n]->GetGPUVirtualAddress();
					m_pInstanceData[i].InstanceContributionToHitGroupIndex = object->getHitGroupIndex();
					m_pInstanceData[i].InstanceID = i;
					m_pInstanceData[i].InstanceMask = 1;
					m_pInstanceData[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
					auto* ptr = reinterpret_cast<XMFLOAT3X4*>(&m_pInstanceData[i].Transform);
					XMStoreFloat3x4(ptr, XMLoadFloat4x4(&object->getWorldMatrix()));
					++i;
				}
			}
		}
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC desc = {
		.DestAccelerationStructureData = m_TLAS->GetGPUVirtualAddress(),
		.Inputs = {
			.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
			.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE,
			.NumDescs = m_nValidObject,
			.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY,
			.InstanceDescs = m_InstanceBuffer->GetGPUVirtualAddress()
		},
		.SourceAccelerationStructureData = m_TLAS->GetGPUVirtualAddress(),
		.ScratchAccelerationStructureData = m_tlasUpdataeScratch->GetGPUVirtualAddress()
	};

	g_DxResource.cmdList->BuildRaytracingAccelerationStructure(&desc, 0, nullptr);
	D3D12_RESOURCE_BARRIER barrier = { .Type = D3D12_RESOURCE_BARRIER_TYPE_UAV, .UAV = {.pResource = m_TLAS.Get()} };
	g_DxResource.cmdList->ResourceBarrier(1, &barrier);
}

// BLASList에 BLAS를 추가한다.
//void CAccelerationStructureManager::AddBLAS(ID3D12Resource* vertexBuffer, UINT vertexcount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
//	ID3D12Resource* indexBuffer, UINT indices, DXGI_FORMAT indexFormat)
//{
//	ComPtr<ID3D12Resource> blas;
//	MakeBLAS(blas, vertexBuffer, vertexcount, vertexStride, vertexFormat,
//		indexBuffer, indices, indexFormat);
//
//	m_vBLASList.push_back(blas);
//}

void CAccelerationStructureManager::InitBLAS()
{
	std::vector<std::unique_ptr<Mesh>>& vMeshes = m_pResourceManager->getMeshList();
	for (std::unique_ptr<Mesh>& mesh : vMeshes) {
		ComPtr<ID3D12Resource> blas{};
		if (mesh->getHasVertex()) {
			MakeBLAS(blas, mesh);
		}
		m_vBLASList.emplace_back(blas);
	}
}

void CAccelerationStructureManager::MakeBLAS(ComPtr<ID3D12Resource>& resource, std::unique_ptr<Mesh>& mesh)
{
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> GeometryDesc{};
	if (mesh->getHasSubmesh()) {
		int nSubMesh = mesh->getSubMeshCount();
		for (int i = 0; i < nSubMesh; ++i) {
			D3D12_RAYTRACING_GEOMETRY_DESC desc{};
			desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;		// 임시
			desc.Triangles.Transform3x4 = 0;
			desc.Triangles.VertexBuffer = {
				.StartAddress = mesh->getVertexBuffer()->GetGPUVirtualAddress(),
				.StrideInBytes = sizeof(float) * 3 
			};
			desc.Triangles.VertexCount = mesh->getVertexCount();
			desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			desc.Triangles.IndexBuffer = mesh->getIndexBuffer(i)->GetGPUVirtualAddress();
			desc.Triangles.IndexCount = mesh->getIndexCount(i);
			desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;

			GeometryDesc.emplace_back(desc);
		}
	}
	else {
		D3D12_RAYTRACING_GEOMETRY_DESC desc{};
		desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;		// 임시
		desc.Triangles.Transform3x4 = 0;
		desc.Triangles.VertexBuffer = {
			.StartAddress = mesh->getVertexBuffer()->GetGPUVirtualAddress(),
			.StrideInBytes = sizeof(float) * 3
		};
		desc.Triangles.VertexCount = mesh->getVertexCount();
		desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		desc.Triangles.IndexBuffer = 0;
		desc.Triangles.IndexCount = 0;
		desc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;

		GeometryDesc.emplace_back(desc);
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = GeometryDesc.size();
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.pGeometryDescs = GeometryDesc.data();

	MakeAccelerationStructure(inputs, resource);
}

// BLAS를 만든다, 현재는 BLAS 하나당 하나의 SubObject를 담는다.
// 추후 모델의 포멧을 보고 수정 예정
//void CAccelerationStructureManager::MakeBLAS(ComPtr<ID3D12Resource>& asResource,
//	ID3D12Resource* vertexBuffer, UINT vertexCount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
//	ID3D12Resource* indexBuffer, UINT indices, DXGI_FORMAT indexFormat, bool bOpaque)
//{
//	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
//	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
//	geometryDesc.Flags = bOpaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
//	geometryDesc.Triangles.Transform3x4 = 0;
//	geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
//	geometryDesc.Triangles.VertexBuffer.StrideInBytes = vertexStride;
//	geometryDesc.Triangles.VertexCount = vertexCount;
//	geometryDesc.Triangles.VertexFormat = vertexFormat;
//	geometryDesc.Triangles.IndexBuffer = indexBuffer ? indexBuffer->GetGPUVirtualAddress() : 0;
//	geometryDesc.Triangles.IndexCount = indices;
//	geometryDesc.Triangles.IndexFormat = indexFormat;
//
//
//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
//	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
//	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;	// 수정 불가능
//	inputs.NumDescs = 1;
//	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
//	inputs.pGeometryDescs = &geometryDesc;
//
//	MakeAccelerationStructure(inputs, asResource);
//}

void CAccelerationStructureManager::InitTLAS()
{
	std::vector<std::unique_ptr<CGameObject>>& vObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<Mesh>>& vMeshes = m_pResourceManager->getMeshList();
	for (std::unique_ptr<CGameObject>& object : vObjects) {
		if (object->getMeshIndex() != -1) {
			if (vMeshes[object->getMeshIndex()]->getHasVertex())
				++m_nValidObject;
		}
	}
	for (std::unique_ptr<CSkinningObject>& Skinning : m_pResourceManager->getSkinningObjectList()) {
		std::vector<std::shared_ptr<Mesh>>& sMeshes = Skinning->getMeshes();
		for (std::unique_ptr<CGameObject>& object : Skinning->getObjects()) {
			if (object->getMeshIndex() != -1) {
				if (sMeshes[object->getMeshIndex()]->getHasVertex())
					++m_nValidObject;
			}
		}
	}

	auto instanceDesc = BASIC_BUFFER_DESC;
	instanceDesc.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_nValidObject;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &instanceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_InstanceBuffer.GetAddressOf()));
	m_InstanceBuffer->Map(0, nullptr, (void**)&m_pInstanceData);

	int i{};
	// static Mehs 지금 지정
	for (std::unique_ptr<CGameObject>& object : vObjects) {
		int n = object->getMeshIndex();
		if (n != -1) {
			if (vMeshes[n]->getHasVertex()) {
				m_pInstanceData[i].AccelerationStructure = m_vBLASList[n]->GetGPUVirtualAddress();
				m_pInstanceData[i].InstanceContributionToHitGroupIndex = object->getHitGroupIndex();
				m_pInstanceData[i].InstanceID = i;
				m_pInstanceData[i].InstanceMask = 1;
				m_pInstanceData[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
				auto* ptr = reinterpret_cast<XMFLOAT3X4*>(&m_pInstanceData[i].Transform);
				XMStoreFloat3x4(ptr, XMLoadFloat4x4(&object->getWorldMatrix()));	// 여기 주의
				++i;
			}
		}
	}
	for (std::unique_ptr<CSkinningObject>& Skinning : m_pResourceManager->getSkinningObjectList()) {
		std::vector<ComPtr<ID3D12Resource>>& skinningBLASs = Skinning->getBLAS();
		std::vector<std::shared_ptr<Mesh>>& sMeshes = Skinning->getMeshes();
		for (std::unique_ptr<CGameObject>& object : Skinning->getObjects()) {
			int n = object->getMeshIndex();
			if (n != -1) {
				if (sMeshes[n]->getHasVertex()) {
					m_pInstanceData[i].AccelerationStructure = skinningBLASs[n]->GetGPUVirtualAddress();
					m_pInstanceData[i].InstanceContributionToHitGroupIndex = object->getHitGroupIndex();
					m_pInstanceData[i].InstanceID = i;
					m_pInstanceData[i].InstanceMask = 1;
					m_pInstanceData[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
					auto* ptr = reinterpret_cast<XMFLOAT3X4*>(&m_pInstanceData[i].Transform);
					XMStoreFloat3x4(ptr, XMLoadFloat4x4(&object->getWorldMatrix()));	// 여기 주의
					++i;
				}
			}
		}
	}

	UINT64 updateScratchSize;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = m_nValidObject;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.InstanceDescs = m_InstanceBuffer->GetGPUVirtualAddress();

	MakeAccelerationStructure(inputs, m_TLAS, &updateScratchSize, true);

	auto desc = BASIC_BUFFER_DESC;
	desc.Width = (updateScratchSize >= 8ULL) ? updateScratchSize : 8ULL;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	g_DxResource.device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_tlasUpdataeScratch.GetAddressOf()));
}

// AS 생성
void CAccelerationStructureManager::MakeAccelerationStructure(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs, ComPtr<ID3D12Resource>& asResource, UINT64* updateScratchSize, bool allowUpdate)
{
	ID3D12Device5* device = g_DxResource.device;
	ID3D12CommandAllocator* cmdAlloc = g_DxResource.cmdAlloc;
	ID3D12GraphicsCommandList4* cmdList = g_DxResource.cmdList;
	ID3D12CommandQueue* cmdQueue = g_DxResource.cmdQueue; 

	auto makeBuffer = [&](ComPtr<ID3D12Resource>& d3dResource, UINT bufferSize, auto InitialState)
		{
			auto desc = BASIC_BUFFER_DESC;
			desc.Width = bufferSize;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc, InitialState, nullptr, IID_PPV_ARGS(&d3dResource));
		};

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo{};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &preBuildInfo);

	if (updateScratchSize)
		if (allowUpdate)
			*updateScratchSize = preBuildInfo.UpdateScratchDataSizeInBytes;
		else
			*updateScratchSize = preBuildInfo.ScratchDataSizeInBytes;

	makeBuffer(asResource, preBuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

	ComPtr<ID3D12Resource> ScratchBuffer{};
	makeBuffer(ScratchBuffer, preBuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_COMMON);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC ASDesc{};
	ASDesc.Inputs = inputs;
	ASDesc.DestAccelerationStructureData = asResource->GetGPUVirtualAddress();
	ASDesc.ScratchAccelerationStructureData = ScratchBuffer->GetGPUVirtualAddress();

	cmdAlloc->Reset();
	cmdList->Reset(cmdAlloc, nullptr);
	cmdList->BuildRaytracingAccelerationStructure(&ASDesc, 0, nullptr);
	D3D12_RESOURCE_BARRIER d3dbr{};
	d3dbr.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	d3dbr.UAV.pResource = asResource.Get();
	cmdList->ResourceBarrier(1, &d3dbr);
	cmdList->Close();
	cmdQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&cmdList));
	Flush();
}