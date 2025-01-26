#include "RayTracingPipeline.h"

void CRayTracingPipeline::AddLibrarySubObject(auto compiledShader)
{
	D3D12_DXIL_LIBRARY_DESC lib{};
	lib.DXILLibrary.pShaderBytecode = compiledShader;
	lib.DXILLibrary.BytecodeLength = sizeof(compiledShader) / sizeof(unsigned char);

	D3D12_STATE_SUBOBJECT temp{};
	temp.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	temp.pDesc = &lib;

	m_vSuvobjects.push_back(temp);
}

// 사용할 것만 넣기
void CRayTracingPipeline::AddHitGroupSubObject(wchar_t* exportName, wchar_t* ClosestHit, wchar_t* AnyHit, wchar_t* Intersect)
{
	D3D12_HIT_GROUP_DESC hitGroup{};
	hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;	// 바뀔 수도 있음
	hitGroup.ClosestHitShaderImport = ClosestHit;
	hitGroup.AnyHitShaderImport = AnyHit;
	hitGroup.IntersectionShaderImport = Intersect;
	hitGroup.HitGroupExport = exportName;

	// export를 저장한다.
	m_exports.push_back(exportName);

	D3D12_STATE_SUBOBJECT temp{};
	temp.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
	temp.pDesc = &hitGroup;
	m_vSuvobjects.push_back(temp);
}

void CRayTracingPipeline::AddShaderConfigSubObject(UINT nMaxAttributeSize, UINT nMaxPayloadSize)
{
	D3D12_RAYTRACING_SHADER_CONFIG shaderCfg{};
	shaderCfg.MaxAttributeSizeInBytes = nMaxAttributeSize;
	shaderCfg.MaxPayloadSizeInBytes = nMaxPayloadSize;

	D3D12_STATE_SUBOBJECT temp{};
	temp.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	temp.pDesc = &shaderCfg;
	m_vSuvobjects.push_back(temp);
}

void CRayTracingPipeline::AddLocalRootAndAsoociationSubObject(ID3D12RootSignature* pLocalRootSignature)
{
	D3D12_LOCAL_ROOT_SIGNATURE localrootsig{ .pLocalRootSignature = pLocalRootSignature };

	D3D12_STATE_SUBOBJECT localrootSubObject{};
	localrootSubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
	localrootSubObject.pDesc = &localrootsig;
	m_vSuvobjects.push_back(localrootSubObject);

	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION exportsAss{};
	exportsAss.pSubobjectToAssociate = &localrootSubObject;
	exportsAss.NumExports = m_exports.size();
	exportsAss.pExports = m_exports.data();
	
	D3D12_STATE_SUBOBJECT temp{};
	temp.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
	temp.pDesc = &exportsAss;
	m_vSuvobjects.push_back(temp);
}

void CRayTracingPipeline::AddGlobalRootSignatureSubObject(ID3D12RootSignature* pGlobalRootSignature)
{
	D3D12_GLOBAL_ROOT_SIGNATURE globalSig = { .pGlobalRootSignature = pGlobalRootSignature };
	D3D12_STATE_SUBOBJECT temp{};
	temp.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	temp.pDesc = &globalSig;
	m_vSuvobjects.push_back(temp);
}

void CRayTracingPipeline::AddPipelineConfigSubObject(UINT nMaxTraceDepth)
{
	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineCfg = { .MaxTraceRecursionDepth = nMaxTraceDepth };
	D3D12_STATE_SUBOBJECT temp{};
	temp.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	temp.pDesc = &pipelineCfg;
	m_vSuvobjects.push_back(temp);
}

void CRayTracingPipeline::MakePipelineState()
{
	D3D12_STATE_OBJECT_DESC desc{};
	desc.NumSubobjects = m_vSuvobjects.size();
	desc.pSubobjects = m_vSuvobjects.data();
	desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

	g_DxResource.device->CreateStateObject(&desc, IID_PPV_ARGS(&m_pRaytracingPipelineState));
}

ID3D12StateObject* CRayTracingPipeline::getPipelineState()
{
	return m_pRaytracingPipelineState.Get();
}

std::vector<LPCWSTR>& CRayTracingPipeline::getExports()
{
	return m_exports;
}