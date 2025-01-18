#include "AccelerationStructureManager.h"

void CAccelerationStructureManager::SetScene()
{
	g_DxResource.cmdList->SetComputeRootShaderResourceView(m_nRootParameterIndex, m_TLAS->GetGPUVirtualAddress());
}

void CAccelerationStructureManager::UpdateScene()
{
	Flush();
}

// BLASList에 BLAS를 추가한다.
void CAccelerationStructureManager::AddBLAS(ID3D12Resource* vertexBuffer, UINT vertexcount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
	ID3D12Resource* indexBuffer, UINT indices, DXGI_FORMAT indexFormat)
{
	ComPtr<ID3D12Resource> blas;
	MakeBLAS(blas, vertexBuffer, vertexcount, vertexStride, vertexFormat,
		indexBuffer, indices, indexFormat);

	m_vBLASList.push_back(blas);
}

// BLAS를 만든다, 현재는 BLAS 하나당 하나의 SubObject를 담는다.
// 추후 모델의 포멧을 보고 수정 예정
void CAccelerationStructureManager::MakeBLAS(ComPtr<ID3D12Resource>& asResource,
	ID3D12Resource* vertexBuffer, UINT vertexCount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
	ID3D12Resource* indexBuffer, UINT indices, DXGI_FORMAT indexFormat)
{
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = vertexStride;
	geometryDesc.Triangles.VertexCount = vertexCount;
	geometryDesc.Triangles.VertexFormat = vertexFormat;
	geometryDesc.Triangles.IndexBuffer = indexBuffer ? indexBuffer->GetGPUVirtualAddress() : 0;
	geometryDesc.Triangles.IndexCount = indices;
	geometryDesc.Triangles.IndexFormat = indexFormat;


	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;	// 수정 불가능
	inputs.NumDescs = 1;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.pGeometryDescs = &geometryDesc;

	MakeAccelerationStructure(inputs, asResource);
}

void CAccelerationStructureManager::MakeTLAS()
{
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * m_nInstances;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_InstanceBuffer));
	m_InstanceBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pInstanceData));
	// 여기는 확정이 아님
	for (UINT i = 0; i < m_nInstances; ++i) {
		m_pInstanceData[i] = {
			.InstanceID = i,
			.InstanceMask = 1,
			.AccelerationStructure = m_vBLASList[i]->GetGPUVirtualAddress()
		};
	}


	UINT64 updateScratchSize{};

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = m_nInstances;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.InstanceDescs = m_InstanceBuffer->GetGPUVirtualAddress();

	MakeAccelerationStructure(inputs, m_TLAS, &updateScratchSize);

	// 스크래치 버퍼 만들기
	desc = BASIC_BUFFER_DESC;
	desc.Width = std::max<UINT64>(updateScratchSize, 8ULL);
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	g_DxResource.device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_TLASUpdateScratch));
}

// AS 생성
void CAccelerationStructureManager::MakeAccelerationStructure(D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs, ComPtr<ID3D12Resource>& asResource, UINT64* updateScratchSize)
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
		*updateScratchSize = preBuildInfo.UpdateScratchDataSizeInBytes;

	makeBuffer(asResource, preBuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);

	ComPtr<ID3D12Resource> ScratchBuffer{};
	makeBuffer(ScratchBuffer, preBuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_GENERIC_READ);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC ASDesc{};
	ASDesc.Inputs = inputs;
	ASDesc.DestAccelerationStructureData = asResource->GetGPUVirtualAddress();
	ASDesc.ScratchAccelerationStructureData = ScratchBuffer->GetGPUVirtualAddress();

	cmdAlloc->Reset();
	cmdList->Reset(cmdAlloc, nullptr);
	cmdList->BuildRaytracingAccelerationStructure(&ASDesc, 0, nullptr);
	cmdList->Close();
	cmdQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&cmdList));
	Flush();
}