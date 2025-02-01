#pragma once
#include "Mesh.h"
#include "Texture.h"

extern DXResources g_DxResource;

struct Material {	// ��������� ����� �̵����� ���ſ���
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
	bool bHasAlbedoColor;
	bool bHasEmissiveColor;
	bool bHasSpecularColor;
	bool bHasGlossiness;
	bool bHasSmoothness;
	bool bHasMetallic;
	bool bHasSpecularHighlight;
	bool bHasGlossyReflection;

	bool bHasAlbedoMap;
	bool bHasSpecularMap;
	bool bHasNormalMap;
	bool bHasMetallicMap;
	bool bHasEmissionMap;
	bool bHasDetailAlbedoMap;
	bool bHasDetailNormalMap;

	XMFLOAT4 AlbedoColor;
	XMFLOAT4 EmissiveColor;
	XMFLOAT4 SpecularColor;
	float Glossiness;
	float Smoothness;
	float Metallic;
	float SpecularHighlight;
	float GlossyReflection;
};

struct HasMesh {
	bool bHasVertex = false;
	bool bHasColor = false;
	bool bHasTex0 = false;
	bool bHasTex1 = false;
	bool bHasNormals = false;
	bool bHasTangenrs = false;
	bool bHasBiTangents = false;
	bool bHasSubMeshes = false;
};

class CGameObject {
public:
	bool InitializeObjectFromFile(std::ifstream& inFile);

	void InitializeConstanctBuffer(std::vector<std::unique_ptr<Mesh>>& meshes);

	std::vector<Material>& getMaterials();
	int getMeshIndex() const;
	int getHitGroupIndex() const;
	ID3D12Resource* getCbuffer(int index) const;
	ID3D12Resource* getMeshCBuffer() const;

	XMFLOAT4X4 getWorldMatrix();

	void SetMeshIndex(int index);
	void SetParentIndex(int index);
	void SetHitGroupIndex(int index);

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

