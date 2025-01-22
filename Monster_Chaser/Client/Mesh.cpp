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
	// 이름 받기
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
	int temp;	// 정점의 수
	inFile.read((char*)&temp, sizeof(int));

	char nStrLength{};
	inFile.read((char*)&nStrLength, sizeof(char));
	m_MeshName.assign(nStrLength, ' ');
	inFile.read(m_MeshName.data(), nStrLength);
}

void Mesh::GetBoundInfoFromFile(std::ifstream& inFile)
{

}

// 정점 정보를 읽음과 동시에 vertexbuffer 생성
void Mesh::GetPositionFromFile(std::ifstream& inFile)
{
	// 파일에서 정점 읽기
	inFile.read((char*)&m_nVertexCount, sizeof(int));	// 정점의 개수 읽기
	if (m_nVertexCount > 0) {
		m_bHasVertex = true;

		std::vector<XMFLOAT3> vPositions;
		vPositions.assign(m_nVertexCount, XMFLOAT3(0.0, 0.0, 0.0));
		inFile.read((char*)vPositions.data(), sizeof(XMFLOAT3) * m_nVertexCount);

		// vertex buffer 생성
		auto desc = BASIC_BUFFER_DESC;
		desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		desc.Width = sizeof(XMFLOAT3) * m_nVertexCount;
		// 일단은 UPLOAD로 생성, 문제 시 DEFAULT로 변경 예정
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, 
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dVertexBuffer.GetAddressOf()));

		void* ptr;
		m_pd3dVertexBuffer->Map(0, nullptr, &ptr);
		memcpy(ptr, vPositions.data(), sizeof(XMFLOAT3) * m_nVertexCount);
		m_pd3dVertexBuffer->Unmap(0, nullptr);
	}
}