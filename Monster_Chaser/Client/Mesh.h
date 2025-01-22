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

class Mesh {
public:
	Mesh(std::ifstream& inFile);		// ctor-�޽� ��� ����					

	void GetMeshNameFromFile(std::ifstream& inFile);
	void GetBoundInfoFromFile(std::ifstream& inFile);
	void GetPositionFromFile(std::ifstream& inFile);

	void MakeResourceView(ComPtr<ID3D12DescriptorHeap>& pDescriptor, ComPtr<ID3D12Resource>& pResource);

	std::string getName() const;

	ID3D12Resource* getVertexBuffer() const;

protected:
private:
	std::string m_MeshName{};							// Mesh�� �̸�

	BoundingOrientedBox m_OBB{};						// Oriented_Bounding_Box

	UINT m_nVertexCount{};								// <Positions> ����
	ComPtr<ID3D12Resource> m_pd3dVertexBuffer{};		// DXGI_FORMAT_R32G32B32_FLOAT
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};	// GPU_DESCRIPTOR_HANDLE�� ��� ���� view
	bool m_bHasVertex = false;

	UINT m_nColorCount{};								// <Colors> ����
	ComPtr<ID3D12Resource> m_pd3dColorsBuffer{};		// ?
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};
	bool m_bHasColor = false;

	UINT m_nTexCoord0Count{};							// <TextureCoords0>
	ComPtr<ID3D12Resource> m_pd3dTexCoord0Buffer{};		// �Ƹ��� DXGI_FORMAT_R32G32B32_UNORM;
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};
	bool m_bHasTex0 = false;

	UINT m_nTexCoord1Count{};							// <TextureCoords1>
	ComPtr<ID3D12Resource> m_pd3dTexCoord1Buffer{};		// �Ƹ��� DXGI_FORMAT_R32G32B32_UNORM;
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};
	bool m_bHasTex1 = false;

	UINT m_nNormalsCount{};								// <Normals>
	ComPtr<ID3D12Resource> m_pd3dNormalsBuffer{};		// ����ȭ����
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};
	bool m_bHasNormals = false;

	UINT m_nTangentsCount{};							// <Tangents>
	ComPtr<ID3D12Resource> m_pd3dTangentsBuffer{};	
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};
	bool m_bHasTangents = false;

	UINT m_nBiTangentsCount{};							// <BiTangents>
	ComPtr<ID3D12Resource> m_pd3dBiTangentsBuffer{};
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};
	bool m_bHasBiTangents = false;

	UINT m_nSubMeshesCount{};							// SubMesh���� (vector::size()�� �˾ƿ� �� �ִ�, �ʿ��Ѱ�?, reserve�Ҷ� ��� ����)
	std::vector<ComPtr<ID3D12Resource>> m_vSubMeshes;	// SubMesh(indexbuffer)�� ����� �迭
	std::vector<ComPtr<ID3D12DescriptorHeap>> m_vSubMeshViews;
	std::vector<UINT> m_vIndices;						// �� SubMesh�� ���� Index ����
	bool m_bHasSubMeshes = false;

	
};

