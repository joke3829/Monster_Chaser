#include "Mesh.h"

CHeightMapImage::CHeightMapImage(const wchar_t* filePath, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	std::unique_ptr<BYTE[]> pHeightMapPixels = std::make_unique<BYTE[]>(m_nWidth * m_nLength);
	HANDLE hFile = ::CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels.get(), (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = std::make_unique<BYTE[]>(m_nWidth * m_nLength);
	for (int z = 0; z < m_nLength; ++z) {
		for (int x = 0; x < m_nWidth; ++x) {
			m_pHeightMapPixels[x + ((m_nLength - 1 - z) * m_nWidth)] = pHeightMapPixels[x + (z * m_nWidth)];
		}
	}
}

float CHeightMapImage::GetHeight(int x, int z)
{
	//float f = m_pHeightMapPixels[x + (z * m_nWidth)];
	return m_pHeightMapPixels[x + (z * m_nWidth)];
}

Mesh::Mesh(std::ifstream& inFile, std::string strMeshName)
{
	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};
	// 이름 받기
	//GetMeshNameFromFile(inFile);
	m_MeshName = strMeshName;

	while (1) {
		readLabel();
		if (strLabel == "<Bounds>:")
			GetBoundInfoFromFile(inFile);
		else if (strLabel == "<Positions>:")
			GetPositionFromFile(inFile);
		else if (strLabel == "<Colors>:")
			GetColorsFromFile(inFile);
		else if (strLabel == "<TextureCoords0>:")
			GetTexCoord0FromFile(inFile);
		else if (strLabel == "<TextureCoords1>:")
			GetTexCoord1FromFile(inFile);
		else if (strLabel == "<Normals>:")
			GetNormalFromFile(inFile);
		else if (strLabel == "<Tangents>:")
			GetTangentFromFile(inFile);
		else if (strLabel == "<BiTangents>:")
			GetBiTangentFromFile(inFile);
		else if (strLabel == "<SubMeshes>:")
			GetSubMeshesFromFile(inFile);
		else if (strLabel == "</Mesh>")
			break;
	}
}

Mesh::Mesh(CHeightMapImage* heightmap, std::string strMeshName)
{
	std::vector<XMFLOAT3> test{};
}

//void Mesh::GetMeshNameFromFile(std::ifstream& inFile)
//{
//	int temp;	// 정점의 수
//	inFile.read((char*)&temp, sizeof(int));
//
//	char nStrLength{};
//	inFile.read((char*)&nStrLength, sizeof(char));
//	m_MeshName.assign(nStrLength, ' ');
//	inFile.read(m_MeshName.data(), nStrLength);
//}

void Mesh::GetBoundInfoFromFile(std::ifstream& inFile)
{
	XMFLOAT3 OBBCenter{}; XMFLOAT3 OBBExtent{};
	inFile.read((char*)&OBBCenter, sizeof(XMFLOAT3));
	inFile.read((char*)&OBBExtent, sizeof(XMFLOAT3));
	m_OBB = BoundingOrientedBox(OBBCenter, OBBExtent, XMFLOAT4(0.0, 0.0, 0.0, 1.0));
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

void Mesh::GetColorsFromFile(std::ifstream& inFile)
{
	inFile.read((char*)&m_nColorCount, sizeof(int));
	if (m_nColorCount > 0) {
		m_bHasColor = true;
		std::vector<XMFLOAT4> vColors;

		vColors.assign(m_nColorCount, XMFLOAT4(0.0, 0.0, 0.0, 1.0));
		inFile.read((char*)vColors.data(), sizeof(XMFLOAT4) * m_nColorCount);

		// CreateBuffer
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = sizeof(XMFLOAT4) * m_nColorCount;
		// 일단은 UPLOAD로 생성, 문제 시 DEFAULT로 변경 예정
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dColorsBuffer.GetAddressOf()));

		void* ptr;
		m_pd3dColorsBuffer->Map(0, nullptr, &ptr);
		memcpy(ptr, vColors.data(), sizeof(XMFLOAT4) * m_nColorCount);
		m_pd3dColorsBuffer->Unmap(0, nullptr);
	}
}

void Mesh::GetTexCoord0FromFile(std::ifstream& inFile)
{
	inFile.read((char*)&m_nTexCoord0Count, sizeof(int));
	if (m_nTexCoord0Count > 0) {
		m_bHasTex0 = true;
		std::vector<XMFLOAT2> uv;
		uv.assign(m_nTexCoord0Count, XMFLOAT2(0.0, 0.0));
		inFile.read((char*)uv.data(), sizeof(XMFLOAT2) * m_nTexCoord0Count);

		// CreateBuffer
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = sizeof(XMFLOAT2) * m_nTexCoord0Count;
		// 일단은 UPLOAD로 생성, 문제 시 DEFAULT로 변경 예정
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dTexCoord0Buffer.GetAddressOf()));

		void* ptr;
		m_pd3dTexCoord0Buffer->Map(0, nullptr, &ptr);
		memcpy(ptr, uv.data(), sizeof(XMFLOAT2) * m_nTexCoord0Count);
		m_pd3dTexCoord0Buffer->Unmap(0, nullptr);
	}
}

void Mesh::GetTexCoord1FromFile(std::ifstream& inFile)
{
	inFile.read((char*)&m_nTexCoord1Count, sizeof(int));
	if (m_nTexCoord1Count > 0) {
		m_bHasTex1 = true;
		std::vector<XMFLOAT2> uv;
		uv.assign(m_nTexCoord1Count, XMFLOAT2(0.0, 0.0));
		inFile.read((char*)uv.data(), sizeof(XMFLOAT2) * m_nTexCoord1Count);

		// CreateBuffer
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = sizeof(XMFLOAT2) * m_nTexCoord1Count;
		// 일단은 UPLOAD로 생성, 문제 시 DEFAULT로 변경 예정
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dTexCoord1Buffer.GetAddressOf()));

		void* ptr;
		m_pd3dTexCoord1Buffer->Map(0, nullptr, &ptr);
		memcpy(ptr, uv.data(), sizeof(XMFLOAT2) * m_nTexCoord1Count);
		m_pd3dTexCoord1Buffer->Unmap(0, nullptr);
	}
}
void Mesh::GetNormalFromFile(std::ifstream& inFile)
{
	inFile.read((char*)&m_nNormalsCount, sizeof(int));
	if (m_nNormalsCount > 0) {
		m_bHasNormals = true;
		std::vector<XMFLOAT3> uv;
		uv.assign(m_nNormalsCount, XMFLOAT3(0.0, 0.0, 0.0));
		inFile.read((char*)uv.data(), sizeof(XMFLOAT3) * m_nNormalsCount);

		// CreateBuffer
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = sizeof(XMFLOAT3) * m_nNormalsCount;
		// 일단은 UPLOAD로 생성, 문제 시 DEFAULT로 변경 예정
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dNormalsBuffer.GetAddressOf()));

		void* ptr;
		m_pd3dNormalsBuffer->Map(0, nullptr, &ptr);
		memcpy(ptr, uv.data(), sizeof(XMFLOAT3) * m_nNormalsCount);
		m_pd3dNormalsBuffer->Unmap(0, nullptr);
	}
}

void Mesh::GetTangentFromFile(std::ifstream& inFile)
{
	inFile.read((char*)&m_nTangentsCount, sizeof(int));
	if (m_nTangentsCount > 0) {
		m_bHasTangents = true;
		std::vector<XMFLOAT3> uv;
		uv.assign(m_nTangentsCount, XMFLOAT3(0.0, 0.0, 0.0));
		inFile.read((char*)uv.data(), sizeof(XMFLOAT3) * m_nTangentsCount);

		// CreateBuffer
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = sizeof(XMFLOAT3) * m_nTangentsCount;
		// 일단은 UPLOAD로 생성, 문제 시 DEFAULT로 변경 예정
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dTangentsBuffer.GetAddressOf()));

		void* ptr;
		m_pd3dTangentsBuffer->Map(0, nullptr, &ptr);
		memcpy(ptr, uv.data(), sizeof(XMFLOAT3) * m_nTangentsCount);
		m_pd3dTangentsBuffer->Unmap(0, nullptr);
	}
}

void Mesh::GetBiTangentFromFile(std::ifstream& inFile)
{
	inFile.read((char*)&m_nBiTangentsCount, sizeof(int));
	if (m_nBiTangentsCount > 0) {
		m_bHasBiTangents = true;
		std::vector<XMFLOAT3> uv;
		uv.assign(m_nBiTangentsCount, XMFLOAT3(0.0, 0.0, 0.0));
		inFile.read((char*)uv.data(), sizeof(XMFLOAT3) * m_nBiTangentsCount);

		// CreateBuffer
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = sizeof(XMFLOAT3) * m_nBiTangentsCount;
		// 일단은 UPLOAD로 생성, 문제 시 DEFAULT로 변경 예정
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dBiTangentsBuffer.GetAddressOf()));

		void* ptr;
		m_pd3dBiTangentsBuffer->Map(0, nullptr, &ptr);
		memcpy(ptr, uv.data(), sizeof(XMFLOAT3) * m_nBiTangentsCount);
		m_pd3dBiTangentsBuffer->Unmap(0, nullptr);
	}
}

void Mesh::GetSubMeshesFromFile(std::ifstream& inFile)
{
	inFile.read((char*)&m_nSubMeshesCount, sizeof(int));
	if (m_nSubMeshesCount > 0) {
		m_bHasSubMeshes = true;
		for (int i = 0; i < m_nSubMeshesCount; ++i)
			MakeSubMesh(inFile);
	}
}

void Mesh::MakeSubMesh(std::ifstream& inFile)
{
	std::string label{};
	char nStrLength{};

	ComPtr<ID3D12Resource> indexBuffer{};
	int subMeshIndex{};
	UINT indices{};
	
	// "<SubMesh>:"
	inFile.read(&nStrLength, sizeof(char));
	label.assign(nStrLength, ' ');
	inFile.read((char*)label.data(), nStrLength);
	
	inFile.read((char*)&subMeshIndex, sizeof(int));

	inFile.read((char*)&indices, sizeof(int));
	m_vIndices.push_back(indices);

	if (indices > 0) {
		std::vector<UINT> index{};
		index.assign(indices, 0);
		inFile.read((char*)index.data(), sizeof(UINT) * indices);

		// CreateBuffer
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = sizeof(UINT) * indices;
		// 일단은 UPLOAD로 생성, 문제 시 DEFAULT로 변경 예정
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(indexBuffer.GetAddressOf()));

		void* ptr;
		indexBuffer->Map(0, nullptr, &ptr);
		memcpy(ptr, index.data(), sizeof(UINT) * indices);
		indexBuffer->Unmap(0, nullptr);
	}
	m_vSubMeshes.push_back(indexBuffer);
}

// =============================== getter ====================================
std::string Mesh::getName() const
{
	return m_MeshName;
}
ID3D12Resource* Mesh::getVertexBuffer() const
{
	return m_pd3dVertexBuffer.Get();
}

ID3D12Resource* Mesh::getColorsBuffer() const
{
	return m_pd3dColorsBuffer.Get();
}

ID3D12Resource* Mesh::getTexCoord0Buffer() const
{
	return m_pd3dTexCoord0Buffer.Get();
}
ID3D12Resource* Mesh::getTexCoord1Buffer() const
{
	return m_pd3dTexCoord1Buffer.Get();
}
ID3D12Resource* Mesh::getNormalsBuffer() const
{
	return m_pd3dNormalsBuffer.Get();
}
ID3D12Resource* Mesh::getTangentsBuffer() const
{
	return m_pd3dTangentsBuffer.Get();
}
ID3D12Resource* Mesh::getBiTangentsBuffer() const
{
	return m_pd3dBiTangentsBuffer.Get();
}
ID3D12Resource* Mesh::getIndexBuffer(UINT index) const
{
	return m_vSubMeshes[index].Get();
}

UINT Mesh::getVertexCount() const
{
	return m_nVertexCount;
}
UINT Mesh::getIndexCount(int index) const
{
	return m_vIndices[index];
}

bool Mesh::getHasVertex() const
{
	return m_bHasVertex;
}

bool Mesh::getHasColor() const
{
	return m_bHasColor;
}
bool Mesh::getHasTex0() const
{
	return m_bHasTex0;
}
bool Mesh::getHasTex1() const
{
	return m_bHasTex1;
}
bool Mesh::getHasNormal() const
{
	return m_bHasNormals;
}
bool Mesh::getHasTangent() const
{
	return m_bHasTangents;
}
bool Mesh::getHasBiTangent() const
{
	return m_bHasBiTangents;
}
bool Mesh::getHasSubmesh() const
{
	return m_bHasSubMeshes;
}

UINT Mesh::getSubMeshCount() const
{
	return m_nSubMeshesCount;
}

// ==========================================================================