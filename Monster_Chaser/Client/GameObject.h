#pragma once
#include "Mesh.h"
#include "Texture.h"

extern DXResources g_DxResource;

struct Material {	// 명시적으로 쓸라고 이따구로 쓴거에요
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

class CGameObject {
public:
	bool InitializeObjectFromFile(std::ifstream& inFile);

	std::vector<Material>& getMaterials();

	void SetMeshIndex(int index);
	void SetParentIndex(int index);

	void InitializeAxis();
protected:
	std::string m_strName{};

	XMFLOAT3 m_xmf3Pos{};
	XMFLOAT3 m_xmf3Scale{};

	XMFLOAT4X4 m_xmf4x4LocalMatrix{};

	XMFLOAT3 m_xmf3Right{};
	XMFLOAT3 m_xmf3Up{};
	XMFLOAT3 m_xmf3Look{};

	std::vector<Material> m_vMaterials;
	// 상수버퍼로 넘길 리소스가 필요한가? 일단 보류
	// 상수 버퍼로 넘길거는 마테리얼의 AlbedoColor, EmissiveColor, Glossiness, Metalic, SpecularHighlight, 
	// 텍스쳐의 경우는 local root 바로 보낼것
	// 위 경우는 DXR을 사용하는 경우이다.

	int m_nMeshIndex = -1;		// 이 오브젝트가 참조하는 Mesh Index
	int m_nParentIndex = -1;	// 이 오브젝트의 부모 GameObject인덱스


};

