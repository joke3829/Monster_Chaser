// ==============================================
// GameFramework.cpp
// 설명 추가
// ==============================================

#include "GameFramework.h"

bool CGameFramework::OnInit(HWND hWnd, HINSTANCE hInstance)
{
	m_hWnd = hWnd; m_hInstance = hInstance;

	// device, fence 초기화
	InitDevice();
	
	// RayTracing 지원 여부 확인
	CheckRayTracingSupport();

	// Command요소 생성
	InitCommand();

	// SwapChain 생성
	InitSwapChain();

	// RTV, DSV
	InitRTVDSV();

	// UAV Buffer
	if (m_bRayTracingSupport)
		InitOutputBuffer();
	
	m_bRaster = true;
	g_DxResource.device = m_pd3dDevice.Get();
	g_DxResource.cmdAlloc = m_pd3dCommandAllocator.Get();
	g_DxResource.cmdList = m_pd3dCommandList.Get();
	g_DxResource.cmdQueue = m_pd3dCommandQueue.Get();
	g_DxResource.fence = m_pd3dFence.Get();

	return true;
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
	desc.Width = DEFINED_UAV_BUFFER_WIDTH;	// 바뀔 수 있다.
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
	resourceDesc.Width = DEFINED_UAV_BUFFER_WIDTH;		// 바뀔 수 있다
	resourceDesc.Height = DEFINED_UAV_BUFFER_HEIGHT;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	m_pd3dDevice->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, nullptr, IID_PPV_ARGS(&m_pd3dDepthStencilBuffer));

	cpuDescriptorHandle = m_pd3dDepthStencilView->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), nullptr, cpuDescriptorHandle);
}

void CGameFramework::InitOutputBuffer()
{

}

void CGameFramework::Render()
{
	auto barrier = [&](ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		{
			D3D12_RESOURCE_BARRIER resBarrier{};
			resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resBarrier.Transition.pResource = pResource;
			resBarrier.Transition.StateBefore = before;
			resBarrier.Transition.StateAfter = after;
			resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			m_pd3dCommandList->ResourceBarrier(1, &resBarrier);
		};
	if (m_bRaster) {
		m_pd3dCommandAllocator->Reset();
		m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), nullptr);

		UINT nCurrentBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

		barrier(m_pd3dBackBuffer[nCurrentBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		D3D12_CPU_DESCRIPTOR_HANDLE d3dCPUHandle = m_pd3dRenderTargetView->GetCPUDescriptorHandleForHeapStart();
		d3dCPUHandle.ptr += (m_nRTVIncrementSize * nCurrentBufferIndex);
		float colors[] = { 0.5f, 0.5f, 1.0f, 1.0f };
		m_pd3dCommandList->ClearRenderTargetView(d3dCPUHandle, colors, 0, nullptr);

		d3dCPUHandle = m_pd3dDepthStencilView->GetCPUDescriptorHandleForHeapStart();
		m_pd3dCommandList->ClearDepthStencilView(d3dCPUHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		// 여기에 렌더링 코드


		barrier(m_pd3dBackBuffer[nCurrentBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		m_pd3dCommandList->Close();
		m_pd3dCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pd3dCommandList.GetAddressOf()));
		Flush();

		m_pdxgiSwapChain->Present(0, 0);
	}
	else {

	}
}