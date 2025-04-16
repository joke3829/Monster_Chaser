#include "Mesh.h"

CHeightMapImage::CHeightMapImage(const wchar_t* filePath, int nWidth, int nLength, XMFLOAT3& xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	
	std::ifstream inFile{ filePath, std::ios::binary };
	std::vector<WORD> v(m_nWidth * m_nLength);
	inFile.read((char*)v.data(), sizeof(WORD) * m_nLength * m_nWidth);

	m_pHeightMapPixels = std::make_unique<WORD[]>(m_nWidth * m_nLength);
	for (int z = 0; z < m_nLength; ++z) {
		for (int x = 0; x < m_nWidth; ++x) {
			m_pHeightMapPixels[x + ((m_nLength - 1 - z) * m_nWidth)] = v[x + (z * m_nWidth)];//pHeightMapPixels[x + (z * m_nWidth)];
		}
	}
}

float CHeightMapImage::GetHeight(int x, int z)
{
	return m_pHeightMapPixels[x + (z * m_nWidth)] * m_xmf3Scale.y;
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
	XMFLOAT3 xmf3Scale = heightmap->m_xmf3Scale;
	int Width = heightmap->m_nWidth; int Length = heightmap->m_nLength;
	std::vector<XMFLOAT3> vertex{};
	std::vector<XMFLOAT2> tex0{};
	std::vector<XMFLOAT2> tex1{};
	vertex.reserve(Length * Width);
	tex0.reserve(Length * Width);
	tex1.reserve(Length * Width);
	std::vector<XMFLOAT3> normals(Length * Width, { 0.0f, 1.0f, 0.0f });

	auto f = [&](XMVECTOR& n, XMFLOAT3& n1, XMFLOAT3& n2, XMFLOAT3& n3) {
		XMVECTOR v1 = XMVector3Normalize(XMLoadFloat3(&n2) - XMLoadFloat3(&n1));
		XMVECTOR v2 = XMVector3Normalize(XMLoadFloat3(&n3) - XMLoadFloat3(&n1));
		n += XMVector3Cross(v1, v2);
		};

	float fHeight;
	for (int z = 0; z < Length; ++z) {
		for (int x = 0; x < Width; ++x) {
			fHeight = heightmap->GetHeight(x, z);
			vertex.emplace_back(x * xmf3Scale.x, fHeight, z * xmf3Scale.z);
			tex0.emplace_back(float(x) / float(Width - 1), float(Length - 1 - z) / float(Length - 1));
			tex1.emplace_back(float(x) / float(xmf3Scale.x * 0.5f), float(z) / float(xmf3Scale.z * 0.5f));

			if (0 == x || Width - 1 == x || 0 == z || Length - 1 == z)
				continue;

			XMVECTOR N{};
			XMFLOAT3 v1{ x * xmf3Scale.x , heightmap->GetHeight(x, z - 1),  (z - 1) * xmf3Scale.z };
			XMFLOAT3 v2{ (x + 1) * xmf3Scale.x , heightmap->GetHeight(x + 1, z - 1),  (z - 1)* xmf3Scale.z };
			XMFLOAT3 v3{ (x - 1) * xmf3Scale.x , heightmap->GetHeight(x - 1, z),  z * xmf3Scale.z };
			XMFLOAT3 v4{ x * xmf3Scale.x , heightmap->GetHeight(x, z),  z * xmf3Scale.z };
			XMFLOAT3 v5{ (x + 1) * xmf3Scale.x , heightmap->GetHeight(x + 1, z),  z * xmf3Scale.z };
			XMFLOAT3 v6{ (x - 1) * xmf3Scale.x , heightmap->GetHeight(x - 1, z + 1),  (z + 1) * xmf3Scale.z };
			XMFLOAT3 v7{ x * xmf3Scale.x , heightmap->GetHeight(x, z + 1),  (z + 1) * xmf3Scale.z };

			f(N, v3, v4, v1); f(N, v1, v4, v2); f(N, v4, v5, v2);
			f(N, v3, v6, v4); f(N, v6, v7, v4); f(N, v4, v7, v5);

			XMStoreFloat3(&normals[x + z * Length], N);
		}
	}

	std::vector<UINT> index{};

	for (int z = 0; z < Length - 1; ++z) {
		for (int x = 0; x < Width - 1; ++x) {
			index.emplace_back(x + (z * Width));
			index.emplace_back(x + (z * Width) + Width);
			index.emplace_back(x + (z * Width) + 1);

			index.emplace_back(x + (z * Width) + 1);
			index.emplace_back(x + (z * Width) + Width);
			index.emplace_back(x + (z * Width) + 1 + Width);
		}
	}

	// Calculate Normals
	
	unsigned int remainderW, quotientH;
	for (unsigned int i = 0; i < vertex.size(); ++i) {
		remainderW = i % Width; quotientH = i / Length;
		if (0 == remainderW || Width - 1 == remainderW || 0 == quotientH || Length - 1 == quotientH)
			continue;
		short cnt{};
		/*XMVECTOR vertexNormal{};
		for (unsigned int j = 0; j < index.size() && cnt < 6; j += 3) {
			if (i == index[j] || i == index[j + 1] || i == index[j + 2]) {
				XMVECTOR v1 = XMVector3Normalize(XMLoadFloat3(&vertex[index[j + 1]]) - XMLoadFloat3(&vertex[index[j]]));
				XMVECTOR v2 = XMVector3Normalize(XMLoadFloat3(&vertex[index[j + 2]]) - XMLoadFloat3(&vertex[index[j]]));
				XMVECTOR N = XMVector3Cross(v1, v2);
				vertexNormal += N;
				++cnt;
			}
		}
		XMStoreFloat3(&normals[i], XMVector3Normalize(vertexNormal));*/
	}



	// Calculate Tangents and BiTangents
	std::vector<XMFLOAT3> tangents(vertex.size(), { 0.0f, 0.0f, 0.0f });
	std::vector<XMFLOAT3> bitangents(vertex.size(), { 0.0f, 0.0f, 0.0f });
	for (size_t i = 0; i < index.size(); i += 3) {
		UINT index0 = index[i];
		UINT index1 = index[i + 1];
		UINT index2 = index[i + 2];

		XMVECTOR v0 = XMLoadFloat3(&vertex[index0]);
		XMVECTOR v1 = XMLoadFloat3(&vertex[index1]);
		XMVECTOR v2 = XMLoadFloat3(&vertex[index2]);

		XMFLOAT2 uv0 = tex0[index0];
		XMFLOAT2 uv1 = tex0[index1];
		XMFLOAT2 uv2 = tex0[index2];

		XMVECTOR deltaPos1 = v1 - v0;
		XMVECTOR deltaPos2 = v2 - v0;

		float deltaU1 = uv1.x - uv0.x;
		float deltaV1 = uv1.y - uv0.y;
		float deltaU2 = uv2.x - uv0.x;
		float deltaV2 = uv2.y - uv0.y;

		float r = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);
		XMVECTOR tangent = (deltaPos1 * deltaV2 - deltaPos2 * deltaV1) * r;
		XMVECTOR bitangent = (deltaPos2 * deltaU1 - deltaPos1 * deltaU2) * r;

		XMFLOAT3 faceTangent, faceBitangent;
		XMStoreFloat3(&faceTangent, XMVector3Normalize(tangent));
		XMStoreFloat3(&faceBitangent, XMVector3Normalize(bitangent));

		tangents[index0].x += faceTangent.x; tangents[index0].y += faceTangent.y; tangents[index0].z += faceTangent.z;
		tangents[index1].x += faceTangent.x; tangents[index1].y += faceTangent.y; tangents[index1].z += faceTangent.z;
		tangents[index2].x += faceTangent.x; tangents[index2].y += faceTangent.y; tangents[index2].z += faceTangent.z;

		bitangents[index0].x += faceBitangent.x; bitangents[index0].y += faceBitangent.y; bitangents[index0].z += faceBitangent.z;
		bitangents[index1].x += faceBitangent.x; bitangents[index1].y += faceBitangent.y; bitangents[index1].z += faceBitangent.z;
		bitangents[index2].x += faceBitangent.x; bitangents[index2].y += faceBitangent.y; bitangents[index2].z += faceBitangent.z;
	}

	for (size_t i = 0; i < tangents.size(); ++i) {
		XMVECTOR tangentVec = XMLoadFloat3(&tangents[i]);
		tangentVec = XMVector3Normalize(tangentVec);
		XMStoreFloat3(&tangents[i], tangentVec);

		XMVECTOR bitangentVec = XMLoadFloat3(&bitangents[i]);
		bitangentVec = XMVector3Normalize(bitangentVec);
		XMStoreFloat3(&bitangents[i], bitangentVec);
	}


	void* tempData{};
	m_bHasVertex = true; m_nVertexCount = vertex.size();
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = sizeof(XMFLOAT3) * m_nVertexCount;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_pd3dVertexBuffer.GetAddressOf()));
	m_pd3dVertexBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, vertex.data(), sizeof(XMFLOAT3) * m_nVertexCount);
	m_pd3dVertexBuffer->Unmap(0, nullptr);

	m_bHasTex0 = true; m_nTexCoord0Count = tex0.size();
	desc.Width = sizeof(XMFLOAT2) * m_nTexCoord0Count;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_pd3dTexCoord0Buffer.GetAddressOf()));
	m_pd3dTexCoord0Buffer->Map(0, nullptr, &tempData);
	memcpy(tempData, tex0.data(), sizeof(XMFLOAT2) * m_nTexCoord0Count);
	m_pd3dTexCoord0Buffer->Unmap(0, nullptr);

	m_bHasTex1 = true; m_nTexCoord1Count = tex1.size();
	desc.Width = sizeof(XMFLOAT2) * m_nTexCoord1Count;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_pd3dTexCoord1Buffer.GetAddressOf()));
	m_pd3dTexCoord1Buffer->Map(0, nullptr, &tempData);
	memcpy(tempData, tex1.data(), sizeof(XMFLOAT2) * m_nTexCoord1Count);
	m_pd3dTexCoord1Buffer->Unmap(0, nullptr);

	m_bHasNormals = true; m_nNormalsCount = normals.size();
	desc.Width = sizeof(XMFLOAT3) * m_nNormalsCount;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_pd3dNormalsBuffer.GetAddressOf()));
	m_pd3dNormalsBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, normals.data(), sizeof(XMFLOAT3)* m_nNormalsCount);
	m_pd3dNormalsBuffer->Unmap(0, nullptr);

	m_bHasTangents = true; m_nTangentsCount = tangents.size();
	desc.Width = sizeof(XMFLOAT3) * m_nTangentsCount;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_pd3dTangentsBuffer.GetAddressOf()));
	m_pd3dTangentsBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, tangents.data(), sizeof(XMFLOAT3)* m_nTangentsCount);
	m_pd3dTangentsBuffer->Unmap(0, nullptr);

	m_bHasBiTangents = true; m_nBiTangentsCount = bitangents.size();
	desc.Width = sizeof(XMFLOAT3) * m_nBiTangentsCount;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_pd3dBiTangentsBuffer.GetAddressOf()));
	m_pd3dBiTangentsBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, bitangents.data(), sizeof(XMFLOAT3)* m_nBiTangentsCount);
	m_pd3dBiTangentsBuffer->Unmap(0, nullptr);

	m_bHasSubMeshes = true;
	ComPtr<ID3D12Resource> tempBuffer{};
	desc.Width = sizeof(UINT) * index.size();
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(tempBuffer.GetAddressOf()));
	tempBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, index.data(), sizeof(UINT) * index.size());
	tempBuffer->Unmap(0, nullptr);

	++m_nSubMeshesCount;
	m_vIndices.emplace_back(index.size());
	m_vSubMeshes.emplace_back(tempBuffer);
}

Mesh::Mesh(XMFLOAT3& center, XMFLOAT3& extent, std::string meshName)
{
	m_MeshName = meshName;
	m_OBB = BoundingOrientedBox(center, extent, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	
	std::vector<XMFLOAT3> pos(8);
	pos[0] = XMFLOAT3(center.x - extent.x, center.y - extent.y, center.z - extent.z);
	pos[1] = XMFLOAT3(center.x + extent.x, center.y - extent.y, center.z - extent.z);
	pos[2] = XMFLOAT3(center.x + extent.x, center.y - extent.y, center.z + extent.z);
	pos[3] = XMFLOAT3(center.x - extent.x, center.y - extent.y, center.z + extent.z);
	pos[4] = XMFLOAT3(center.x - extent.x, center.y + extent.y, center.z - extent.z);
	pos[5] = XMFLOAT3(center.x + extent.x, center.y + extent.y, center.z - extent.z);
	pos[6] = XMFLOAT3(center.x + extent.x, center.y + extent.y, center.z + extent.z);
	pos[7] = XMFLOAT3(center.x - extent.x, center.y + extent.y, center.z + extent.z);


	void* tempData{};
	m_bHasVertex = true;
	m_nVertexCount = 8;
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = sizeof(XMFLOAT3) * 8;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(m_pd3dVertexBuffer.GetAddressOf()));
	m_pd3dVertexBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, pos.data(), sizeof(XMFLOAT3) * 8);
	m_pd3dVertexBuffer->Unmap(0, nullptr);

	std::vector<UINT> index(36);
	index[0] = 0; index[1] = 1; index[2] = 3; index[3] = 1; index[4] = 2; index[5] = 3;
	index[6] = 0; index[7] = 4; index[8] = 1; index[9] = 4; index[10] = 5; index[11] = 1;
	index[12] = 5; index[13] = 6; index[14] = 2; index[15] = 5; index[16] = 2; index[17] = 1;
	index[18] = 6; index[19] = 7; index[20] = 3; index[21] = 6; index[22] = 3; index[23] = 2;
	index[24] = 7; index[25] = 4; index[26] = 3; index[27] = 4; index[28] = 0; index[29] = 3;
	index[30] = 7; index[31] = 6; index[32] = 4; index[33] = 4; index[34] = 6; index[35] = 5;

	m_bHasSubMeshes = true;
	ComPtr<ID3D12Resource> tempBuffer{};
	desc.Width = sizeof(UINT) * 36;
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(tempBuffer.GetAddressOf()));
	tempBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, index.data(), sizeof(UINT) * 36);
	tempBuffer->Unmap(0, nullptr);

	++m_nSubMeshesCount;
	m_vIndices.emplace_back(36);
	m_vSubMeshes.emplace_back(tempBuffer);
}

Mesh::Mesh(XMFLOAT3& center, float radius, std::string meshName)
{
	m_MeshName = meshName;

	const UINT sliceCount = 80;
	const UINT stackCount = 80;

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT3> normals;
	std::vector<UINT> indices;

	// Top vertex
	positions.emplace_back(center.x, center.y + radius, center.z);
	normals.emplace_back(0.0f, 1.0f, 0.0f);

	// Generate vertices
	for (UINT stack = 1; stack < stackCount; ++stack)
	{
		float phi = XM_PI * stack / stackCount;
		float y = radius * cosf(phi);
		float r = radius * sinf(phi);

		for (UINT slice = 0; slice <= sliceCount; ++slice)
		{
			float theta = 2.0f * XM_PI * slice / sliceCount;
			float x = r * cosf(theta);
			float z = r * sinf(theta);

			positions.emplace_back(center.x + x, center.y + y, center.z + z);
			normals.emplace_back(x / radius, y / radius, z / radius);
		}
	}

	// Bottom vertex
	positions.emplace_back(center.x, center.y - radius, center.z);
	normals.emplace_back(0.0f, -1.0f, 0.0f);

	// Top cap indices
	for (UINT i = 1; i <= sliceCount; ++i)
	{
		indices.push_back(0);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	// Middle quads (converted to two triangles)
	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT stack = 0; stack < stackCount - 2; ++stack)
	{
		for (UINT slice = 0; slice < sliceCount; ++slice)
		{
			UINT a = baseIndex + stack * ringVertexCount + slice;
			UINT b = baseIndex + (stack + 1) * ringVertexCount + slice;
			UINT c = baseIndex + (stack + 1) * ringVertexCount + slice + 1;
			UINT d = baseIndex + stack * ringVertexCount + slice + 1;

			// Triangle 1
			indices.push_back(a);
			indices.push_back(c);
			indices.push_back(b);

			// Triangle 2
			indices.push_back(a);
			indices.push_back(d);
			indices.push_back(c);
		}
	}

	// Bottom cap indices
	UINT southPoleIndex = (UINT)positions.size() - 1;
	baseIndex = southPoleIndex - ringVertexCount;
	for (UINT i = 0; i < sliceCount; ++i)
	{
		indices.push_back(southPoleIndex);
		indices.push_back(baseIndex + i);
		indices.push_back(baseIndex + i + 1);
	}

	m_bHasVertex = true;
	m_nVertexCount = static_cast<UINT>(positions.size());

	// Create Vertex Buffer
	{
		D3D12_RESOURCE_DESC desc = BASIC_BUFFER_DESC;
		desc.Width = sizeof(XMFLOAT3) * m_nVertexCount;
		g_DxResource.device->CreateCommittedResource(
			&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(m_pd3dVertexBuffer.GetAddressOf()));

		void* tempData{};
		m_pd3dVertexBuffer->Map(0, nullptr, &tempData);
		memcpy(tempData, positions.data(), sizeof(XMFLOAT3) * m_nVertexCount);
		m_pd3dVertexBuffer->Unmap(0, nullptr);
	}

	m_bHasNormals = true;
	m_nNormalsCount = static_cast<UINT>(normals.size());
	// Create Normal Buffer
	{
		D3D12_RESOURCE_DESC desc = BASIC_BUFFER_DESC;
		desc.Width = sizeof(XMFLOAT3) * m_nVertexCount;
		g_DxResource.device->CreateCommittedResource(
			&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(m_pd3dNormalsBuffer.GetAddressOf()));

		void* tempData{};
		m_pd3dNormalsBuffer->Map(0, nullptr, &tempData);
		memcpy(tempData, normals.data(), sizeof(XMFLOAT3) * m_nVertexCount);
		m_pd3dNormalsBuffer->Unmap(0, nullptr);
	}

	// Create Index Buffer
	m_bHasSubMeshes = true;
	ComPtr<ID3D12Resource> tempBuffer{};
	D3D12_RESOURCE_DESC desc = BASIC_BUFFER_DESC;
	desc.Width = sizeof(UINT) * (UINT)indices.size();
	g_DxResource.device->CreateCommittedResource(
		&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(tempBuffer.GetAddressOf()));

	void* tempData{};
	tempBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, indices.data(), sizeof(UINT) * indices.size());
	tempBuffer->Unmap(0, nullptr);

	++m_nSubMeshesCount;
	m_vIndices.emplace_back((UINT)indices.size());
	m_vSubMeshes.emplace_back(tempBuffer);
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
	m_vIndices.emplace_back(indices);

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
	m_vSubMeshes.emplace_back(indexBuffer);
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