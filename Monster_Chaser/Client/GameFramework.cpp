// ==============================================
// GameFramework.cpp
// 설명 추가
// ==============================================

#include "GameFramework.h"

bool CGameFramework::OnInit(HWND hWnd, HINSTANCE hInstance)
{
	m_hWnd = hWnd; m_hInstance = hInstance;
	
	g_DxResource.device = m_pd3dDevice.Get();
}

void CGameFramework::InitDevice()
{
	// Create Factory And Device
	::CreateDXGIFactory(IID_PPV_ARGS(&m_pdxgiFactory));
	ComPtr<IDXGIAdapter> adapter{};

	::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_pd3dDevice));
	if (!m_pd3dDevice) {
		OutputDebugString(L"Device not Created\n");
		exit(0);
	}

	// Create Fence
	m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pd3dFence));
}

void CGameFramework::CheckRayTracingSupport()
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5{};
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5));
	if (options5.RaytracingTier < D3D12_RAYTRACING_TIER_1_0) {
		OutputDebugString(L"This Device Not Support RayTracing!\n");
		m_bRayTracingSupport = false;
		m_bRaster = true;
	}
	else {
		m_bRayTracingSupport = true;
		m_bRaster = false;
	}
}

void CGameFramework::InitCommand()
{
	D3D12_COMMAND_QUEUE_DESC desc{};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	m_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pd3dCommandQueue));
	m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pd3dCommandAllocator));
	m_pd3dDevice->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_pd3dCommandList));
}

void CGameFramework::InitSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 desc{};
	desc.Width = DEFINED_UAV_BUFFER_WIDTH;
	desc.Height = DEFINED_UAV_BUFFER_HEIGHT;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc = NO_AA;
	desc.BufferCount = 2;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	IDXGISwapChain1* swapchain1{};
	m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue.Get(), m_hWnd, &desc, nullptr, nullptr, &swapchain1);
	swapchain1->QueryInterface(m_pdxgiSwapChain.GetAddressOf());
	swapchain1->Release();

	if (!m_pdxgiSwapChain) {
		OutputDebugString(L"Not SwapChain Created\n");
		exit(0);
	}
}

void CGameFramework::InitRTVDSV()
{
	// RTV
	D3D12_DESCRIPTOR_HEAP_DESC heapdesc{};
	heapdesc.NumDescriptors = 2;
	heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_pd3dDevice->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&m_pd3dRenderTargetView));

	m_nRTVIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle = m_pd3dRenderTargetView->GetCPUDescriptorHandleForHeapStart();

	for (int i = 0; i < 2; ++i) {
		m_pdxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pd3dBackBuffer[i]));
		m_pd3dDevice->CreateRenderTargetView(m_pd3dBackBuffer[i].Get(), nullptr, cpuDescriptorHandle);
		cpuDescriptorHandle.ptr += m_nRTVIncrementSize;
	}

	heapdesc.NumDescriptors = 1;
	heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	m_pd3dDevice->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&m_pd3dDepthStencilView));

	D3D12_RESOURCE_DESC resourceDesc = BASIC_BUFFER_DESC;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.Width = DEFINED_UAV_BUFFER_WIDTH;
	resourceDesc.Height = DEFINED_UAV_BUFFER_HEIGHT;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	m_pd3dDevice->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, nullptr, IID_PPV_ARGS(&m_pd3dDepthStencilBuffer));

	UINT mDSVIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	cpuDescriptorHandle = m_pd3dDepthStencilView->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), nullptr, cpuDescriptorHandle);
}

void CGameFramework::InitOutputBuffer()
{

}

void CGameFramework::Render()
{

}