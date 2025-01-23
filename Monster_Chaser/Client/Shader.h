//-----------------------------------------------------------------------------
// File: Shader.h
// Shader 데이터와 동작을 관리하는 클래스
// 
// 진행하면서 내용 추가 예정
//-----------------------------------------------------------------------------
#pragma once
#include "stdafx.h"

class Shader
{
public:
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

private:
	ComPtr<ID3D12PipelineState> m_PipelineState{};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PipelineStateDesc{};

	ComPtr<ID3DBlob> m_VSblob{};
	ComPtr<ID3DBlob> m_PSblob{};
};

