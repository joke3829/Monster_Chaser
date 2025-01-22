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

class Mesh {
public:
	Mesh(std::ifstream& inFile);		// ctor-메시 즉시 생성					

	void GetMeshNameFromFile(std::ifstream& inFile);
	void GetBoundInfoFromFile(std::ifstream& inFile);
	void GetPositionFromFile(std::ifstream& inFile);

	void MakeResourceView(ComPtr<ID3D12DescriptorHeap>& pDescriptor, ComPtr<ID3D12Resource>& pResource);

	std::string getName() const;

	ID3D12Resource* getVertexBuffer() const;

protected:
private:
	std::string m_MeshName{};							// Mesh의 이름

	BoundingOrientedBox m_OBB{};						// Oriented_Bounding_Box

	UINT m_nVertexCount{};								// <Positions> 개수
	ComPtr<ID3D12Resource> m_pd3dVertexBuffer{};		// DXGI_FORMAT_R32G32B32_FLOAT
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};	// GPU_DESCRIPTOR_HANDLE을 얻기 위한 view
	bool m_bHasVertex = false;

	UINT m_nColorCount{};								// <Colors> 개수
	ComPtr<ID3D12Resource> m_pd3dColorsBuffer{};		// ?
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};
	bool m_bHasColor = false;

	UINT m_nTexCoord0Count{};							// <TextureCoords0>
	ComPtr<ID3D12Resource> m_pd3dTexCoord0Buffer{};		// 아마도 DXGI_FORMAT_R32G32B32_UNORM;
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};
	bool m_bHasTex0 = false;

	UINT m_nTexCoord1Count{};							// <TextureCoords1>
	ComPtr<ID3D12Resource> m_pd3dTexCoord1Buffer{};		// 아마도 DXGI_FORMAT_R32G32B32_UNORM;
	ComPtr<ID3D12DescriptorHeap> m_pd3dVertexBufferView{};
	bool m_bHasTex1 = false;

	UINT m_nNormalsCount{};								// <Normals>
	ComPtr<ID3D12Resource> m_pd3dNormalsBuffer{};		// 정규화겠지
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

	UINT m_nSubMeshesCount{};							// SubMesh개수 (vector::size()로 알아올 수 있다, 필요한가?, reserve할때 사용 가능)
	std::vector<ComPtr<ID3D12Resource>> m_vSubMeshes;	// SubMesh(indexbuffer)가 저장된 배열
	std::vector<ComPtr<ID3D12DescriptorHeap>> m_vSubMeshViews;
	std::vector<UINT> m_vIndices;						// 각 SubMesh에 대한 Index 개수
	bool m_bHasSubMeshes = false;

	
};

