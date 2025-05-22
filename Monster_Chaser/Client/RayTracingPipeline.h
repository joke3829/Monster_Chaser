#pragma once
#include "stdafx.h"

extern DXResources g_DxResource;

class CRayTracingPipeline {
public:
	~CRayTracingPipeline();
	void Setup(UINT nSubObject);
	// Make it in the following order
	void AddLibrarySubObject(const unsigned char* compiledShader, size_t shaderSize);
	void AddHitGroupSubObject(wchar_t* exportName, wchar_t* ClosestHit = nullptr, wchar_t* AnyHit = nullptr, wchar_t* Intersect = nullptr);			// ClosestHit, AnyHit, Intersect
	void AddShaderConfigSubObject(UINT nMaxAttributeSize, UINT nMaxPayloadSize);
	void AddLocalRootAndAsoociationSubObject(ID3D12RootSignature* pLocalRootSignature);
	void AddGlobalRootSignatureSubObject(ID3D12RootSignature* pGlobalRootSignature);
	void AddPipelineConfigSubObject(UINT nMaxTraceDepth);

	void MakePipelineState();

	ID3D12StateObject* getPipelineState();
	std::vector<LPCWSTR>& getExports();
private:
	ComPtr<ID3D12StateObject> m_pRaytracingPipelineState{};
	
	//std::vector<D3D12_STATE_SUBOBJECT> m_vSubobjects;
	// test
	std::unique_ptr<D3D12_STATE_SUBOBJECT[]> m_pSubobjects{};
	UINT m_nSubObjects{};
	UINT m_nCurrentIndex{};

	std::vector<LPCWSTR> m_exports;			// Types of hit groups
};

