#include "Particle.h"

constexpr unsigned int PARTICLE_EMITTER = 0;
constexpr unsigned int PARTICLE_FLAME = 1;

struct ParticleVertex {
	XMFLOAT3 position;
	XMFLOAT3 direction;
	float size;
	float lifeTime;
	UINT particleType;
};

CParticle::CParticle()
{
	m_nMaxVertex = 150;
	BufferReady();
}

CParticle::CParticle(UINT maxVertex)
	: m_nMaxVertex(maxVertex)
{
	if (m_nMaxVertex > 150)
		m_nMaxVertex = 150;
	BufferReady();
}

void CParticle::BufferReady()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = Align(sizeof(XMFLOAT4X4), 256);

	ID3D12Device5* device = g_DxResource.device;

	device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_WorldBuffer.GetAddressOf()));

	// Vertex Emitter
	desc.Width = sizeof(ParticleVertex);
	device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_VertexBuffer.GetAddressOf()));

	void* tempData{};

	m_VertexBuffer->Map(0, nullptr, &tempData);
	ParticleVertex tempV = { XMFLOAT3(0.0, 0.0, 0.0), XMFLOAT3(0.0, 1.0, 0.0), 0.0, 0.0, PARTICLE_EMITTER };
	memcpy(tempData, &tempV, sizeof(tempV));
	m_VertexBuffer->Unmap(0, nullptr);

	desc.Width = m_nMaxVertex * sizeof(ParticleVertex);
	device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_DrawBuffer.GetAddressOf()));

	device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_STREAM_OUT,
		nullptr, IID_PPV_ARGS(m_StreamOutputBuffer.GetAddressOf()));

	desc.Width = sizeof(UINT64);
	device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_STREAM_OUT,
		nullptr, IID_PPV_ARGS(m_FilledSizeBuffer.GetAddressOf()));
	device->CreateCommittedResource(&READBACK_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(m_ReadBackBuffer.GetAddressOf()));
	device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_UploadBuffer.GetAddressOf()));

	m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_Mappedptr));
}

XMFLOAT3 CParticle::getPosition()
{
	return XMFLOAT3(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43);
}

void CParticle::setPosition(XMFLOAT3& pos)
{
	m_WorldMatrix._41 = pos.x; m_WorldMatrix._42 = pos.y; m_WorldMatrix._43 = pos.z;
}

// =====================================================================================

CRaytracingParticle::CRaytracingParticle()
	: CParticle()
{
	BufferReady();
}

CRaytracingParticle::CRaytracingParticle(UINT maxVertex)
	: CParticle(maxVertex)
{
	BufferReady();
}

void CRaytracingParticle::BufferReady()
{
	ID3D12Device5* device = g_DxResource.device;
	auto desc = BASIC_BUFFER_DESC;

	desc.Width = Align(sizeof(HasMesh), 256);
	device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_MeshCB.GetAddressOf()));

	desc.Width = Align(sizeof(HasMaterial), 256);
	device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_MaterialCB.GetAddressOf()));

	desc.Width = m_nMaxVertex * 6 * sizeof(XMFLOAT3);
	device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_STREAM_OUT,
		nullptr, IID_PPV_ARGS(m_BillboardVertex.GetAddressOf()));

	desc.Width = m_nMaxVertex * 6 * sizeof(XMFLOAT2);
	device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_STREAM_OUT,
		nullptr, IID_PPV_ARGS(m_TexCoord0Buffer.GetAddressOf()));

	InitBLAS();
}

void CRaytracingParticle::InitBLAS()
{
	ID3D12Device5* device = g_DxResource.device;
	ID3D12CommandAllocator* cmdAlloc = g_DxResource.cmdAlloc;
	ID3D12GraphicsCommandList4* cmdList = g_DxResource.cmdList;
	ID3D12CommandQueue* cmdQueue = g_DxResource.cmdQueue;

	D3D12_RAYTRACING_GEOMETRY_DESC desc{};
	desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
	desc.Triangles.Transform3x4 = 0;
	desc.Triangles.VertexBuffer = {
		.StartAddress = m_BillboardVertex->GetGPUVirtualAddress(),
		.StrideInBytes = sizeof(float) * 3
	};
	desc.Triangles.VertexCount = m_nMaxVertex * 6;
	desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	desc.Triangles.IndexBuffer = 0;
	desc.Triangles.IndexCount = 0;
	desc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &desc;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo{};
	device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &preBuildInfo);

	auto bdesc = BASIC_BUFFER_DESC;
	bdesc.Width = preBuildInfo.ResultDataMaxSizeInBytes;
	bdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &bdesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		nullptr, IID_PPV_ARGS(m_BLAS.GetAddressOf()));

	bdesc.Width = preBuildInfo.ScratchDataSizeInBytes;
	device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &bdesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		nullptr, IID_PPV_ARGS(m_ScratchBuffer.GetAddressOf()));

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC ASDesc{};
	ASDesc.Inputs = inputs;
	ASDesc.DestAccelerationStructureData = m_BLAS->GetGPUVirtualAddress();
	ASDesc.ScratchAccelerationStructureData = m_ScratchBuffer->GetGPUVirtualAddress();

	cmdAlloc->Reset();
	cmdList->Reset(cmdAlloc, nullptr);
	cmdList->BuildRaytracingAccelerationStructure(&ASDesc, 0, nullptr);
	D3D12_RESOURCE_BARRIER d3dbr{};
	d3dbr.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	d3dbr.UAV.pResource = m_BLAS.Get();
	cmdList->ResourceBarrier(1, &d3dbr);
	cmdList->Close();
	cmdQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&cmdList));
	Flush();
}

void CRaytracingParticle::ReBuildBLAS()
{
	D3D12_RAYTRACING_GEOMETRY_DESC desc{};
	desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
	desc.Triangles.Transform3x4 = 0;
	desc.Triangles.VertexBuffer = {
		.StartAddress = m_BillboardVertex->GetGPUVirtualAddress(),
		.StrideInBytes = sizeof(float) * 3
	};
	desc.Triangles.VertexCount = m_nCurrentVertex * 6;
	desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	desc.Triangles.IndexBuffer = 0;
	desc.Triangles.IndexCount = 0;
	desc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	inputs.NumDescs = 1;
	inputs.pGeometryDescs = &desc;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC ASDesc{};
	ASDesc.Inputs = inputs;
	ASDesc.DestAccelerationStructureData = m_BLAS->GetGPUVirtualAddress();
	ASDesc.ScratchAccelerationStructureData = m_ScratchBuffer->GetGPUVirtualAddress();

	g_DxResource.cmdList->BuildRaytracingAccelerationStructure(&ASDesc, 0, nullptr);
	D3D12_RESOURCE_BARRIER d3dbr{};
	d3dbr.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	d3dbr.UAV.pResource = m_BLAS.Get();
	g_DxResource.cmdList->ResourceBarrier(1, &d3dbr);
}