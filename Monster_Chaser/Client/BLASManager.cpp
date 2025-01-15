#include "BLASManager.h"

void CBLASManager::MakeBLAS(ComPtr<ID3D12Resource>& asResource,
	ID3D12Resource* vertexBuffer, UINT vertexCount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
	ID3D12Resource* indexBuffer = nullptr, UINT indices = 0, DXGI_FORMAT indexFormat = DXGI_FORMAT_UNKNOWN)
{
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexBuffer.StartAddress = vertexBuffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = vertexStride;
	geometryDesc.Triangles.VertexCount = vertexCount;
	geometryDesc.Triangles.VertexFormat = vertexFormat;
	geometryDesc.Triangles.IndexBuffer = indexBuffer ? indexBuffer->GetGPUVirtualAddress() : 0;
	geometryDesc.Triangles.IndexCount = indices;
	geometryDesc.Triangles.IndexFormat = indexFormat;


	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = 1;
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.pGeometryDescs = &geometryDesc;

	MakeAccelerationStructure(inputs, asResource);
}

void CBLASManager::AddBLAS(ID3D12Resource* vertexBuffer, UINT vertexcount, UINT64 vertexStride, DXGI_FORMAT vertexFormat,
	ID3D12Resource* indexBuffer, UINT indices, DXGI_FORMAT indexFormat)
{
	ComPtr<ID3D12Resource> blas;
	MakeBLAS(blas, vertexBuffer, vertexcount, vertexStride, vertexFormat,
		indexBuffer, indices, indexFormat);

	m_vBLASList.push_back(blas);
}