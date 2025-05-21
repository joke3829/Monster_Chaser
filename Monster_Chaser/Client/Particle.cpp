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
	m_nMaxVertex = 200;
	BufferReady();
}

CParticle::CParticle(UINT maxVertex)
	: m_nMaxVertex(maxVertex)
{
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