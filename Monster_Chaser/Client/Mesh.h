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
	float GetHeightinWorldSpace(float x, float z);

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
	Mesh(XMFLOAT3& center, float radius, std::string meshName = "noNameMesh");
	Mesh(XMFLOAT3& center, float width, float height);			// Plane

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
	UINT getTexCoord0Count() const { return m_nTexCoord0Count; }

	bool getHasVertex() const;
	bool getHasColor() const;
	bool getHasTex0() const;
	bool getHasTex1() const;
	bool getHasNormal() const;
	bool getHasTangent() const;
	bool getHasBiTangent() const;
	bool getHasSubmesh() const;
	bool getbSkinning() const { return m_bSkinningMesh; }
	bool getHasBoundingBox() const { return m_bHasBoundingBox; }

	UINT getSubMeshCount() const;
	
	std::vector<XMFLOAT2>& getTex0() { return m_vTex0; }
	std::vector<XMFLOAT3> getPositions() const { return m_vPositions; }
	std::vector<UINT> getIndices() const { return m_vIndexs; }
	void setSkinning(bool bSkinning) { m_bSkinningMesh = bSkinning; }
protected:
private:
	std::string m_MeshName{};							// Mesh�� �̸�

	bool m_bHasBoundingBox = false;
	BoundingOrientedBox m_OBB{};						// Oriented_Bounding_Box

	UINT m_nVertexCount{};								// <Positions> ����
	ComPtr<ID3D12Resource> m_pd3dVertexBuffer{};		// DXGI_FORMAT_R32G32B32_FLOAT
	bool m_bHasVertex = false;

	UINT m_nColorCount{};								// <Colors> ����
	ComPtr<ID3D12Resource> m_pd3dColorsBuffer{};		// ?
	bool m_bHasColor = false;

	UINT m_nTexCoord0Count{};							// <TextureCoords0>
	std::vector<XMFLOAT2> m_vTex0{};
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

	std::vector<XMFLOAT3> m_vPositions;
	std::vector<UINT> m_vIndexs;
};

// ===============================================================================================

struct BVHNode {
	BoundingBox aabb;
	std::vector<size_t> triangleIndices;
	std::unique_ptr<BVHNode> left;
	std::unique_ptr<BVHNode> right;

	bool IsLeaf() const { return !left && !right; }
};

class BVHTree {
public:
	std::unique_ptr<BVHNode> root;

	BVHTree() : root(nullptr) {}
	void query(const DirectX::BoundingSphere& sphere, std::vector<size_t>& candidateTriangles) const {
		queryNode(root.get(), sphere, candidateTriangles);
	}

private:
	void queryNode(BVHNode* node, const DirectX::BoundingSphere& sphere, std::vector<size_t>& candidateTriangles) const {
		if (!node || !node->aabb.Intersects(sphere)) return;

		if (node->IsLeaf()) {
			candidateTriangles.insert(candidateTriangles.end(), node->triangleIndices.begin(), node->triangleIndices.end());
		}
		else {
			queryNode(node->left.get(), sphere, candidateTriangles);
			queryNode(node->right.get(), sphere, candidateTriangles);
		}
	}
};

class MeshCollider {
public:
	MeshCollider(Mesh& mesh);

	bool CheckCollision(const MeshCollider& other) const;

	void BuildBVH();
	std::unique_ptr<BVHNode> BuildBVHNode(const std::vector<size_t>& triangleIndices, size_t start, size_t end);
	void GetTriangleCentroid(size_t triangleIdx, XMFLOAT3& centroid) const;
	bool BVHCollisionTest(const BVHNode* node1, const BVHNode* node2) const;
	bool TriangleIntersects(const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2,
		const XMFLOAT3& u0, const XMFLOAT3& u1, const XMFLOAT3& u2) const;
	void collectTriangleIndices(const BVHNode* node, std::vector<size_t>& indices) const;

	std::vector<size_t> getTriangleIndices() const {
		std::vector<size_t> indices;
		if (m_bvhRoot) {
			collectTriangleIndices(m_bvhRoot.get(), indices);
		}
		return indices;
	}

	const std::vector<DirectX::XMFLOAT3>& getPositions() const { return m_vPositions; }
	const std::vector<UINT>& getIndices() const { return m_vIndices; }
	const DirectX::BoundingOrientedBox& getOBB() const { return m_OBB; }
	const std::string& getName() const { return m_MeshName; }

	const BVHTree& getBVHTree() const {
		return m_bvhTree;
	}

private:
	std::string m_MeshName;
	std::vector<DirectX::XMFLOAT3> m_vPositions;
	std::vector<UINT> m_vIndices;
	DirectX::BoundingOrientedBox m_OBB;
	std::unique_ptr<BVHNode> m_bvhRoot;
	BVHTree m_bvhTree;
};