#include "GameObject.h"

bool CGameObject::InitializeObjectFromFile(std::ifstream& inFile)
{
	int temp;	// 불필요한 정보들을 빼낸다.
	inFile.read((char*)&temp, sizeof(int));	// 프레임 번호
	inFile.read((char*)&temp, sizeof(int));	// 텍스쳐 수

	// 이름 읽고 저장
	char strLength{};
	inFile.read(&strLength, sizeof(char));
	m_strName.assign(strLength, ' ');
	inFile.read(m_strName.data(), strLength);

	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};

	// <Transform>:
	readLabel();
	float dummyFloat[4];
	inFile.read((char*)&m_xmf3Pos, sizeof(XMFLOAT3));
	inFile.read((char*)dummyFloat, sizeof(float) * 3);
	inFile.read((char*)&m_xmf3Scale, sizeof(XMFLOAT3));
	inFile.read((char*)dummyFloat, sizeof(float) * 4);

	// <TransformMatrix>:
	readLabel();
	inFile.read((char*)&m_xmf4x4LocalMatrix, sizeof(XMFLOAT4X4));

	InitializeAxis();

	return true;
}

void CGameObject::InitializeConstanctBuffer(std::vector<std::unique_ptr<Mesh>>& meshes)
{
	auto makeBuffer = [&](UINT argSize) {
		ComPtr<ID3D12Resource> resource{};
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = Align(argSize, 256);
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(resource.GetAddressOf()));
		m_vCBuffers.push_back(resource);
		};

	for (int i = 0; i < m_vMaterials.size(); ++i) {
		makeBuffer(sizeof(HasMaterial));
		HasMaterial* pHas;
		m_vCBuffers[i]->Map(0, nullptr, (void**)&pHas);
		pHas->bHasAlbedoColor = m_vMaterials[i].m_bHasAlbedoColor;
		pHas->bHasEmissiveColor = m_vMaterials[i].m_bHasEmissiveColor;
		pHas->bHasSpecularColor = m_vMaterials[i].m_bHasSpecularColor;
		pHas->bHasGlossiness = m_vMaterials[i].m_bHasGlossiness;
		pHas->bHasSmoothness = m_vMaterials[i].m_bHasSmoothness;
		pHas->bHasMetallic = m_vMaterials[i].m_bHasMetallic;
		pHas->bHasSpecularHighlight = m_vMaterials[i].m_bHasSpecularHighlight;
		pHas->bHasGlossyReflection = m_vMaterials[i].m_bHasGlossyReflection;

		pHas->bHasAlbedoMap = m_vMaterials[i].m_bHasAlbedoMap;
		pHas->bHasSpecularMap = m_vMaterials[i].m_bHasSpecularMap;
		pHas->bHasNormalMap = m_vMaterials[i].m_bHasNormalMap;
		pHas->bHasMetallicMap = m_vMaterials[i].m_bHasMetallicMap;
		pHas->bHasEmissionMap = m_vMaterials[i].m_bHasEmissionMap;
		pHas->bHasDetailAlbedoMap = m_vMaterials[i].m_bHasDetailAlbedoMap;
		pHas->bHasDetailNormalMap = m_vMaterials[i].m_bHasDetailNormalMap;

		pHas->AlbedoColor = m_vMaterials[i].m_xmf4AlbedoColor;
		pHas->EmissiveColor = m_vMaterials[i].m_xmf4EmissiveColor;
		pHas->SpecularColor = m_vMaterials[i].m_xmf4SpecularColor;
		pHas->Glossiness = m_vMaterials[i].m_fGlossiness;
		pHas->Smoothness = m_vMaterials[i].m_fSmoothness;
		pHas->Metallic = m_vMaterials[i].m_fMetallic;
		pHas->SpecularHighlight = m_vMaterials[i].m_fSpecularHighlight;
		pHas->GlossyReflection = m_vMaterials[i].m_fGlossyReflection;
		m_vCBuffers[i]->Unmap(0, nullptr);
	}

	auto makeMeshCBuffer = [&](ComPtr<ID3D12Resource>& resource) {
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = Align(sizeof(HasMesh), 256);
		g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(resource.GetAddressOf()));
		};

	makeMeshCBuffer(m_pd3dMeshCBuffer);
	HasMesh* pHas;
	HasMesh tempHas{};
	m_pd3dMeshCBuffer->Map(0, nullptr, (void**)&pHas);
	if (m_nMeshIndex != -1) {
		if (meshes[m_nMeshIndex]->getHasVertex())
			tempHas.bHasVertex = true;
		if (meshes[m_nMeshIndex]->getHasColor())
			tempHas.bHasColor = true;
		if (meshes[m_nMeshIndex]->getHasTex0())
			tempHas.bHasTex0 = true;
		if (meshes[m_nMeshIndex]->getHasTex1())
			tempHas.bHasTex1 = true;
		if (meshes[m_nMeshIndex]->getHasNormal())
			tempHas.bHasNormals = true;
		if (meshes[m_nMeshIndex]->getHasTangent())
			tempHas.bHasTangenrs = true;
		if (meshes[m_nMeshIndex]->getHasBiTangent())
			tempHas.bHasBiTangents = true;
		if (meshes[m_nMeshIndex]->getHasSubmesh())
			tempHas.bHasSubMeshes = true;
	}
	memcpy(pHas, &tempHas, sizeof(HasMesh));
	m_pd3dMeshCBuffer->Unmap(0, nullptr);
}

void CGameObject::UpdateWorldMatrix()
{
	m_xmf4x4WorldMatrix._11 = m_xmf3Right.x; m_xmf4x4WorldMatrix._12 = m_xmf3Right.y; m_xmf4x4WorldMatrix._13 = m_xmf3Right.z; m_xmf4x4WorldMatrix._14 = 0;
	m_xmf4x4WorldMatrix._21 = m_xmf3Up.x; m_xmf4x4WorldMatrix._22 = m_xmf3Up.y; m_xmf4x4WorldMatrix._23 = m_xmf3Up.z; m_xmf4x4WorldMatrix._24 = 0;
	m_xmf4x4WorldMatrix._31 = m_xmf3Look.x; m_xmf4x4WorldMatrix._32 = m_xmf3Look.y; m_xmf4x4WorldMatrix._33 = m_xmf3Look.z; m_xmf4x4WorldMatrix._34 = 0;
	m_xmf4x4WorldMatrix._41 = m_xmf3Pos.x; m_xmf4x4WorldMatrix._42 = m_xmf3Pos.y; m_xmf4x4WorldMatrix._43 = m_xmf3Pos.z; m_xmf4x4WorldMatrix._44 = 1;
}

std::vector<Material>& CGameObject::getMaterials()
{
	return m_vMaterials;
}

int CGameObject::getMeshIndex() const
{
	return m_nMeshIndex;
}

int CGameObject::getHitGroupIndex() const
{
	return m_nHitGroupIndex;
}

int CGameObject::getParentIndex() const
{
	return m_nParentIndex;
}

ID3D12Resource* CGameObject::getCbuffer(int index) const
{
	return m_vCBuffers[index].Get();
}

ID3D12Resource* CGameObject::getMeshCBuffer() const
{
	return m_pd3dMeshCBuffer.Get();
}

XMFLOAT4X4 CGameObject::getWorldMatrix()
{
	return m_xmf4x4WorldMatrix;
}

XMFLOAT4X4 CGameObject::getLocalMatrix()
{
	return m_xmf4x4LocalMatrix;
}

void CGameObject::SetMeshIndex(int index)
{
	m_nMeshIndex = index;
}

void CGameObject::SetParentIndex(int index)
{
	m_nParentIndex = index;
}

void CGameObject::SetHitGroupIndex(int index)
{
	m_nHitGroupIndex = index;
}

void CGameObject::SetWorlaMatrix(XMFLOAT4X4& mtx)
{
	m_xmf4x4WorldMatrix = mtx;
}

void CGameObject::InitializeAxis()
{
	/*auto normalizeFloat3 = [](XMFLOAT3& xmf) {
		XMStoreFloat3(&xmf, XMVector3Normalize(XMLoadFloat3(&xmf)));
		};
	m_xmf3Right = XMFLOAT3(m_xmf4x4LocalMatrix._11, m_xmf4x4LocalMatrix._12, m_xmf4x4LocalMatrix._13);
	m_xmf3Up = XMFLOAT3(m_xmf4x4LocalMatrix._21, m_xmf4x4LocalMatrix._22, m_xmf4x4LocalMatrix._23);
	m_xmf3Look = XMFLOAT3(m_xmf4x4LocalMatrix._31, m_xmf4x4LocalMatrix._32, m_xmf4x4LocalMatrix._33);

	normalizeFloat3(m_xmf3Right);
	normalizeFloat3(m_xmf3Up);
	normalizeFloat3(m_xmf3Look);*/

	m_xmf3Right = XMFLOAT3(1.0, 0.0, 0.0);
	m_xmf3Up = XMFLOAT3(0.0, 1.0, 0.0);
	m_xmf3Look = XMFLOAT3(0.0, 0.0, 1.0);

}
