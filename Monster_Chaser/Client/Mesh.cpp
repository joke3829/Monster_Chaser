#include "Mesh.h"


Mesh::Mesh(std::ifstream& inFile)
{
	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read((char*)&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};
	// �̸� �ޱ�
	GetMeshNameFromFile(inFile);

	while (1) {
		readLabel();
		if (strLabel == "<Bounds>:")
			GetBoundInfoFromFile(inFile);
		else if (strLabel == "<Positions>:")
			GetPositionFromFile(inFile);
		else if (strLabel == "</Mesh>")
			break;
	}
}

void Mesh::MakeResourceView(ComPtr<ID3D12DescriptorHeap>& pDescriptor, ComPtr<ID3D12Resource>& pResource)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	g_DxResource.device->CreateDescriptorHeap()
}


void Mesh::GetMeshNameFromFile(std::ifstream& inFile)
{
	int temp;	// ������ ��
	inFile.read((char*)&temp, sizeof(int));

	char nStrLength{};
	inFile.read((char*)&nStrLength, sizeof(char));
	m_MeshName.assign(nStrLength, ' ');
	inFile.read(m_MeshName.data(), nStrLength);
}

void Mesh::GetBoundInfoFromFile(std::ifstream& inFile)
{

}

// ���� ������ ������ ���ÿ� vertexbuffer ����
void Mesh::GetPositionFromFile(std::ifstream& inFile)
{
	// ���Ͽ��� ���� �б�
	inFile.read((char*)&m_nVertexCount, sizeof(int));	// ������ ���� �б�
	if (m_nVertexCount > 0) {
		m_bHasVertex = true;

		std::vector<XMFLOAT3> vPositions;
		vPositions.assign(m_nVertexCount, XMFLOAT3(0.0, 0.0, 0.0));
		inFile.read((char*)vPositions.data(), sizeof(XMFLOAT3) * m_nVertexCount);

		// vertex buffer ����
		auto desc = BASIC_BUFFER_DESC;
		desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		desc.Width = sizeof(XMFLOAT3) * m_nVertexCount;
		// �ϴ��� UPLOAD�� ����, ���� �� DEFAULT�� ���� ����
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dVertexBuffer.GetAddressOf()));

		void* ptr;
		m_pd3dVertexBuffer->Map(0, nullptr, &ptr);
		memcpy(ptr, vPositions.data(), sizeof(XMFLOAT3) * m_nVertexCount);
		m_pd3dVertexBuffer->Unmap(0, nullptr);
	}
}