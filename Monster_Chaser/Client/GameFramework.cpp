// ==============================================
// GameFramework.cpp
// 
// ==============================================

#include "GameFramework.h"

CGameFramework::~CGameFramework()
{
	::CloseHandle(m_hFenceHandle);
}

bool CGameFramework::OnInit(HWND hWnd, HINSTANCE hInstance)
{
	m_hWnd = hWnd; m_hInstance = hInstance;

	_tcscpy_s(m_pszFrameRate, _T("Client ("));

	// device, fence Create
	InitDevice();
	
	// RayTracing Support Check
	CheckRayTracingSupport();

	// Command Object Ready
	InitCommand();

	// SwapChain Ready
	InitSwapChain();

	// RTV, DSV
	InitRTVDSV();

	// UAV Buffer
	if (m_bRayTracingSupport)
		InitOutputBuffer();

	// Global Variable & Scene Ready
	
	g_DxResource.device = m_pd3dDevice.Get();
	g_DxResource.cmdAlloc = m_pd3dCommandAllocator.Get();
	g_DxResource.cmdList = m_pd3dCommandList.Get();
	g_DxResource.cmdQueue = m_pd3dCommandQueue.Get();
	g_DxResource.fence = m_pd3dFence.Get();
	g_DxResource.pFenceHandle = &m_hFenceHandle;

	// Ready NullResource
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = sizeof(XMFLOAT3);
	m_pd3dDevice->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(g_DxResource.nullBuffer.GetAddressOf()));

	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Width = desc.Height = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	m_pd3dDevice->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(g_DxResource.nullTexture.GetAddressOf()));

	m_pCamera = std::make_shared<CCamera>();
	m_pCamera->Setup(2);

	InitScene();

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
	m_hFenceHandle = ::CreateEvent(NULL, FALSE, FALSE, NULL);
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
	desc.Width = DEFINED_UAV_BUFFER_WIDTH;	// subject to change
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

	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
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
	resourceDesc.Width = DEFINED_UAV_BUFFER_WIDTH;		// Subject to change
	resourceDesc.Height = DEFINED_UAV_BUFFER_HEIGHT;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	m_pd3dDevice->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, nullptr, IID_PPV_ARGS(&m_pd3dDepthStencilBuffer));

	cpuDescriptorHandle = m_pd3dDepthStencilView->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), nullptr, cpuDescriptorHandle);
}

void CGameFramework::InitOutputBuffer()
{
	D3D12_DESCRIPTOR_HEAP_DESC heapdesc{};
	heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapdesc.NumDescriptors = 1;
	heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	m_pd3dDevice->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&m_pd3dOutputBufferView));

	D3D12_RESOURCE_DESC desc = BASIC_BUFFER_DESC;
	desc.Width = DEFINED_UAV_BUFFER_WIDTH;
	desc.Height = DEFINED_UAV_BUFFER_HEIGHT;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	m_pd3dDevice->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&m_pd3dOutputBuffer));

	D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc{};
	viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	m_pd3dDevice->CreateUnorderedAccessView(m_pd3dOutputBuffer.Get(), nullptr, &viewDesc, m_pd3dOutputBufferView->GetCPUDescriptorHandleForHeapStart());

}

void CGameFramework::InitScene()
{
	m_pScene = std::make_unique<CRaytracingWinterLandScene>();
	m_pScene->SetCamera(m_pCamera);
	m_pScene->SetUp(m_pd3dOutputBuffer);
}

LRESULT CALLBACK CGameFramework::WMMessageProcessing(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
	case WM_KEYUP:
		KeyboardProcessing(hWnd, nMessage, wParam, lParam);
		break;
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		MouseProcessing(hWnd, nMessage, wParam, lParam);
		break;
	}
	return 0;
}

void CGameFramework::KeyboardProcessing(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		//case 'p':
		case 'P':
			if (m_bRayTracingSupport) {
				m_bRaster = !m_bRaster;
			}
			break;
		default:
			m_pScene->OnProcessingKeyboardMessage(hWnd, nMessage, wParam, lParam);
			break;
		}
		break;
	case WM_KEYUP:
		break;
	}
}

void CGameFramework::MouseProcessing(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_LBUTTONDOWN:;
	case WM_LBUTTONUP:
	case WM_MOUSEMOVE:
		m_pScene->OnProcessingMouseMessage(hWnd, nMessage, wParam, lParam);
		break;
	}
}

void CGameFramework::ProcessInput(float fElapsedTime)
{
	m_pScene->ProcessInput(fElapsedTime);
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
	//m_Timer.Tick(60.0f);
	m_Timer.Tick();
	if (m_bRaster) {	// Not Used
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
		
		// Render Here(Set & Draw) ===================

		// ===========================================

		barrier(m_pd3dBackBuffer[nCurrentBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		m_pd3dCommandList->Close();
		m_pd3dCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pd3dCommandList.GetAddressOf()));
		Flush();

		m_pdxgiSwapChain->Present(0, 0);
	}
	else {	// RayTracing
		m_pd3dCommandAllocator->Reset();
		m_pd3dCommandList->Reset(m_pd3dCommandAllocator.Get(), nullptr);

		ProcessInput(m_Timer.GetTimeElapsed());
		// Render Here(Set & Draw) ===================

		m_pScene->UpdateObject(m_Timer.GetTimeElapsed());

		m_pScene->PrepareRender();

		m_pd3dCommandList->SetDescriptorHeaps(1, m_pd3dOutputBufferView.GetAddressOf());
		m_pd3dCommandList->SetComputeRootDescriptorTable(0, m_pd3dOutputBufferView->GetGPUDescriptorHandleForHeapStart());

		m_pScene->Render();
		// ===========================================

		ID3D12Resource* backBuffer{};
		m_pdxgiSwapChain->GetBuffer(m_pdxgiSwapChain->GetCurrentBackBufferIndex(), IID_PPV_ARGS(&backBuffer));

		barrier(m_pd3dOutputBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		barrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);

		m_pd3dCommandList->CopyResource(backBuffer, m_pd3dOutputBuffer.Get());

		barrier(m_pd3dOutputBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		barrier(backBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);

		backBuffer->Release();

		m_pd3dCommandList->Close();
		m_pd3dCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(m_pd3dCommandList.GetAddressOf()));

		Flush();

		m_pdxgiSwapChain->Present(0, 0);
	}

	m_Timer.GetFrameRate(m_pszFrameRate + 8, 37);
	SetWindowText(m_hWnd, m_pszFrameRate);
}