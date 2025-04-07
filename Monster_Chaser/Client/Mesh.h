//-----------------------------------------------------------------------------
// File: Mesh.h
// Mesh 데이터와 동작을 관리하는 클래스
// 
// 01.22
// 일단은 exporter로 추출되는 정보를 모두 쓰는중,
// 필요없는 정보가 있단 판단하면 수정
// 각각의 리소스가 유의미한 정보를 가지고 있는지 m_bHas로 판단, 이것을 상수 버퍼로
// 셰이더에 넘겨줄거다. 
// 또한 확실하진 않지만 서브메시의 개수 만큼 마테리얼이 존재한다 판단
// 
//-----------------------------------------------------------------------------
#pragma once

#include "stdafx.h"

extern DXResources g_DxResource;

class CHeightMapImage {
public:
	CHeightMapImage(const wchar_t* filePath, int nWidth, int nLength, XMFLOAT3& xmf3Scale);
	float GetHeight(int x, int z);
	XMFLOAT3 GetNormal(int x, int z);

	std::unique_ptr<WORD[]> m_pHeightMapPixels;

	int m_nWidth;
	int m_nLength;

	XMFLOAT3 m_xmf3Scale;
};

// =============================================================================

class Mesh {
public:
	Mesh(std::ifstream& inFile, std::string strMeshName);		// ctor-메시 즉시 생성	
	Mesh(CHeightMapImage* heightmap, std::string strMeshName);
	Mesh(XMFLOAT3& center, XMFLOAT3& extent, std::string meshName = "noNameMesh");					// boundingOBB 만들때 사용

	//void GetMeshNameFromFile(std::ifstream& inFile);
	void GetBoundInfoFromFile(std::ifstream& inFile);
	void GetPositionFromFile(std::ifstream& inFile);
	void GetColorsFromFile(std::ifstream& inFile);
	void GetTexCoord0FromFile(std::ifstream& inFile);
	void GetTexCoord1FromFile(std::ifstream& inFile);
	void GetNormalFromFile(std::ifstream& inFile);
	void GetTangentFromFile(std::ifstream& inFile);
	void GetBiTangentFromFile(std::ifstream& inFile);
	void GetSubMeshesFromFile(std::ifstream& inFile);
	void MakeSubMesh(std::ifstream& inFile);

	void SetMeshName(std::string& name) { m_MeshName = name; }

	std::string getName() const;

	ID3D12Resource* getVertexBuffer() const;
	ID3D12Resource* getColorsBuffer() const;
	ID3D12Resource* getTexCoord0Buffer() const;
	ID3D12Resource* getTexCoord1Buffer() const;
	ID3D12Resource* getNormalsBuffer() const;
	ID3D12Resource* getTangentsBuffer() const;
	ID3D12Resource* getBiTangentsBuffer() const;
	ID3D12Resource* getIndexBuffer(UINT index) const;
	BoundingOrientedBox& getOBB() { return m_OBB; }

	UINT getVertexCount() const;
	UINT getIndexCount(int index) const;

	bool getHasVertex() const;
	bool getHasColor() const;
	bool getHasTex0() const;
	bool getHasTex1() const;
	bool getHasNormal() const;
	bool getHasTangent() const;
	bool getHasBiTangent() const;
	bool getHasSubmesh() const;
	bool getbSkinning() const { return m_bSkinningMesh; }

	UINT getSubMeshCount() const;
	
	void setSkinning(bool bSkinning) { m_bSkinningMesh = bSkinning; }
protected:
private:
	std::string m_MeshName{};							// Mesh의 이름

	BoundingOrientedBox m_OBB{};						// Oriented_Bounding_Box

	UINT m_nVertexCount{};								// <Positions> 개수
	ComPtr<ID3D12Resource> m_pd3dVertexBuffer{};		// DXGI_FORMAT_R32G32B32_FLOAT
	bool m_bHasVertex = false;

	UINT m_nColorCount{};								// <Colors> 개수
	ComPtr<ID3D12Resource> m_pd3dColorsBuffer{};		// ?
	bool m_bHasColor = false;

	UINT m_nTexCoord0Count{};							// <TextureCoords0>
	ComPtr<ID3D12Resource> m_pd3dTexCoord0Buffer{};		// 아마도 DXGI_FORMAT_R32G32B32_UNORM;
	bool m_bHasTex0 = false;

	UINT m_nTexCoord1Count{};							// <TextureCoords1>
	ComPtr<ID3D12Resource> m_pd3dTexCoord1Buffer{};		// 아마도 DXGI_FORMAT_R32G32B32_UNORM;
	bool m_bHasTex1 = false;

	UINT m_nNormalsCount{};								// <Normals>
	ComPtr<ID3D12Resource> m_pd3dNormalsBuffer{};		// 정규화겠지
	bool m_bHasNormals = false;

	UINT m_nTangentsCount{};							// <Tangents>
	ComPtr<ID3D12Resource> m_pd3dTangentsBuffer{};	
	bool m_bHasTangents = false;

	UINT m_nBiTangentsCount{};							// <BiTangents>
	ComPtr<ID3D12Resource> m_pd3dBiTangentsBuffer{};
	bool m_bHasBiTangents = false;

	UINT m_nSubMeshesCount{};							// SubMesh개수 (vector::size()로 알아올 수 있다, 필요한가?, reserve할때 사용 가능)
	std::vector<ComPtr<ID3D12Resource>> m_vSubMeshes;	// SubMesh(indexbuffer)가 저장된 배열
	std::vector<UINT> m_vIndices;						// 각 SubMesh에 대한 Index 개수, 이게 0이면 존재하지 않는다.
	bool m_bHasSubMeshes = false;

	bool m_bSkinningMesh = false;
};