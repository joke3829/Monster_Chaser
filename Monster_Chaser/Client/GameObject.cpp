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

/*template<class T>
void CGameObject::InitializeConstanctBuffer(std::vector<T>& meshes)
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
*/

void CGameObject::UpdateWorldMatrix()
{
	m_xmf4x4WorldMatrix._11 = m_xmf3Right.x; m_xmf4x4WorldMatrix._12 = m_xmf3Right.y; m_xmf4x4WorldMatrix._13 = m_xmf3Right.z; m_xmf4x4WorldMatrix._14 = 0;
	m_xmf4x4WorldMatrix._21 = m_xmf3Up.x; m_xmf4x4WorldMatrix._22 = m_xmf3Up.y; m_xmf4x4WorldMatrix._23 = m_xmf3Up.z; m_xmf4x4WorldMatrix._24 = 0;
	m_xmf4x4WorldMatrix._31 = m_xmf3Look.x; m_xmf4x4WorldMatrix._32 = m_xmf3Look.y; m_xmf4x4WorldMatrix._33 = m_xmf3Look.z; m_xmf4x4WorldMatrix._34 = 0;
	m_xmf4x4WorldMatrix._41 = m_xmf3Pos.x; m_xmf4x4WorldMatrix._42 = m_xmf3Pos.y; m_xmf4x4WorldMatrix._43 = m_xmf3Pos.z; m_xmf4x4WorldMatrix._44 = 1;
}

std::string CGameObject::getFrameName() const
{
	return m_strName;
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


// =============================================================

CSkinningInfo::CSkinningInfo(std::ifstream& inFile, UINT nRefMesh)
{
	std::string strLabel{};
	UINT tempInt{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};
	m_nRefMesh = nRefMesh;

	readLabel();	// 이름
	while (1) {
		readLabel();
		if (strLabel == "</SkinningInfo>")
			break;
		else if ("<BonesPerVertex>:" == strLabel)
			inFile.read((char*)&m_nBonesPerVertex, sizeof(int));
		else if ("<Bounds>:" == strLabel) { // 바운딩 박스 정보, 필요하면 가져간다
			XMFLOAT3 tempData{};
			inFile.read((char*)&tempData, sizeof(XMFLOAT3));
			inFile.read((char*)&tempData, sizeof(XMFLOAT3));
		}
		else if ("<BoneNames>:" == strLabel) {
			inFile.read((char*)&m_nBones, sizeof(int));
			for (int i = 0; i < m_nBones; ++i) {
				readLabel();
				m_vBoneNames.push_back(strLabel);
			}
		}
		else if ("<BoneOffsets>:" == strLabel) {
			inFile.read((char*)&m_nBones, sizeof(int));
			m_vOffsetMatrix.assign(m_nBones, XMFLOAT4X4());
			inFile.read((char*)m_vOffsetMatrix.data(), sizeof(XMFLOAT4X4) * m_nBones);
		}
		else if ("<BoneIndices>:" == strLabel) {
			inFile.read((char*)&tempInt, sizeof(UINT));
			m_vBoneIndices.assign(tempInt * m_nBonesPerVertex, 0);
			inFile.read((char*)m_vBoneIndices.data(), sizeof(int) * tempInt * m_nBonesPerVertex);
		}
		else if ("<BoneWeights>:" == strLabel) {
			inFile.read((char*)&tempInt, sizeof(UINT));
			m_nVertexCount = tempInt;
			m_vBoneWeight.assign(tempInt * m_nBonesPerVertex, 0);
			inFile.read((char*)m_vBoneWeight.data(), sizeof(float) * tempInt * m_nBonesPerVertex);
		}
	}
}

void CSkinningInfo::MakeAnimationMatrixIndex(std::vector<std::string>& vFrameNames)
{
	for (std::string& name : m_vBoneNames) {
		auto p = std::find(vFrameNames.begin(), vFrameNames.end(), name);
		m_vAnimationMatrixIndex.push_back(std::distance(vFrameNames.begin(), p));
	}
}

void CSkinningInfo::MakeBufferAndDescriptorHeap(ComPtr<ID3D12Resource>& pMatrixBuffer, UINT nElements)
{
	D3D12_DESCRIPTOR_HEAP_DESC ddesc{};
	ddesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ddesc.NumDescriptors = 6;
	ddesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	g_DxResource.device->CreateDescriptorHeap(&ddesc, IID_PPV_ARGS(m_pd3dDesciptorHeap.GetAddressOf()));

	void* tempData{};

	// 상수 버퍼
	auto desc = BASIC_BUFFER_DESC;
	
	desc.Width = Align(sizeof(UINT) * 2, 256);
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pConstantBuffer.GetAddressOf()));
	m_pConstantBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, &m_nVertexCount, sizeof(UINT));
	tempData = static_cast<char*>(tempData) + sizeof(UINT);
	memcpy(tempData, &m_nBonesPerVertex, sizeof(UINT));
	m_pConstantBuffer->Unmap(0, nullptr);

	// 오프셋 행렬
	desc.Width = sizeof(XMFLOAT4X4) * m_vOffsetMatrix.size();
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pOffsetMatrixBuffer.GetAddressOf()));
	m_pOffsetMatrixBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, m_vOffsetMatrix.data(), sizeof(XMFLOAT4X4) * m_vOffsetMatrix.size());
	m_pOffsetMatrixBuffer->Unmap(0, nullptr);

	// 뼈 인덱스
	desc.Width = sizeof(UINT) * m_vBoneIndices.size();
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pBoneIndicesBuffer.GetAddressOf()));
	m_pBoneIndicesBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, m_vBoneIndices.data(), sizeof(UINT) * m_vBoneIndices.size());
	m_pBoneIndicesBuffer->Unmap(0, nullptr);

	// 뼈 가중치
	desc.Width = sizeof(float) * m_vBoneWeight.size();
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pBoneWeightBuffer.GetAddressOf()));
	m_pBoneWeightBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, m_vBoneWeight.data(), sizeof(float) * m_vBoneWeight.size());
	m_pBoneWeightBuffer->Unmap(0, nullptr);

	// 애니메이션 행렬 인덱스
	desc.Width = sizeof(UINT) * m_vAnimationMatrixIndex.size();
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pAnimationMatrixIndexBuffer.GetAddressOf()));
	m_pAnimationMatrixIndexBuffer->Map(0, nullptr, &tempData);
	memcpy(tempData, m_vBoneWeight.data(), sizeof(UINT) * m_vAnimationMatrixIndex.size());
	m_pAnimationMatrixIndexBuffer->Unmap(0, nullptr);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_pd3dDesciptorHeap->GetCPUDescriptorHandleForHeapStart();

	UINT incrementSize = g_DxResource.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_CONSTANT_BUFFER_VIEW_DESC cDesc{};
	// 상수버퍼
	cDesc.BufferLocation = m_pConstantBuffer->GetGPUVirtualAddress();
	cDesc.SizeInBytes = Align(sizeof(UINT) * 2, 256);
	g_DxResource.device->CreateConstantBufferView(&cDesc, handle);
	handle.ptr += incrementSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC vDesc{};
	vDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	vDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	vDesc.Buffer.FirstElement = 0;
	vDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	// 오프셋 행렬
	vDesc.Buffer.NumElements = m_vOffsetMatrix.size();
	vDesc.Buffer.StructureByteStride = sizeof(XMFLOAT4X4);
	g_DxResource.device->CreateShaderResourceView(m_pOffsetMatrixBuffer.Get(), &vDesc, handle);
	handle.ptr += incrementSize;

	// 뼈 인덱스
	vDesc.Buffer.NumElements = m_vBoneIndices.size();
	vDesc.Buffer.StructureByteStride = sizeof(UINT);
	g_DxResource.device->CreateShaderResourceView(m_pBoneIndicesBuffer.Get(), &vDesc, handle);
	handle.ptr += incrementSize;

	// 뼈 가중치
	vDesc.Buffer.NumElements = m_vBoneWeight.size();
	vDesc.Buffer.StructureByteStride = sizeof(float);
	g_DxResource.device->CreateShaderResourceView(m_pBoneWeightBuffer.Get(), &vDesc, handle);
	handle.ptr += incrementSize;

	// 애니메이션 행렬 인덱스
	vDesc.Buffer.NumElements = m_vAnimationMatrixIndex.size();
	vDesc.Buffer.StructureByteStride = sizeof(UINT);
	g_DxResource.device->CreateShaderResourceView(m_pAnimationMatrixIndexBuffer.Get(), &vDesc, handle);
	handle.ptr += incrementSize;

	// 애니메이션 행렬
	vDesc.Buffer.NumElements = nElements;
	vDesc.Buffer.StructureByteStride = sizeof(XMFLOAT4X4);
	g_DxResource.device->CreateShaderResourceView(pMatrixBuffer.Get(), &vDesc, handle);
}

void CSkinningInfo::SetShaderVariables()
{
	g_DxResource.cmdList->SetDescriptorHeaps(1, m_pd3dDesciptorHeap.GetAddressOf());
	g_DxResource.cmdList->SetComputeRootDescriptorTable(0, m_pd3dDesciptorHeap->GetGPUDescriptorHandleForHeapStart());
}

// ============================================================================

void CSkinningObject::AddResourceFromFile(std::ifstream& inFile, std::string strFront)
{
	FilePathFront = strFront;
	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};
	while (1) {
		readLabel();
		if (strLabel == "</Hierarchy>")
			break;
		else if (strLabel == "<Frame>:")
			AddObjectFromFile(inFile);
	}

	if (m_vObjects.size() != 0)
		m_strObjectName = m_vObjects[0]->getFrameName();
}

void CSkinningObject::AddObjectFromFile(std::ifstream& inFile, int nParentIndex)
{
	UINT nCurrentObjectIndex = m_vObjects.size();
	m_vObjects.push_back(std::make_unique<CGameObject>());
	m_vObjects[nCurrentObjectIndex]->InitializeObjectFromFile(inFile);

	if (nParentIndex != -1) {		// 부모가 존재한다는 뜻
		m_vObjects[nCurrentObjectIndex]->SetParentIndex(nParentIndex);
	}

	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};
	int tempData{};
	while (1) {
		readLabel();
		if (strLabel == "<Mesh>:") {
			inFile.read((char*)&tempData, sizeof(int));
			readLabel();	// 메시의 이름 읽기
			auto p = std::find_if(m_vMeshes.begin(), m_vMeshes.end(), [&](std::shared_ptr<Mesh>& tempMesh) {
				return tempMesh->getName() == strLabel;
				});
			if (p != m_vMeshes.end()) {	// 이미 리스트에 해당 이름을 가진 메시가 존재
				// 주의할게 이름이 아예 중복이 없는지는 아직 확인을 못함
				m_vObjects[nCurrentObjectIndex]->SetMeshIndex(std::distance(m_vMeshes.begin(), p));
				// 중복 메시가 나와도 파일에는 전부 기록되어 있다.
				Mesh* tempMesh = new Mesh(inFile, strLabel);	// 그 기록을 빼주는 작업
				delete tempMesh;
			}
			else {	// 없으면 새로 생성과 동시에 인덱스 지정
				m_vObjects[nCurrentObjectIndex]->SetMeshIndex(m_vMeshes.size());
				m_vMeshes.push_back(std::make_shared<Mesh>(inFile, strLabel));
			}
		}
		else if (strLabel == "<SkinningInfo>:") {
			m_vSkinningInfo.push_back(std::make_unique<CSkinningInfo>(inFile, m_vMeshes.size()));
		}
		else if (strLabel == "<Materials>:") {
			inFile.read((char*)&tempData, sizeof(int));
			AddMaterialFromFile(inFile, nCurrentObjectIndex);
		}
		else if (strLabel == "<Children>:") {
			inFile.read((char*)&tempData, sizeof(int));
			if (tempData > 0) {
				for (int i = 0; i < tempData; ++i) {
					readLabel();	// <Frame>: 부분을 빼준다
					AddObjectFromFile(inFile, nCurrentObjectIndex);
				}
			}
		}
		else if (strLabel == "</Frame>")
			break;
	}
}

void CSkinningObject::AddMaterialFromFile(std::ifstream& inFile, int nCurrentIndex)
{
	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};
	int tempData{};
	std::vector<Material> vMaterials;

	std::string FilePathBack{ ".dds" };				// 포맷도 바뀔 수 있다.

	int nCurrentMaterial{};
	while (1) {
		readLabel();
		if (strLabel == "<Material>:") {
			nCurrentMaterial = vMaterials.size();
			vMaterials.push_back(Material());
			inFile.read((char*)&tempData, sizeof(int));
		}
		else if (strLabel == "<AlbedoColor>:") {
			vMaterials[nCurrentMaterial].m_bHasAlbedoColor = true;
			inFile.read((char*)&vMaterials[nCurrentMaterial].m_xmf4AlbedoColor, sizeof(XMFLOAT4));
		}
		else if (strLabel == "<EmissiveColor>:") {
			vMaterials[nCurrentMaterial].m_bHasEmissiveColor = true;
			inFile.read((char*)&vMaterials[nCurrentMaterial].m_xmf4EmissiveColor, sizeof(XMFLOAT4));
		}
		else if (strLabel == "<SpecularColor>:") {
			vMaterials[nCurrentMaterial].m_bHasSpecularColor = true;
			inFile.read((char*)&vMaterials[nCurrentMaterial].m_xmf4SpecularColor, sizeof(XMFLOAT4));
		}
		else if (strLabel == "<Glossiness>:") {
			vMaterials[nCurrentMaterial].m_bHasGlossiness = true;
			inFile.read((char*)&vMaterials[nCurrentMaterial].m_fGlossiness, sizeof(float));
		}
		else if (strLabel == "<Smoothness>:") {
			vMaterials[nCurrentMaterial].m_bHasSmoothness = true;
			inFile.read((char*)&vMaterials[nCurrentMaterial].m_fSmoothness, sizeof(float));
		}
		else if (strLabel == "<Metallic>:") {
			vMaterials[nCurrentMaterial].m_bHasMetallic = true;
			inFile.read((char*)&vMaterials[nCurrentMaterial].m_fMetallic, sizeof(float));
		}
		else if (strLabel == "<SpecularHighlight>:") {
			vMaterials[nCurrentMaterial].m_bHasSpecularHighlight = true;
			inFile.read((char*)&vMaterials[nCurrentMaterial].m_fSpecularHighlight, sizeof(float));
		}
		else if (strLabel == "<GlossyReflection>:") {
			vMaterials[nCurrentMaterial].m_bHasGlossyReflection = true;
			inFile.read((char*)&vMaterials[nCurrentMaterial].m_fGlossyReflection, sizeof(float));
		}
		else if (strLabel == "<AlbedoMap>:") {
			readLabel();
			if (strLabel[0] == '@') {	// 이미 존재함
				strLabel.erase(strLabel.begin());
				auto p = std::find_if(m_vTextures.begin(), m_vTextures.end(), [&](std::shared_ptr<CTexture>& txt) {
					return txt->getName() == strLabel;
					});
				vMaterials[nCurrentMaterial].m_bHasAlbedoMap = true;
				vMaterials[nCurrentMaterial].m_nAlbedoMapIndex = std::distance(m_vTextures.begin(), p);
			}
			else if (strLabel == "null") {	// null이면 사용 안한단 뜻
				continue;
			}
			else {	// 없으니 새로 만들어라
				vMaterials[nCurrentMaterial].m_bHasAlbedoMap = true;
				vMaterials[nCurrentMaterial].m_nAlbedoMapIndex = m_vTextures.size();
				std::string FilePath = FilePathFront + strLabel + FilePathBack;
				std::wstring wstr;
				wstr.assign(FilePath.begin(), FilePath.end());
				m_vTextures.push_back(std::make_shared<CTexture>(wstr.c_str()));
				m_vTextures[m_vTextures.size() - 1]->SetTextureName(strLabel);
			}
		}
		else if (strLabel == "<SpecularMap>:") {
			readLabel();
			if (strLabel[0] == '@') {	// 이미 존재함
				strLabel.erase(strLabel.begin());
				auto p = std::find_if(m_vTextures.begin(), m_vTextures.end(), [&](std::shared_ptr<CTexture>& txt) {
					return txt->getName() == strLabel;
					});
				vMaterials[nCurrentMaterial].m_bHasSpecularMap = true;
				vMaterials[nCurrentMaterial].m_nSpecularMapIndex = std::distance(m_vTextures.begin(), p);	// 여기
			}
			else if (strLabel == "null") {	// null이면 사용 안한단 뜻
				continue;
			}
			else {	// 없으니 새로 만들어라
				vMaterials[nCurrentMaterial].m_bHasSpecularMap = true;	// 여기
				vMaterials[nCurrentMaterial].m_nSpecularMapIndex = m_vTextures.size();	// 여기
				std::string FilePath = FilePathFront + strLabel + FilePathBack;
				std::wstring wstr;
				wstr.assign(FilePath.begin(), FilePath.end());
				m_vTextures.push_back(std::make_shared<CTexture>(wstr.c_str()));
				m_vTextures[m_vTextures.size() - 1]->SetTextureName(strLabel);
			}
		}
		else if (strLabel == "<MetallicMap>:") {
			readLabel();
			if (strLabel[0] == '@') {	// 이미 존재함
				strLabel.erase(strLabel.begin());
				auto p = std::find_if(m_vTextures.begin(), m_vTextures.end(), [&](std::shared_ptr<CTexture>& txt) {
					return txt->getName() == strLabel;
					});
				vMaterials[nCurrentMaterial].m_bHasMetallicMap = true;
				vMaterials[nCurrentMaterial].m_nMetallicMapIndex = std::distance(m_vTextures.begin(), p);
			}
			else if (strLabel == "null") {	// null이면 사용 안한단 뜻
				continue;
			}
			else {	// 없으니 새로 만들어라
				vMaterials[nCurrentMaterial].m_bHasMetallicMap = true;
				vMaterials[nCurrentMaterial].m_nMetallicMapIndex = m_vTextures.size();
				std::string FilePath = FilePathFront + strLabel + FilePathBack;
				std::wstring wstr;
				wstr.assign(FilePath.begin(), FilePath.end());
				m_vTextures.push_back(std::make_shared<CTexture>(wstr.c_str()));
				m_vTextures[m_vTextures.size() - 1]->SetTextureName(strLabel);
			}
		}
		else if (strLabel == "<NormalMap>:") {
			readLabel();
			if (strLabel[0] == '@') {	// 이미 존재함
				strLabel.erase(strLabel.begin());
				auto p = std::find_if(m_vTextures.begin(), m_vTextures.end(), [&](std::shared_ptr<CTexture>& txt) {
					return txt->getName() == strLabel;
					});
				vMaterials[nCurrentMaterial].m_bHasNormalMap = true;
				vMaterials[nCurrentMaterial].m_nNormalMapIndex = std::distance(m_vTextures.begin(), p);
			}
			else if (strLabel == "null") {	// null이면 사용 안한단 뜻
				continue;
			}
			else {	// 없으니 새로 만들어라
				vMaterials[nCurrentMaterial].m_bHasNormalMap = true;
				vMaterials[nCurrentMaterial].m_nNormalMapIndex = m_vTextures.size();
				std::string FilePath = FilePathFront + strLabel + FilePathBack;
				std::wstring wstr;
				wstr.assign(FilePath.begin(), FilePath.end());
				m_vTextures.push_back(std::make_shared<CTexture>(wstr.c_str()));
				m_vTextures[m_vTextures.size() - 1]->SetTextureName(strLabel);
			}
		}
		else if (strLabel == "<EmissionMap>:") {
			readLabel();
			if (strLabel[0] == '@') {	// 이미 존재함
				strLabel.erase(strLabel.begin());
				auto p = std::find_if(m_vTextures.begin(), m_vTextures.end(), [&](std::shared_ptr<CTexture>& txt) {
					return txt->getName() == strLabel;
					});
				vMaterials[nCurrentMaterial].m_bHasEmissionMap = true;
				vMaterials[nCurrentMaterial].m_nEmissionMapIndex = std::distance(m_vTextures.begin(), p);
			}
			else if (strLabel == "null") {	// null이면 사용 안한단 뜻
				continue;
			}
			else {	// 없으니 새로 만들어라
				vMaterials[nCurrentMaterial].m_bHasEmissionMap = true;
				vMaterials[nCurrentMaterial].m_nEmissionMapIndex = m_vTextures.size();
				std::string FilePath = FilePathFront + strLabel + FilePathBack;
				std::wstring wstr;
				wstr.assign(FilePath.begin(), FilePath.end());
				m_vTextures.push_back(std::make_shared<CTexture>(wstr.c_str()));
				m_vTextures[m_vTextures.size() - 1]->SetTextureName(strLabel);
			}
		}
		else if (strLabel == "<DetailAlbedoMap>:") {
			readLabel();
			if (strLabel[0] == '@') {	// 이미 존재함
				strLabel.erase(strLabel.begin());
				auto p = std::find_if(m_vTextures.begin(), m_vTextures.end(), [&](std::shared_ptr<CTexture>& txt) {
					return txt->getName() == strLabel;
					});
				vMaterials[nCurrentMaterial].m_bHasDetailAlbedoMap = true;
				vMaterials[nCurrentMaterial].m_nDetailAlbedoMapIndex = std::distance(m_vTextures.begin(), p);
			}
			else if (strLabel == "null") {	// null이면 사용 안한단 뜻
				continue;
			}
			else {	// 없으니 새로 만들어라
				vMaterials[nCurrentMaterial].m_bHasDetailAlbedoMap = true;
				vMaterials[nCurrentMaterial].m_nDetailAlbedoMapIndex = m_vTextures.size();
				std::string FilePath = FilePathFront + strLabel + FilePathBack;
				std::wstring wstr;
				wstr.assign(FilePath.begin(), FilePath.end());
				m_vTextures.push_back(std::make_shared<CTexture>(wstr.c_str()));
				m_vTextures[m_vTextures.size() - 1]->SetTextureName(strLabel);
			}
		}
		else if (strLabel == "<DetailNormalMap>:") {
			readLabel();
			if (strLabel[0] == '@') {	// 이미 존재함
				strLabel.erase(strLabel.begin());
				auto p = std::find_if(m_vTextures.begin(), m_vTextures.end(), [&](std::shared_ptr<CTexture>& txt) {
					return txt->getName() == strLabel;
					});
				vMaterials[nCurrentMaterial].m_bHasDetailNormalMap = true;
				vMaterials[nCurrentMaterial].m_nDetailNormalMapIndex = std::distance(m_vTextures.begin(), p);
			}
			else if (strLabel == "null") {	// 있더라고 null이면 사용 안한단 뜻
				continue;
			}
			else {	// 없으니 새로 만들어라
				vMaterials[nCurrentMaterial].m_bHasDetailNormalMap = true;
				vMaterials[nCurrentMaterial].m_nDetailNormalMapIndex = m_vTextures.size();
				std::string FilePath = FilePathFront + strLabel + FilePathBack;
				std::wstring wstr;
				wstr.assign(FilePath.begin(), FilePath.end());
				m_vTextures.push_back(std::make_shared<CTexture>(wstr.c_str()));
				m_vTextures[m_vTextures.size() - 1]->SetTextureName(strLabel);
			}
		}
		else if (strLabel == "</Materials>")
			break;
	}
	// 게임 오브젝트에 마테리얼 저장
	for (int i = 0; i < vMaterials.size(); ++i) {
		m_vObjects[nCurrentIndex]->getMaterials().push_back(vMaterials[i]);
	}
}

void CSkinningObject::InitializeGameObjectCBuffer()
{
	for (std::unique_ptr<CGameObject>& object : m_vObjects)
		object->InitializeConstanctBuffer(m_vMeshes);
}

std::vector<std::unique_ptr<CSkinningInfo>>& CSkinningObject::getSkinningInfo()
{
	return m_vSkinningInfo;
}


// ==================================================================================

void CRayTracingSkinningObject::PrepareObject()
{
	MakeBLAS();
	ReadyOutputVertexBuffer();
}

void CRayTracingSkinningObject::UpdateObject(float fElapsedTime)
{
	for (int i = 0; i < m_vSkinningInfo.size(); ++i) {
		m_vSkinningInfo[i]->SetShaderVariables();
		UINT ref = m_vSkinningInfo[i]->getRefMeshIndex();
		g_DxResource.cmdList->SetComputeRootShaderResourceView(1, m_vMeshes[ref]->getVertexBuffer()->GetGPUVirtualAddress());
		g_DxResource.cmdList->SetDescriptorHeaps(1, m_vUAV[ref].GetAddressOf());
		g_DxResource.cmdList->SetComputeRootDescriptorTable(2, m_vUAV[ref]->GetGPUDescriptorHandleForHeapStart());

		UINT dispatch = m_vMeshes[ref]->getVertexCount() / 32 + 1;
		g_DxResource.cmdList->Dispatch(dispatch, 1, 1);
	}
}

void CRayTracingSkinningObject::ReBuildBLAS()
{
	for (std::unique_ptr<CSkinningInfo>& info : m_vSkinningInfo) {
		UINT ref = info->getRefMeshIndex();
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> GeometryDesc{};
		if (m_vMeshes[ref]->getHasSubmesh()) {
			int nSubMesh = m_vMeshes[ref]->getSubMeshCount();
			for (int i = 0; i < nSubMesh; ++i) {
				D3D12_RAYTRACING_GEOMETRY_DESC desc{};
				desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
				desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;		// 임시
				desc.Triangles.Transform3x4 = 0;
				desc.Triangles.VertexBuffer = {
					.StartAddress = m_vOutputVertexBuffer[ref]->GetGPUVirtualAddress(),
					.StrideInBytes = sizeof(float) * 3
				};
				desc.Triangles.VertexCount = m_vMeshes[ref]->getVertexCount();
				desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				desc.Triangles.IndexBuffer = m_vMeshes[ref]->getIndexBuffer(i)->GetGPUVirtualAddress();
				desc.Triangles.IndexCount = m_vMeshes[ref]->getIndexCount(i);
				desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;

				GeometryDesc.push_back(desc);
			}
		}
		else {
			D3D12_RAYTRACING_GEOMETRY_DESC desc{};
			desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;		// 임시
			desc.Triangles.Transform3x4 = 0;
			desc.Triangles.VertexBuffer = {
				.StartAddress = m_vOutputVertexBuffer[ref]->GetGPUVirtualAddress(),
				.StrideInBytes = sizeof(float) * 3
			};
			desc.Triangles.VertexCount = m_vMeshes[ref]->getVertexCount();
			desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			desc.Triangles.IndexBuffer = 0;
			desc.Triangles.IndexCount = 0;
			desc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;

			GeometryDesc.push_back(desc);
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		inputs.NumDescs = GeometryDesc.size();
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.pGeometryDescs = GeometryDesc.data();

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC ASDesc{};
		ASDesc.Inputs = inputs;
		ASDesc.DestAccelerationStructureData = m_vBLAS[ref]->GetGPUVirtualAddress();
		ASDesc.ScratchAccelerationStructureData = m_pScratchBuffer->GetGPUVirtualAddress();

		g_DxResource.cmdList->BuildRaytracingAccelerationStructure(&ASDesc, 0, nullptr);
		D3D12_RESOURCE_BARRIER d3dbr{};
		d3dbr.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		d3dbr.UAV.pResource = m_vBLAS[ref].Get();
		g_DxResource.cmdList->ResourceBarrier(1, &d3dbr);
	}
}

void CRayTracingSkinningObject::MakeBLAS()
{
	for (std::shared_ptr<Mesh>& mesh : m_vMeshes) {
		ComPtr<ID3D12Resource> blas{};
		if (mesh->getHasVertex()) {
			InitBLAS(blas, mesh);
		}
		m_vBLAS.push_back(blas);
	}
	if (m_nScratchSize != 0) {
		auto desc = BASIC_BUFFER_DESC;
		desc.Width = m_nScratchSize;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		g_DxResource.device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(m_pScratchBuffer.GetAddressOf()));
	}
}

void CRayTracingSkinningObject::InitBLAS(ComPtr<ID3D12Resource>& resource, std::shared_ptr<Mesh>& mesh)
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

			GeometryDesc.push_back(desc);
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

		GeometryDesc.push_back(desc);
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = GeometryDesc.size();
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.pGeometryDescs = GeometryDesc.data();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo{};
	g_DxResource.device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &preBuildInfo);
	m_nScratchSize = (preBuildInfo.ScratchDataSizeInBytes > m_nScratchSize) ? preBuildInfo.ScratchDataSizeInBytes : m_nScratchSize;

	// blas
	auto bDesc = BASIC_BUFFER_DESC;
	bDesc.Width = preBuildInfo.ResultDataMaxSizeInBytes;
	bDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	g_DxResource.device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &bDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(resource.GetAddressOf()));

	ComPtr<ID3D12Resource> ScratchBuffer{};
	bDesc.Width = preBuildInfo.ScratchDataSizeInBytes;
	g_DxResource.device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &bDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(ScratchBuffer.GetAddressOf()));

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC ASDesc{};
	ASDesc.Inputs = inputs;
	ASDesc.DestAccelerationStructureData = resource->GetGPUVirtualAddress();
	ASDesc.ScratchAccelerationStructureData = ScratchBuffer->GetGPUVirtualAddress();

	g_DxResource.cmdAlloc->Reset();
	g_DxResource.cmdList->Reset(g_DxResource.cmdAlloc, nullptr);
	g_DxResource.cmdList->BuildRaytracingAccelerationStructure(&ASDesc, 0, nullptr);
	D3D12_RESOURCE_BARRIER d3dbr{};
	d3dbr.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	d3dbr.UAV.pResource = resource.Get();
	g_DxResource.cmdList->ResourceBarrier(1, &d3dbr);
	g_DxResource.cmdList->Close();
	g_DxResource.cmdQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&g_DxResource.cmdList));
	Flush();
}

void CRayTracingSkinningObject::ReadyOutputVertexBuffer()
{
	for (std::unique_ptr<CSkinningInfo>& info : m_vSkinningInfo)
		m_vMeshes[info->getRefMeshIndex()]->setSkinning(true);
	for (std::shared_ptr<Mesh>& mesh : m_vMeshes) {
		ComPtr<ID3D12Resource> resource{};
		ComPtr<ID3D12DescriptorHeap> descriptorHeap{};
		if (mesh->getbSkinning()) {
			auto desc = BASIC_BUFFER_DESC;
			desc.Width = mesh->getVertexCount() * sizeof(XMFLOAT3);
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			g_DxResource.device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(resource.GetAddressOf()));

			D3D12_DESCRIPTOR_HEAP_DESC hDesc{};
			hDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			hDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			hDesc.NumDescriptors = 1;

			g_DxResource.device->CreateDescriptorHeap(&hDesc, IID_PPV_ARGS(descriptorHeap.GetAddressOf()));

			D3D12_UNORDERED_ACCESS_VIEW_DESC vDesc{};
			vDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			vDesc.Buffer.NumElements = mesh->getVertexCount();
			vDesc.Buffer.StructureByteStride = sizeof(XMFLOAT3);

			g_DxResource.device->CreateUnorderedAccessView(resource.Get(), nullptr, &vDesc, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
		}
		m_vOutputVertexBuffer.push_back(resource);
		m_vUAV.push_back(descriptorHeap);
	}
}