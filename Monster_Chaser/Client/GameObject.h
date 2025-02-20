#pragma once
#include "Mesh.h"
#include "Texture.h"

extern DXResources g_DxResource;

struct Material {	// 명시적으로 쓸라고 이렇게 쓴 것
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
	bool InitializeObjectFromFile(std::ifstream& inFile);

	void InitializeConstanctBuffer(std::vector<std::unique_ptr<Mesh>>& meshes);

	void UpdateWorldMatrix();

	void testLocalMatrix(int i)
	{
		m_xmf4x4LocalMatrix._41 += (10 * i);
		m_xmf4x4LocalMatrix._43 += (10 * i);
	}

	std::string getFrameName() const;
	std::vector<Material>& getMaterials();
	int getMeshIndex() const;
	int getHitGroupIndex() const;
	int getParentIndex() const;
	ID3D12Resource* getCbuffer(int index) const;
	ID3D12Resource* getMeshCBuffer() const;

	XMFLOAT4X4 getWorldMatrix();
	XMFLOAT4X4 getLocalMatrix();

	void SetMeshIndex(int index);
	void SetParentIndex(int index);
	void SetHitGroupIndex(int index);
	void SetWorlaMatrix(XMFLOAT4X4& mtx);

	void InitializeAxis();
protected:
	std::string m_strName{};

	XMFLOAT3 m_xmf3Pos{};
	XMFLOAT3 m_xmf3Scale{};

	XMFLOAT4X4 m_xmf4x4LocalMatrix{};
	XMFLOAT4X4 m_xmf4x4WorldMatrix{};

	XMFLOAT3 m_xmf3Right{};
	XMFLOAT3 m_xmf3Up{};
	XMFLOAT3 m_xmf3Look{};

	std::vector<Material> m_vMaterials;
	std::vector<ComPtr<ID3D12Resource>> m_vCBuffers;		// 상수 버퍼 모음
	ComPtr<ID3D12Resource> m_pd3dMeshCBuffer{};
	// 상수버퍼로 넘길 리소스가 필요한가? 일단 보류
	// 상수 버퍼로 넘길거는 마테리얼의 AlbedoColor, EmissiveColor, Glossiness, Metalic, SpecularHighlight, 
	// 텍스쳐의 경우는 local root 바로 보낼것
	// 위 경우는 DXR을 사용하는 경우이다.

	int m_nMeshIndex = -1;		// 이 오브젝트가 참조하는 Mesh Index
	int m_nParentIndex = -1;	// 이 오브젝트의 부모 GameObject인덱스
	int m_nHitGroupIndex = -1;	// 어떤 HitGroup을 볼거냐?

};

// ==================================================================

// 이 친구는 프레임에서 필요한 행렬만 모아서 상수버퍼를 만들어 set하는 역할
class CSkinningInfo {
public:
	CSkinningInfo(std::ifstream& inFile, UINT nRefMesh);
private:
	UINT m_nBonesPerVertex{};	// 정점 당 사용 뼈 개수
	UINT m_nBones{};			// 뼈 개수

	UINT m_nRefMesh{};			// 참조하는 메시의 번호

	std::vector<std::string> m_vBoneNames{};	// 뼈 이름 리스트
	std::vector<XMFLOAT4X4> m_vOffsetMatrix{};	// offset 행렬 -> 셰이더 상에서 최대 몇개인지 모름
	std::vector<UINT> m_vBoneIndices{};			// 뼈 인덱스 -> 셰이더 상에서 최대 몇개인지 모름
	std::vector<float> m_vBoneWeight{};			// 뼈 가중치 -> 셰이더 상에서 최대 몇개인지 모름
	std::vector<UINT>m_vAnimationMatrixIndex{};	// 뼈의 애니메이션 행렬을 찾기 위해 인덱스를 하나 더 만들어준다.
};

class CSkinningObject {
public:
	void AddResourceFromFile(std::ifstream& inFile, std::string strFront);
	void AddObjectFromFile(std::ifstream& inFile, int nParentIndex = -1);
	void AddMaterialFromFile(std::ifstream& inFile, int nCurrentIndex);
protected:
	std::string m_strObjectName{};

	std::string FilePathFront{};
	std::vector<std::unique_ptr<CGameObject>> m_vObjects{};
	std::vector<std::shared_ptr<Mesh>> m_vMeshes{};
	std::vector<std::shared_ptr<CTexture>> m_vTextures{};
	std::vector<std::unique_ptr<CSkinningInfo>> m_vSkinningInfo{};
};

// Rasterizer와 RayTracing에서의 Skinning Animation은 방법이 다르다
// 레이트레이싱 용 스키닝 오브젝트, BLAS를 가지고 있음
class CRayTracingSkinningObject : public CSkinningObject {
public:
protected:
};