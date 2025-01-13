// ===================================================
// GameFramework.h
// 게임의 틀?
// ===================================================
#pragma once
#include "stdafx.h"

class CGameFramework { 
public:

private:
	ComPtr<IDXGIFactory> m_pdxgiFactory{};
	ComPtr<IDXGISwapChain> m_pdxgiSwapChain{};
	ComPtr<ID3D12Device5> m_pd3dDevice{};
	
	ComPtr<ID3D12CommandAllocator> m_pd3dCommandAllocator{};
	ComPtr<ID3D12CommandQueue> m_pd3dCommandQueue{};
	ComPtr<ID3D12GraphicsCommandList4> m_pd3dCommandList{};

	ComPtr<ID3D12Fence> m_pd3dFence{};

	
};

