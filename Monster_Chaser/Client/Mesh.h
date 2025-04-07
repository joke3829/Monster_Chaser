//-----------------------------------------------------------------------------
// File: Mesh.h
// Mesh �����Ϳ� ������ �����ϴ� Ŭ����
// 
// 01.22
// �ϴ��� exporter�� ����Ǵ� ������ ��� ������,
// �ʿ���� ������ �ִ� �Ǵ��ϸ� ����
// ������ ���ҽ��� ���ǹ��� ������ ������ �ִ��� m_bHas�� �Ǵ�, �̰��� ��� ���۷�
// ���̴��� �Ѱ��ٰŴ�. 
// ���� Ȯ������ ������ ����޽��� ���� ��ŭ ���׸����� �����Ѵ� �Ǵ�
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
	Mesh(std::ifstream& inFile, std::string strMeshName);		// ctor-�޽� ��� ����	
	Mesh(CHeightMapImage* heightmap, std::string strMeshName);
	Mesh(XMFLOAT3& center, XMFLOAT3& extent, std::string meshName = "noNameMesh");					// boundingOBB ���鶧 ���

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
	std::string m_MeshName{};							// Mesh�� �̸�

	BoundingOrientedBox m_OBB{};						// Oriented_Bounding_Box

	UINT m_nVertexCount{};								// <Positions> ����
	ComPtr<ID3D12Resource> m_pd3dVertexBuffer{};		// DXGI_FORMAT_R32G32B32_FLOAT
	bool m_bHasVertex = false;

	UINT m_nColorCount{};								// <Colors> ����
	ComPtr<ID3D12Resource> m_pd3dColorsBuffer{};		// ?
	bool m_bHasColor = false;

	UINT m_nTexCoord0Count{};							// <TextureCoords0>
	ComPtr<ID3D12Resource> m_pd3dTexCoord0Buffer{};		// �Ƹ��� DXGI_FORMAT_R32G32B32_UNORM;
	bool m_bHasTex0 = false;

	UINT m_nTexCoord1Count{};							// <TextureCoords1>
	ComPtr<ID3D12Resource> m_pd3dTexCoord1Buffer{};		// �Ƹ��� DXGI_FORMAT_R32G32B32_UNORM;
	bool m_bHasTex1 = false;

	UINT m_nNormalsCount{};								// <Normals>
	ComPtr<ID3D12Resource> m_pd3dNormalsBuffer{};		// ����ȭ����
	bool m_bHasNormals = false;

	UINT m_nTangentsCount{};							// <Tangents>
	ComPtr<ID3D12Resource> m_pd3dTangentsBuffer{};	
	bool m_bHasTangents = false;

	UINT m_nBiTangentsCount{};							// <BiTangents>
	ComPtr<ID3D12Resource> m_pd3dBiTangentsBuffer{};
	bool m_bHasBiTangents = false;

	UINT m_nSubMeshesCount{};							// SubMesh���� (vector::size()�� �˾ƿ� �� �ִ�, �ʿ��Ѱ�?, reserve�Ҷ� ��� ����)
	std::vector<ComPtr<ID3D12Resource>> m_vSubMeshes;	// SubMesh(indexbuffer)�� ����� �迭
	std::vector<UINT> m_vIndices;						// �� SubMesh�� ���� Index ����, �̰� 0�̸� �������� �ʴ´�.
	bool m_bHasSubMeshes = false;

	bool m_bSkinningMesh = false;
};