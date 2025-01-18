#pragma once
#include "stdafx.h"

extern DXResources g_DxResource;

class CRayTracingPipeline {
public:
	// 다음과 같은 순서로 만들어라
	void AddLibrarySubObject(auto compiledShader);
	void AddHitGroupSubObject(wchar_t* exportName, wchar_t* ClosestHit = nullptr, wchar_t* AnyHit = nullptr, wchar_t* Intersect = nullptr);			// ClosestHit, AnyHit, Intersect
	void AddShaderConfigSubObject(UINT nMaxAttributeSize, UINT nMaxPayloadSize);
	void AddLocalRootAndAsoociationSubObject(ID3D12RootSignature* pLocalRootSignature);
	void AddGlobalRootSignatureSubObject(ID3D12RootSignature* pGlobalRootSignature);
	void AddPipelineConfigSubObject(UINT nMaxTraceDepth);

	void MakePipelineState();
private:
	ComPtr<ID3D12StateObject> m_pRaytracingPipelineState{};
	
	std::vector<D3D12_STATE_SUBOBJECT> m_vSuvobjects;
	std::vector<LPCWSTR> m_exports;
};

