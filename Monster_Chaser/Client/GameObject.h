#pragma once
#include "Mesh.h"
#include "Texture.h"

extern DXResources g_DxResource;

struct Material {	// ��������� ����� �̷��� �� ��
	bool m_bHasAlbedoColor = false;
	bool m_bHasEmissiveColor = false;
	bool m_bHasSpecularColor = false;
	bool m_bHasGlossiness = false;
	bool m_bHasSmoothness = false;
	bool m_bHasMetallic = false;
	bool m_bHasSpecularHighlight = false;
	bool m_bHasGlossyReflection = false;

	bool m_bHasAlbedoMap = false;
	bool m_bHasSpecularMap = false;
	bool m_bHasNormalMap = false;
	bool m_bHasMetallicMap = false;
	bool m_bHasEmissionMap = false;
	bool m_bHasDetailAlbedoMap = false;
	bool m_bHasDetailNormalMap = false;

	XMFLOAT4 m_xmf4AlbedoColor{};
	XMFLOAT4 m_xmf4EmissiveColor{};
	XMFLOAT4 m_xmf4SpecularColor{};
	float m_fGlossiness{};
	float m_fSmoothness{};
	float m_fMetallic{};
	float m_fSpecularHighlight{};
	float m_fGlossyReflection{};

	int m_nAlbedoMapIndex = -1;
	int m_nSpecularMapIndex = -1;
	int m_nNormalMapIndex = -1;
	int m_nMetallicMapIndex = -1;
	int m_nEmissionMapIndex = -1;
	int m_nDetailAlbedoMapIndex = -1;
	int m_nDetailNormalMapIndex = -1;
};

struct HasMaterial
{
	int bHasAlbedoColor;
	int bHasEmissiveColor;
	int bHasSpecularColor;
	int bHasGlossiness;
	int bHasSmoothness;
	int bHasMetallic;
	int bHasSpecularHighlight;
	int bHasGlossyReflection;

	int bHasAlbedoMap;
	int bHasSpecularMap;
	int bHasNormalMap;
	int bHasMetallicMap;
	int bHasEmissionMap;
	int bHasDetailAlbedoMap;
	int bHasDetailNormalMap;
	int padding[1];

	XMFLOAT4 AlbedoColor;
	XMFLOAT4 EmissiveColor;
	XMFLOAT4 SpecularColor;
	float Glossiness;
	float Smoothness;
	float Metallic;
	float SpecularHighlight;
	float GlossyReflection;
	float padding2[3];
};

struct HasMesh {
	int bHasVertex = false;
	int bHasColor = false;
	int bHasTex0 = false;
	int bHasTex1 = false;
	int bHasNormals = false;
	int bHasTangenrs = false;
	int bHasBiTangents = false;
	int bHasSubMeshes = false;
};

class CGameObject {
public:
	CGameObject() {};
	CGameObject(const CGameObject& other);
	CGameObject& operator=(const CGameObject& other);	// ���� �Ҵ�
	bool InitializeObjectFromFile(std::ifstream& inFile);

	template<class T>
	void InitializeConstanctBuffer(std::vector<T>& meshes) 
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

	void SetPosition(XMFLOAT3 pos);
	void Rotate(XMFLOAT3 rot);	// ���� right, up, look ������ ȸ��
	void SetScale(XMFLOAT3 scale);

	void move(float fElapsedTime);	// test��

	std::string getFrameName() const;
	std::vector<Material>& getMaterials();
	int getMeshIndex() const;
	int getHitGroupIndex() const;
	int getParentIndex() const;
	ID3D12Resource* getCbuffer(int index) const;
	ID3D12Resource* getMeshCBuffer() const;


	XMFLOAT4X4 getWorldMatrix();
	XMFLOAT4X4 getLocalMatrix();
	XMFLOAT4X4 getAnimationMatrix() { return m_xmf4x4AnimationMatrix; }

	void SetMeshIndex(int index);
	void SetParentIndex(int index);
	void SetHitGroupIndex(int index);
	void SetFrameName(std::string& name) { m_strName = name; }
	void SetBoundingOBB(XMFLOAT3& center, XMFLOAT3& extent) { m_bUseBoundingInfo |= 0x0011; m_OBB = BoundingOrientedBox(center, extent, XMFLOAT4(0.0, 0.0, 0.0, 1.0)); }
	void SetBoundingSphere(XMFLOAT3& center, float rad) { m_bUseBoundingInfo |= 0x1100, m_BoundingSphere = BoundingSphere(center, rad); }
	void SetWorlaMatrix(XMFLOAT4X4& mtx);
	void SetLocalMatrix(XMFLOAT4X4& ltx) { m_xmf4x4LocalMatrix = ltx; }
	void SetLocalMatrixTranspose(XMFLOAT4X4& ltx) { XMStoreFloat4x4(&m_xmf4x4LocalMatrix, XMMatrixTranspose(XMLoadFloat4x4(&ltx))); }

	void SetAnimationMatrix(XMFLOAT4X4& atx) { m_xmf4x4AnimationMatrix = atx; }

	void InitializeAxis();
protected:
	void UpdateLocalMatrix();
	std::string m_strName{};

	unsigned short m_bUseBoundingInfo{};	// �ٿ�� ���� ���� ��->Sphere, ��->OBB		
	BoundingOrientedBox m_OBB{};
	BoundingSphere m_BoundingSphere{};

	XMFLOAT3 m_xmf3Pos{};
	XMFLOAT3 m_xmf3Scale{1.0f, 1.0f, 1.0f};

	XMFLOAT4X4 m_xmf4x4LocalMatrix{};
	XMFLOAT4X4 m_xmf4x4WorldMatrix{};
	XMFLOAT4X4 m_xmf4x4AnimationMatrix{};

	XMFLOAT3 m_xmf3Right{1.0f, 0.0f, 0.0f};
	XMFLOAT3 m_xmf3Up{0.0f, 1.0f, 0.0f};
	XMFLOAT3 m_xmf3Look{0.0f, 0.0f, 1.0f};

	std::vector<Material> m_vMaterials;
	std::vector<ComPtr<ID3D12Resource>> m_vCBuffers;		// ��� ���� ����
	ComPtr<ID3D12Resource> m_pd3dMeshCBuffer{};
	// ������۷� �ѱ� ���ҽ��� �ʿ��Ѱ�? �ϴ� ����
	// ��� ���۷� �ѱ�Ŵ� ���׸����� AlbedoColor, EmissiveColor, Glossiness, Metalic, SpecularHighlight, 
	// �ؽ����� ���� local root �ٷ� ������
	// �� ���� DXR�� ����ϴ� ����̴�.

	int m_nMeshIndex = -1;		// �� ������Ʈ�� �����ϴ� Mesh Index
	int m_nParentIndex = -1;	// �� ������Ʈ�� �θ� GameObject�ε���
	int m_nHitGroupIndex = -1;	// � HitGroup�� ���ų�?

};

// ==================================================================

// �� ģ���� �����ӿ��� �ʿ��� ��ĸ� ��Ƽ� ������۸� ����� set�ϴ� ����
class CSkinningInfo {
public:
	CSkinningInfo(std::ifstream& inFile, UINT nRefMesh);
	CSkinningInfo(const CSkinningInfo& other);

	void MakeAnimationMatrixIndex(std::vector<std::string>& vFrameNames);
	void MakeBufferAndDescriptorHeap(ComPtr<ID3D12Resource>& pMatrixBuffer, UINT nElements);

	void SetShaderVariables();

	UINT getRefMeshIndex() const { return m_nRefMesh; }
private:
	UINT m_nBonesPerVertex{};	// ���� �� ��� �� ����
	UINT m_nBones{};			// �� ����
	UINT m_nVertexCount{};		// �ش� �޽��� ���� ����, ����Ȯ, Ȯ�� �ʿ�

	UINT m_nRefMesh{};			// �����ϴ� �޽��� ��ȣ

	std::vector<std::string> m_vBoneNames{};	// �� �̸� ����Ʈ
	std::vector<XMFLOAT4X4> m_vOffsetMatrix{};	// offset ���
	std::vector<UINT> m_vBoneIndices{};			// �� �ε���
	std::vector<float> m_vBoneWeight{};			// �� ����ġ
	std::vector<UINT>m_vAnimationMatrixIndex{};	// ���� �ִϸ��̼� ����� ã�� ���� �ε����� �ϳ� �� ������ش�.

	ComPtr<ID3D12DescriptorHeap> m_pd3dDesciptorHeap{};

	ComPtr<ID3D12Resource> m_pConstantBuffer{};
	ComPtr<ID3D12Resource> m_pOffsetMatrixBuffer{};
	ComPtr<ID3D12Resource> m_pBoneIndicesBuffer{};
	ComPtr<ID3D12Resource> m_pBoneWeightBuffer{};
	ComPtr<ID3D12Resource> m_pAnimationMatrixIndexBuffer{};
};

class CSkinningObject {
public:
	CSkinningObject();

	virtual void CopyFromOtherObject(CSkinningObject* other);

	void AddResourceFromFile(std::ifstream& inFile, std::string strFront);
	void AddObjectFromFile(std::ifstream& inFile, int nParentIndex = -1);
	void AddMaterialFromFile(std::ifstream& inFile, int nCurrentIndex);

	virtual void PrepareObject() {};
	virtual void UpdateAnimationMatrixes();
	virtual void UpdateObject(float fElapsedTime) {};
	virtual void ReBuildBLAS() {}

	void InitializeGameObjectCBuffer();
	void setPreTransform(float scale, XMFLOAT3 rotate, XMFLOAT3 position);
	void UpdateFrameWorldMatrix();
	void UpdateWorldMatrix();

	void SetPosition(XMFLOAT3 pos);
	void Rotate(XMFLOAT3 rot);
	void move(float fElapsedTime, short arrow);

	std::string getName() const { return m_strObjectName; }
	std::vector<std::unique_ptr<CSkinningInfo>>& getSkinningInfo();
	std::vector<std::unique_ptr<CGameObject>>& getObjects() { return m_vObjects; }
	std::vector<std::shared_ptr<Mesh>>& getMeshes() { return m_vMeshes; }
	std::vector<std::shared_ptr<CTexture>>& getTextures() { return m_vTextures; }

	virtual std::vector<ComPtr<ID3D12Resource>>& getBLAS() = 0;
protected:
	std::string m_strObjectName{};

	std::string FilePathFront{};
	std::vector<std::unique_ptr<CGameObject>> m_vObjects{};
	std::vector<std::shared_ptr<Mesh>> m_vMeshes{};
	std::vector<std::shared_ptr<CTexture>> m_vTextures{};
	std::vector<std::unique_ptr<CSkinningInfo>> m_vSkinningInfo{};

	// XMFLOAT4X4
	XMFLOAT4X4 m_xmf4x4WorldMatrix{};
	XMFLOAT4X4 m_xmf4x4PreTransformMatrix{};	// �޽��� ������ �����ϱ� ���� ���
	bool m_bUsePreTransform = false;

	XMFLOAT3 m_xmf3Right{};
	XMFLOAT3 m_xmf3Up{};
	XMFLOAT3 m_xmf3Look{};
	XMFLOAT3 m_xmf3Position{};
};

// Rasterizer�� RayTracing������ Skinning Animation�� ����� �ٸ���
// ����Ʈ���̽� �� ��Ű�� ������Ʈ, BLAS�� ������ ����
class CRayTracingSkinningObject : public CSkinningObject {
public:
	void PrepareObject();
	void UpdateObject(float fElapsedTime);
	void ReBuildBLAS();

	std::vector<ComPtr<ID3D12Resource>>& getBLAS() { return m_vBLAS; }
protected:
	// ===================================================
	void MakeBLAS();
	void InitBLAS(ComPtr<ID3D12Resource>& resource, std::shared_ptr<Mesh>& mesh);

	void ReadyOutputVertexBuffer();
	// ===================================================
	std::vector<ComPtr<ID3D12Resource>> m_vBLAS{};
	ComPtr<ID3D12Resource> m_pScratchBuffer{};
	UINT64 m_nScratchSize{};

	std::vector<ComPtr<ID3D12Resource>> m_vOutputVertexBuffer{};
	std::vector<ComPtr<ID3D12DescriptorHeap>> m_vUAV{};
};