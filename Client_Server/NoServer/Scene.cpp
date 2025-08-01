﻿#include "Scene.h"

constexpr unsigned short NUM_G_ROOTPARAMETER = 6;

void CScene::CreateRTVDSV()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ID3D12Device5* device = g_DxResource.device;
	device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_RTV.GetAddressOf()));

	device->CreateRenderTargetView(m_pOutputBuffer.Get(), nullptr, m_RTV->GetCPUDescriptorHandleForHeapStart());

	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_DSV.GetAddressOf()));

	D3D12_RESOURCE_DESC resourceDesc = BASIC_BUFFER_DESC;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.Width = DEFINED_UAV_BUFFER_WIDTH;		// Subject to change
	resourceDesc.Height = DEFINED_UAV_BUFFER_HEIGHT;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	device->CreateCommittedResource(&DEFAULT_HEAP, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, nullptr, IID_PPV_ARGS(m_pDepthStencilBuffer.GetAddressOf()));
	device->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, m_DSV->GetCPUDescriptorHandleForHeapStart());
}

void CScene::CreateOrthoMatrixBuffer()
{
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = Align(sizeof(XMFLOAT4X4), 256);
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_cameraCB.GetAddressOf()));

	XMFLOAT4X4 ortho{};
	XMStoreFloat4x4(&ortho, XMMatrixTranspose(XMMatrixOrthographicLH(DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT, -1, 1)));

	void* temp{};
	m_cameraCB->Map(0, nullptr, &temp);
	memcpy(temp, &ortho, sizeof(XMFLOAT4X4));
	m_cameraCB->Unmap(0, nullptr);
}

// ==================================================================================

void TitleScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline)
{
	m_pOutputBuffer = outputBuffer;

	CreateRTVDSV();
	CreateRootSignature();
	CreatePipelineState();

	CreateOrthoMatrixBuffer();

	m_pResourceManager = std::make_unique<CResourceManager>();

	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\Title\\title1.dds"));
	m_vTitleUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vTitleUIs[m_vTitleUIs.size() - 1]->setPositionInViewport(0, 0);

	m_vTitleUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get()));
	m_vTitleUIs[m_vTitleUIs.size() - 1]->setPositionInViewport(0, 0);
	m_vTitleUIs[m_vTitleUIs.size() - 1]->setColor(0.0, 0.0, 0.0, 1.0);

	// Room Select =======================================================================================

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\RoomSelect\\background1.dds"));
	m_vRoomSelectUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vRoomSelectUIs[m_vRoomSelectUIs.size() - 1]->setPositionInViewport(0, 0);

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 440, 84));
	//textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\RoomSelect\\people.dds"));
	for (int i = 0; i < 10; ++i) {
		int j = i % 2;
		if (j == 0) {
			m_vRoomSelectUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get()));
			m_vRoomSelectUIs[m_vRoomSelectUIs.size() - 1]->setPositionInViewport(20 + 160, i / 2 * 100 + 20 + 180);
			m_vRoomSelectUIs[m_vRoomSelectUIs.size() - 1]->setColor(1.0, 1.0, 1.0, 0.5);
		}
		else {
			m_vRoomSelectUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get()));
			m_vRoomSelectUIs[m_vRoomSelectUIs.size() - 1]->setPositionInViewport(500 + 160, i / 2 * 100 + 20 + 180);
			m_vRoomSelectUIs[m_vRoomSelectUIs.size() - 1]->setColor(1.0, 1.0, 1.0, 0.5);
		}
	}
	peopleindex = m_vRoomSelectUIs.size();

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 30, 84));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\RoomSelect\\people.dds"));
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 3; ++j) {
			int k = i % 2;
			if (k == 0) {
				m_vRoomSelectUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
				m_vRoomSelectUIs[m_vRoomSelectUIs.size() - 1]->setPositionInViewport(350 + (j * 40) + 160, i / 2 * 100 + 20 + 180);
			}
			else {
				m_vRoomSelectUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
				m_vRoomSelectUIs[m_vRoomSelectUIs.size() - 1]->setPositionInViewport(830 + (j * 40) + 160, i / 2 * 100 + 20 + 180);
			}
		}
	}

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\RoomSelect\\SelectRoomText.dds"));
	m_vRoomSelectUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vRoomSelectUIs[m_vRoomSelectUIs.size() - 1]->setPositionInViewport(0, 0);

	// InRoom=======================================================================================

	int tempIndex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT));
	int temptxt = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\RoomSelect\\background1.dds"));
	m_vInRoomUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vInRoomUIs[m_vInRoomUIs.size() - 1]->setPositionInViewport(0, 0);

	backUIIndex = m_vInRoomUIs.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 296, 484));
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			textures.emplace_back(std::make_unique<CTexture>(std::format(L"src\\texture\\UI\\InRoom\\CharacterStanding{}.dds", j).data()));
			m_vInRoomUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
			m_vInRoomUIs[m_vInRoomUIs.size() - 1]->setPositionInViewport((i * 296) + (18 * (i + 1)), 18);
			//m_vInRoomUIs[m_vInRoomUIs.size() - 1]->setColor(0.0, 0.0, 0.0, 0.5);
		}
	}

	readyUIIndex = m_vInRoomUIs.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 175, 65));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InRoom\\ReadyText.dds"));
	for (int i = 0; i < 3; ++i) {
		m_vInRoomUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
		m_vInRoomUIs[m_vInRoomUIs.size() - 1]->setPositionInViewport((i * 296) + (18 * (i + 1)) + 130, 437);
	}

	// select button
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 300, 90));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InRoom\\SelectJobText.dds"));
	m_vInRoomUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size()- 1].get()));
	m_vInRoomUIs[m_vInRoomUIs.size() - 1]->setPositionInViewport(18, 610);

	// curtain
	m_vInRoomUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[tempIndex].get()));
	m_vInRoomUIs[m_vInRoomUIs.size() - 1]->setPositionInViewport(0, 0);
	m_vInRoomUIs[m_vInRoomUIs.size() - 1]->setColor(0.0, 0.0, 0.0, 0.0);

	// ==========================================================================

	m_vSelectCUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[tempIndex].get(), textures[temptxt].get()));
	m_vSelectCUIs[m_vSelectCUIs.size() - 1]->setPositionInViewport(0, 0);

	// arrow
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 100, 100));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\SelectC\\ArrowLeft.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\SelectC\\ArrowRight.dds"));
	m_vSelectCUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 2].get()));
	m_vSelectCUIs[m_vSelectCUIs.size() - 1]->setPositionInViewport(20, 310);
	m_vSelectCUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vSelectCUIs[m_vSelectCUIs.size() - 1]->setPositionInViewport(1150, 310);

	// back
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 300, 90));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\SelectC\\BackText.dds"));
	m_vSelectCUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vSelectCUIs[m_vSelectCUIs.size() - 1]->setPositionInViewport(18, 610);

	// ok
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\SelectC\\SelectText.dds"));
	m_vSelectCUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vSelectCUIs[m_vSelectCUIs.size() - 1]->setPositionInViewport(962, 610);

	CUIindex = m_vSelectCUIs.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 800, 550));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\SelectC\\CharacterInfo1.dds"));
	m_vSelectCUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vSelectCUIs[m_vSelectCUIs.size() - 1]->setPositionInViewport(240, 40);

	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\SelectC\\CharacterInfo2.dds"));
	m_vSelectCUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vSelectCUIs[m_vSelectCUIs.size() - 1]->setPositionInViewport(240, 40);

	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\SelectC\\CharacterInfo3.dds"));
	m_vSelectCUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get(), textures[textures.size() - 1].get()));
	m_vSelectCUIs[m_vSelectCUIs.size() - 1]->setPositionInViewport(240, 40);
}

void TitleScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN: {
		switch (m_nState) {
		case Title:
			m_nState = RoomSelect;
			break;
		case RoomSelect:
			switch (wParam) {
			case 'R':
				++userPerRoom[1];
				break;
			}
			break;
		case InRoom: {
			switch (wParam) {
			case 'R':
				userReadyState[local_uid] = !userReadyState[local_uid];
				break;
			case VK_BACK:
				--userPerRoom[currentRoom];
				m_nState = RoomSelect;
				break;
			}
			break;
		}
		}
		break;
	}
	case WM_KEYUP:
		break;
	}
}
void TitleScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_LBUTTONDOWN: {
		int mx = LOWORD(lParam);
		int my = HIWORD(lParam);

		switch (m_nState) {
		case Title:
			m_nState = RoomSelect;
			break;
		case RoomSelect:
			for (int i = 0; i < 10; ++i) {
				int j = i % 2;
				if (j == 0) {
					int x1 = 20 + 160, x2 = 460 + 160;
					int y1 = i / 2 * 100 + 20 + 180, y2 = i / 2 * 100 + 20 + 84 + 180;
					if (mx >= x1 && mx <= x2 && my >= y1 && my <= y2) {
						if (userPerRoom[i] < 3) {
							local_uid = userPerRoom[i]++;
							currentRoom = i;
							m_nState = InRoom;
							break;
						}
					}
				}
				else {
					int x1 = 500 + 160, x2 = 940 + 160;
					int y1 = i / 2 * 100 + 20 + 180, y2 = i / 2 * 100 + 20 + 84 + 180;
					if (mx >= x1 && mx <= x2 && my >= y1 && my <= y2) {
						if (userPerRoom[i] < 3) {
							local_uid = userPerRoom[i]++;
							currentRoom = i;
							m_nState = InRoom;
							break;
						}
					}
				}
			}
			break;
		case InRoom:
			if (mx >= 0 && mx < 960) {
				m_nState = SelectC;
				prevJob = userJob[local_uid];
			}
			break;
		case SelectC:
			if (mx >= 0 && mx < 960) {
				if (my >= 400) {
					userJob[local_uid] = prevJob;
					m_nState = InRoom;
				}
				else {
					--userJob[local_uid];
					if (userJob[local_uid] < 1)
						userJob[local_uid] = 3;
				}
			}
			else {
				if (my >= 400)
					m_nState = InRoom;
				else {
					++userJob[local_uid];
					if (userJob[local_uid] > 3)
						userJob[local_uid] = 1;
				}
			}
			break;
		}
		break;
	}
	case WM_LBUTTONUP: {
		break;
	}
	case WM_MOUSEMOVE: {
		break;
	}
	}
}

void TitleScene::CreateRootSignature()
{
	D3D12_DESCRIPTOR_RANGE tRange{};
	tRange.BaseShaderRegister = 0;
	tRange.NumDescriptors = 1;
	tRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	D3D12_ROOT_PARAMETER params[3]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[0].Descriptor.RegisterSpace = 0;
	params[0].Descriptor.ShaderRegister = 0;

	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[1].Descriptor.RegisterSpace = 0;
	params[1].Descriptor.ShaderRegister = 1;

	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 1;
	params[2].DescriptorTable.pDescriptorRanges = &tRange;

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};								// s0
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rtDesc{};
	rtDesc.NumParameters = 3;
	rtDesc.NumStaticSamplers = 1;
	rtDesc.pParameters = params;
	rtDesc.pStaticSamplers = &samplerDesc;
	rtDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* pBlob{};
	D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
	g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pGlobalRootSignature.GetAddressOf()));
	pBlob->Release();
}
void TitleScene::CreatePipelineState()
{
	ID3DBlob* pd3dVBlob{ nullptr };
	ID3DBlob* pd3dPBlob{ nullptr };
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineState{};
	d3dPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineState.pRootSignature = m_pGlobalRootSignature.Get();

	D3D12_INPUT_ELEMENT_DESC ldesc[3]{};
	ldesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3dPipelineState.InputLayout.pInputElementDescs = ldesc;
	d3dPipelineState.InputLayout.NumElements = 3;

	d3dPipelineState.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dPipelineState.DepthStencilState.StencilEnable = FALSE;

	d3dPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	d3dPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	d3dPipelineState.RasterizerState.AntialiasedLineEnable = FALSE;
	d3dPipelineState.RasterizerState.FrontCounterClockwise = FALSE;
	d3dPipelineState.RasterizerState.MultisampleEnable = FALSE;
	d3dPipelineState.RasterizerState.DepthClipEnable = FALSE;

	d3dPipelineState.BlendState.AlphaToCoverageEnable = FALSE;
	d3dPipelineState.BlendState.IndependentBlendEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendEnable = TRUE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dPipelineState.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	d3dPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineState.NumRenderTargets = 1;
	d3dPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineState.SampleDesc.Count = 1;
	d3dPipelineState.SampleMask = UINT_MAX;

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &pd3dVBlob, nullptr);
	d3dPipelineState.VS.BytecodeLength = pd3dVBlob->GetBufferSize();
	d3dPipelineState.VS.pShaderBytecode = pd3dVBlob->GetBufferPointer();

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", 0, 0, &pd3dPBlob, nullptr);
	d3dPipelineState.PS.BytecodeLength = pd3dPBlob->GetBufferSize();
	d3dPipelineState.PS.pShaderBytecode = pd3dPBlob->GetBufferPointer();
	
	g_DxResource.device->CreateGraphicsPipelineState(&d3dPipelineState, IID_PPV_ARGS(m_UIPipelineState.GetAddressOf()));

	if (pd3dVBlob)
		pd3dVBlob->Release();
	if (pd3dPBlob)
		pd3dPBlob->Release();
}

void TitleScene::UpdateObject(float fElapsedTime)
{
	switch (m_nState) {
	case Title:
		if (startTime < 3.0f)
			startTime += fElapsedTime;
		else {
			wOpacity -= 0.1f * fElapsedTime;
			if (wOpacity < 0.0f)
				wOpacity = 0.0f;
			m_vTitleUIs[1]->setColor(0.0, 0.0, 0.0, wOpacity);
		}
		break;
	case RoomSelect: {
		//m_vRoomSelectUIs[0]->Animation(fElapsedTime);
		for (int i = 0; i < 10; ++i) {
			for (int j = 0; j < 3; ++j) {
				if (j < userPerRoom[i]) {
					m_vRoomSelectUIs[(i * 3) + j + peopleindex]->setRenderState(true);
				}
				else
					m_vRoomSelectUIs[(i*3) + j + peopleindex]->setRenderState(false);
			}
		}
		break;
	}
	case InRoom: {
		//m_vInRoomUIs[0]->Animation(fElapsedTime);
		bool allready = true;
		for (int i = 0; i < 3; ++i) {
			if (i < userPerRoom[currentRoom]) {
				for (int j = 0; j < 3; ++j) {
					if(userJob[i] == j + 1)
						m_vInRoomUIs[backUIIndex + (i * 3) + j]->setRenderState(true);
					else
						m_vInRoomUIs[backUIIndex + (i * 3) + j]->setRenderState(false);
				}
				if(userReadyState[i])
					m_vInRoomUIs[readyUIIndex + i]->setRenderState(true);
				else {
					m_vInRoomUIs[readyUIIndex + i]->setRenderState(false);
					allready = false;
				}
			}
			else {
				for(int j = 0 ; j < 3; ++j)
					m_vInRoomUIs[backUIIndex + (i * 3) + j]->setRenderState(false);
				m_vInRoomUIs[readyUIIndex + i]->setRenderState(false);
			}
		}
		if (allready) {
			wOpacity = 0.0f;
			m_nState = GoLoading;
		}
		break;
	}
	case GoLoading: {
			wOpacity += 0.35f * fElapsedTime;
			if (wOpacity > 1.0f) {
				wOpacity = 1.0f;
				m_nNextScene = SCENE_PLAIN;
			}
			m_vInRoomUIs[m_vInRoomUIs.size() - 1]->setColor(0.0, 0.0, 0.0, wOpacity);
		break;
	}
	case SelectC: {
		//m_vSelectCUIs[0]->Animation(fElapsedTime);
		for (int i = CUIindex; i < CUIindex + 3; ++i) {
			if (userJob[local_uid] == i - CUIindex + 1)
				m_vSelectCUIs[i]->setRenderState(true);
			else
				m_vSelectCUIs[i]->setRenderState(false);
		}
		break;
	}
	}
}
void TitleScene::Render()
{
	ID3D12GraphicsCommandList4* cmdList = g_DxResource.cmdList;
	auto barrier = [&](ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		{
			D3D12_RESOURCE_BARRIER resBarrier{};
			resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resBarrier.Transition.pResource = pResource;
			resBarrier.Transition.StateBefore = before;
			resBarrier.Transition.StateAfter = after;
			resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmdList->ResourceBarrier(1, &resBarrier);
		};
	
	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_VIEWPORT vv{};
	vv.Width = DEFINED_UAV_BUFFER_WIDTH; vv.Height = DEFINED_UAV_BUFFER_HEIGHT; vv.MinDepth = 0.0f; vv.MaxDepth = 1.0f;
	cmdList->RSSetViewports(1, &vv);
	D3D12_RECT ss{ 0, 0, DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT };
	cmdList->RSSetScissorRects(1, &ss);
	cmdList->OMSetRenderTargets(1, &m_RTV->GetCPUDescriptorHandleForHeapStart(), FALSE, &m_DSV->GetCPUDescriptorHandleForHeapStart());
	cmdList->SetGraphicsRootSignature(m_pGlobalRootSignature.Get());
	cmdList->SetPipelineState(m_UIPipelineState.Get());
	cmdList->SetGraphicsRootConstantBufferView(0, m_cameraCB->GetGPUVirtualAddress());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	switch (m_nState) {
	case Title:
		for (auto& p : m_vTitleUIs)
			p->Render();
		break;
	case RoomSelect:
		for (auto& p : m_vRoomSelectUIs)
			p->Render();
		break;
	case InRoom:
	case GoLoading:
		for (auto& p : m_vInRoomUIs)
			p->Render();
		break;
	case SelectC:
		for (auto& p : m_vSelectCUIs)
			p->Render();
		break;
	}

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

// ==================================================================================

void CRaytracingScene::UpdateObject(float fElapsedTime)
{
	// compute shader & rootSignature set
	g_DxResource.cmdList->SetPipelineState(m_pAnimationComputeShader.Get());
	g_DxResource.cmdList->SetComputeRootSignature(m_pComputeRootSignature.Get());


	m_pResourceManager->UpdateSkinningMesh(fElapsedTime);
	Flush();
	// BLAS RebBuild
	m_pResourceManager->ReBuildBLAS();

	m_pResourceManager->UpdateWorldMatrix();

	m_pCamera->getEyeCalculateOffset();
	m_pCamera->UpdateViewMatrix();
	m_pAccelerationStructureManager->UpdateScene(m_pCamera->getEye());
}

void CRaytracingScene::PrepareRender()
{
	g_DxResource.cmdList->SetPipelineState1(m_pRaytracingPipeline->getPipelineState());
	g_DxResource.cmdList->SetComputeRootSignature(m_pGlobalRootSignature.Get());
}

void CRaytracingScene::Render()
{
	m_pCamera->SetShaderVariable();
	m_pAccelerationStructureManager->SetScene();
	m_pResourceManager->SetLights();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	g_DxResource.cmdList->SetComputeRootDescriptorTable(4, textures[textures.size() - 1]->getView()->GetGPUDescriptorHandleForHeapStart());

	D3D12_DISPATCH_RAYS_DESC raydesc{};
	raydesc.Depth = 1;
	raydesc.Width = DEFINED_UAV_BUFFER_WIDTH;
	raydesc.Height = DEFINED_UAV_BUFFER_HEIGHT;

	raydesc.RayGenerationShaderRecord.StartAddress = m_pShaderBindingTable->getRayGenTable()->GetGPUVirtualAddress();
	raydesc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	raydesc.MissShaderTable.StartAddress = m_pShaderBindingTable->getMissTable()->GetGPUVirtualAddress();
	raydesc.MissShaderTable.SizeInBytes = m_pShaderBindingTable->getMissSize();
	raydesc.MissShaderTable.StrideInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	raydesc.HitGroupTable.StartAddress = m_pShaderBindingTable->getHitGroupTable()->GetGPUVirtualAddress();
	raydesc.HitGroupTable.SizeInBytes = m_pShaderBindingTable->getHitGroupSize();
	raydesc.HitGroupTable.StrideInBytes = m_pShaderBindingTable->getHitGroupStride();

	g_DxResource.cmdList->DispatchRays(&raydesc);
}



//void CRaytracingScene::CreateRootSignature()
//{
//	{
//		// Global Root Signature
//		D3D12_DESCRIPTOR_RANGE rootRange{};								// u0
//		rootRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
//		rootRange.NumDescriptors = 1;
//		rootRange.BaseShaderRegister = 0;
//		rootRange.RegisterSpace = 0;
//
//		D3D12_DESCRIPTOR_RANGE cubeMapRange{};							// t3
//		cubeMapRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//		cubeMapRange.NumDescriptors = 1;
//		cubeMapRange.BaseShaderRegister = 3;
//		cubeMapRange.RegisterSpace = 0;
//
//		D3D12_DESCRIPTOR_RANGE terrainTRange[2]{};						// b0, space2
//		terrainTRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
//		terrainTRange[0].NumDescriptors = 1;
//		terrainTRange[0].BaseShaderRegister = 2;
//		terrainTRange[0].RegisterSpace = 0;
//
//		terrainTRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//		terrainTRange[1].NumDescriptors = 13;
//		terrainTRange[1].BaseShaderRegister = 4;
//		terrainTRange[1].RegisterSpace = 0;
//		terrainTRange[1].OffsetInDescriptorsFromTableStart = 1;
//
//		// 0. uavBuffer, 1. AS, 2. camera, 3. Lights, 4. Enviorment(cubeMap), 5. TerrainInfo
//		D3D12_ROOT_PARAMETER params[NUM_G_ROOTPARAMETER] = {};
//		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	// u0
//		params[0].DescriptorTable.NumDescriptorRanges = 1;
//		params[0].DescriptorTable.pDescriptorRanges = &rootRange;
//
//		params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;				// t0
//		params[1].Descriptor.RegisterSpace = 0;
//		params[1].Descriptor.ShaderRegister = 0;
//
//		params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;				// b0, space0
//		params[2].Descriptor.RegisterSpace = 0;
//		params[2].Descriptor.ShaderRegister = 0;
//
//		params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;				// b0, space1
//		params[3].Descriptor.RegisterSpace = 1;
//		params[3].Descriptor.ShaderRegister = 0;
//
//		params[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		params[4].DescriptorTable.NumDescriptorRanges = 1;
//		params[4].DescriptorTable.pDescriptorRanges = &cubeMapRange;
//
//		params[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		params[5].DescriptorTable.NumDescriptorRanges = 2;
//		params[5].DescriptorTable.pDescriptorRanges = terrainTRange;
//
//		D3D12_STATIC_SAMPLER_DESC samplerDesc{};								// s0
//		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
//		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
//		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
//		samplerDesc.RegisterSpace = 0;
//		samplerDesc.ShaderRegister = 0;
//		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
//
//		D3D12_ROOT_SIGNATURE_DESC rtDesc{};
//		rtDesc.NumParameters = NUM_G_ROOTPARAMETER;
//		rtDesc.NumStaticSamplers = 1;
//		rtDesc.pParameters = params;
//		rtDesc.pStaticSamplers = &samplerDesc;
//
//		ID3DBlob* pBlob{};
//		D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
//		g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pGlobalRootSignature.GetAddressOf()));
//		pBlob->Release();
//	}
//	{
//		// LocalRootSignature
//		D3D12_DESCRIPTOR_RANGE srvRange[7] = {};
//		srvRange[0].BaseShaderRegister = 2;		// t2, space0
//		srvRange[0].NumDescriptors = 1;
//		srvRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//		srvRange[0].RegisterSpace = 0;
//
//		srvRange[1].BaseShaderRegister = 2;		// t2, space1
//		srvRange[1].NumDescriptors = 1;
//		srvRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//		srvRange[1].RegisterSpace = 1;
//
//		srvRange[2].BaseShaderRegister = 2;		// t2, space2
//		srvRange[2].NumDescriptors = 1;
//		srvRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//		srvRange[2].RegisterSpace = 2;
//
//		srvRange[3].BaseShaderRegister = 2;		// t2, space3
//		srvRange[3].NumDescriptors = 1;
//		srvRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//		srvRange[3].RegisterSpace = 3;
//
//		srvRange[4].BaseShaderRegister = 2;		// t2, space4
//		srvRange[4].NumDescriptors = 1;
//		srvRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//		srvRange[4].RegisterSpace = 4;
//
//		srvRange[5].BaseShaderRegister = 2;		// t2, space5
//		srvRange[5].NumDescriptors = 1;
//		srvRange[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//		srvRange[5].RegisterSpace = 5;
//
//		srvRange[6].BaseShaderRegister = 2;		// t2, space6
//		srvRange[6].NumDescriptors = 1;
//		srvRange[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
//		srvRange[6].RegisterSpace = 6;
//
//		D3D12_ROOT_PARAMETER params[17] = {};
//		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// b1, space0
//		params[0].Descriptor.RegisterSpace = 0;
//		params[0].Descriptor.ShaderRegister = 1;
//
//		params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// b1, spcae1
//		params[1].Descriptor.RegisterSpace = 1;
//		params[1].Descriptor.ShaderRegister = 1;
//
//		params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space0
//		params[2].Descriptor.RegisterSpace = 0;
//		params[2].Descriptor.ShaderRegister = 1;
//
//		params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space1
//		params[3].Descriptor.RegisterSpace = 1;
//		params[3].Descriptor.ShaderRegister = 1;
//
//		params[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space2
//		params[4].Descriptor.RegisterSpace = 2;
//		params[4].Descriptor.ShaderRegister = 1;
//
//		params[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space3
//		params[5].Descriptor.RegisterSpace = 3;
//		params[5].Descriptor.ShaderRegister = 1;
//
//		params[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space4
//		params[6].Descriptor.RegisterSpace = 4;
//		params[6].Descriptor.ShaderRegister = 1;
//
//		params[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space5
//		params[7].Descriptor.RegisterSpace = 5;
//		params[7].Descriptor.ShaderRegister = 1;
//
//		params[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space6
//		params[8].Descriptor.RegisterSpace = 6;
//		params[8].Descriptor.ShaderRegister = 1;
//
//		params[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space7
//		params[9].Descriptor.RegisterSpace = 7;
//		params[9].Descriptor.ShaderRegister = 1;
//
//		// texture
//		params[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		params[10].DescriptorTable.NumDescriptorRanges = 1;
//		params[10].DescriptorTable.pDescriptorRanges = &srvRange[0];
//
//		params[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		params[11].DescriptorTable.NumDescriptorRanges = 1;
//		params[11].DescriptorTable.pDescriptorRanges = &srvRange[1];
//
//		params[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		params[12].DescriptorTable.NumDescriptorRanges = 1;
//		params[12].DescriptorTable.pDescriptorRanges = &srvRange[2];
//
//		params[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		params[13].DescriptorTable.NumDescriptorRanges = 1;
//		params[13].DescriptorTable.pDescriptorRanges = &srvRange[3];
//
//		params[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		params[14].DescriptorTable.NumDescriptorRanges = 1;
//		params[14].DescriptorTable.pDescriptorRanges = &srvRange[4];
//
//		params[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		params[15].DescriptorTable.NumDescriptorRanges = 1;
//		params[15].DescriptorTable.pDescriptorRanges = &srvRange[5];
//
//		params[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//		params[16].DescriptorTable.NumDescriptorRanges = 1;
//		params[16].DescriptorTable.pDescriptorRanges = &srvRange[6];
//
//		D3D12_ROOT_SIGNATURE_DESC rtDesc{};
//		rtDesc.NumParameters = 17;
//		rtDesc.NumStaticSamplers = 0;
//		rtDesc.pParameters = params;
//		rtDesc.pStaticSamplers = nullptr;
//		rtDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
//
//		ID3DBlob* pBlob{};
//		D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
//		g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pLocalRootSignature.GetAddressOf()));
//		pBlob->Release();
//	}
//}

void CRaytracingScene::CreateComputeRootSignature()
{
	D3D12_DESCRIPTOR_RANGE skinningRange[6]{};
	{
		skinningRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		skinningRange[0].NumDescriptors = 1;
		skinningRange[0].BaseShaderRegister = 0;
		skinningRange[0].RegisterSpace = 0;
		skinningRange[0].OffsetInDescriptorsFromTableStart = 0;

		skinningRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		skinningRange[1].NumDescriptors = 1;
		skinningRange[1].BaseShaderRegister = 0;
		skinningRange[1].RegisterSpace = 0;
		skinningRange[1].OffsetInDescriptorsFromTableStart = 1;

		skinningRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		skinningRange[2].NumDescriptors = 1;
		skinningRange[2].BaseShaderRegister = 1;
		skinningRange[2].RegisterSpace = 0;
		skinningRange[2].OffsetInDescriptorsFromTableStart = 2;

		skinningRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		skinningRange[3].NumDescriptors = 1;
		skinningRange[3].BaseShaderRegister = 2;
		skinningRange[3].RegisterSpace = 0;
		skinningRange[3].OffsetInDescriptorsFromTableStart = 3;

		skinningRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		skinningRange[4].NumDescriptors = 1;
		skinningRange[4].BaseShaderRegister = 3;
		skinningRange[4].RegisterSpace = 0;
		skinningRange[4].OffsetInDescriptorsFromTableStart = 4;

		skinningRange[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		skinningRange[5].NumDescriptors = 1;
		skinningRange[5].BaseShaderRegister = 4;
		skinningRange[5].RegisterSpace = 0;
		skinningRange[5].OffsetInDescriptorsFromTableStart = 5;
	}

	D3D12_DESCRIPTOR_RANGE insertRange[5]{};
	{
		insertRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		insertRange[0].NumDescriptors = 1;
		insertRange[0].BaseShaderRegister = 1;
		insertRange[0].RegisterSpace = 0;
		insertRange[0].OffsetInDescriptorsFromTableStart = 0;

		insertRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		insertRange[1].NumDescriptors = 1;
		insertRange[1].BaseShaderRegister = 5;
		insertRange[1].RegisterSpace = 0;
		insertRange[1].OffsetInDescriptorsFromTableStart = 1;

		insertRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		insertRange[2].NumDescriptors = 1;
		insertRange[2].BaseShaderRegister = 6;
		insertRange[2].RegisterSpace = 0;
		insertRange[2].OffsetInDescriptorsFromTableStart = 2;

		insertRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		insertRange[3].NumDescriptors = 1;
		insertRange[3].BaseShaderRegister = 7;
		insertRange[3].RegisterSpace = 0;
		insertRange[3].OffsetInDescriptorsFromTableStart = 3;

		insertRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		insertRange[4].NumDescriptors = 1;
		insertRange[4].BaseShaderRegister = 8;
		insertRange[4].RegisterSpace = 0;
		insertRange[4].OffsetInDescriptorsFromTableStart = 4;
	}

	D3D12_DESCRIPTOR_RANGE uavRange[4]{};
	{
		uavRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		uavRange[0].NumDescriptors = 1;
		uavRange[0].BaseShaderRegister = 0;
		uavRange[0].RegisterSpace = 0;
		uavRange[0].OffsetInDescriptorsFromTableStart = 0;

		uavRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		uavRange[1].NumDescriptors = 1;
		uavRange[1].BaseShaderRegister = 1;
		uavRange[1].RegisterSpace = 0;
		uavRange[1].OffsetInDescriptorsFromTableStart = 1;

		uavRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		uavRange[2].NumDescriptors = 1;
		uavRange[2].BaseShaderRegister = 2;
		uavRange[2].RegisterSpace = 0;
		uavRange[2].OffsetInDescriptorsFromTableStart = 2;

		uavRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		uavRange[3].NumDescriptors = 1;
		uavRange[3].BaseShaderRegister = 3;
		uavRange[3].RegisterSpace = 0;
		uavRange[3].OffsetInDescriptorsFromTableStart = 3;
	}

	// 0 - ani info, 1 - InputVertex, 2 - OutputVertex
	D3D12_ROOT_PARAMETER params[3]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[0].DescriptorTable.NumDescriptorRanges = 6;
	params[0].DescriptorTable.pDescriptorRanges = skinningRange;

	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[1].DescriptorTable.NumDescriptorRanges = 5;
	params[1].DescriptorTable.pDescriptorRanges = insertRange;

	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 4;
	params[2].DescriptorTable.pDescriptorRanges = uavRange;

	//D3D12_ROOT_PARAMETER params[9]{};
	//params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	//params[0].Descriptor.ShaderRegister = 0;

	//params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	//params[1].Descriptor.ShaderRegister = 0;

	//params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	//params[2].Descriptor.ShaderRegister = 1;

	//params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	//params[3].Descriptor.ShaderRegister = 2;

	//params[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	//params[4].Descriptor.ShaderRegister = 3;

	//params[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	//params[5].Descriptor.ShaderRegister = 4;

	//params[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	//params[6].Descriptor.ShaderRegister = 5;

	//params[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//params[7].DescriptorTable.NumDescriptorRanges = 1;
	//params[7].DescriptorTable.pDescriptorRanges = &uavRange;

	//// test
	//params[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	//params[8].Descriptor.ShaderRegister = 6;



	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.NumParameters = 3;
	desc.pParameters = params;
	desc.NumStaticSamplers = 0;
	desc.pStaticSamplers = nullptr;

	ID3DBlob* pBlob{};
	D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
	g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pComputeRootSignature.GetAddressOf()));
	pBlob->Release();
}

template<typename T, typename U>
	requires (HasGameObjectInterface<T> || HasSkinningObjectInterface<T>) && HasSkinningObjectInterface<U>
inline bool CRaytracingScene::CheckSphereCollision(const std::vector<std::unique_ptr<T>>& object1, const std::vector<std::unique_ptr<U>>& object2)
{
	bool collisionDetected = false;
	auto& meshes = m_pResourceManager->getMeshList();

	for (const std::unique_ptr<T>& obj1 : object1) {
		// ��-ĳ���� (���Ǿ)
		if constexpr (HasGameObjectInterface<T>) {
			int meshIndex = obj1->getMeshIndex();
			if (meshIndex != -1 && meshIndex < meshes.size() && meshes[meshIndex]->getHasVertex() && meshes[meshIndex]->getHasBoundingBox()) {
				DirectX::BoundingOrientedBox mapOBB;
				meshes[meshIndex]->getOBB().Transform(mapOBB, DirectX::XMLoadFloat4x4(&obj1->getWorldMatrix()));

				for (const auto& character : object2) {
					for (const auto& bone : character->getObjects()) {
						if (bone->getBoundingInfo() & 0x1100) { // ĳ���� ���Ǿ�
							DirectX::BoundingSphere boneSphere = bone->getObjectSphere();
							bone->getObjectSphere().Transform(boneSphere, DirectX::XMLoadFloat4x4(&bone->getWorldMatrix()));
							if (mapOBB.Intersects(boneSphere)) {
								collisionDetected = true;
								// �浹 ó��
							}
						}
					}
				}
			}
		}

		// ĳ����-ĳ���� (���Ǿ�-���Ǿ)
		if constexpr (HasSkinningObjectInterface<T>) {
			for (const auto& bone1 : obj1->getObjects()) {
				if (bone1->getBoundingInfo() & 0x1100) { // ĳ���� ���Ǿ�
					DirectX::BoundingSphere boneSphere1 = bone1->getObjectSphere();
					bone1->getObjectSphere().Transform(boneSphere1, DirectX::XMLoadFloat4x4(&bone1->getWorldMatrix()));

					for (const auto& character : object2) {
						if (obj1 != character) { // ���� ĳ���� ����
							for (const auto& bone2 : character->getObjects()) {
								if (bone2->getBoundingInfo() & 0x1100) { // ĳ���� ���Ǿ�
									DirectX::BoundingSphere boneSphere2 = bone2->getObjectSphere();
									bone2->getObjectSphere().Transform(boneSphere2, DirectX::XMLoadFloat4x4(&bone2->getWorldMatrix()));
									if (boneSphere1.Intersects(boneSphere2)) {
										collisionDetected = true;
										// �浹 ó��
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return collisionDetected;
}

template<typename T, typename U>
	requires (HasGameObjectInterface<T> || HasSkinningObjectInterface<T>) && HasSkinningObjectInterface<U>
inline void CRaytracingScene::CheckOBBCollisions(const std::vector<std::unique_ptr<T>>& object1, const std::vector<std::unique_ptr<U>>& object2)
{
	auto& meshes = m_pResourceManager->getMeshList();

	for (const std::unique_ptr<T>& obj1 : object1) {
		// ��-ĳ���� (OBB��)
		if constexpr (HasGameObjectInterface<T>) {
			int meshIndex = obj1->getMeshIndex();
			if (meshIndex != -1 && meshIndex < meshes.size() && meshes[meshIndex]->getHasVertex() && meshes[meshIndex]->getHasBoundingBox()) {
				DirectX::BoundingOrientedBox mapOBB;
				meshes[meshIndex]->getOBB().Transform(mapOBB, DirectX::XMLoadFloat4x4(&obj1->getWorldMatrix()));

				for (const auto& character : object2) {
					for (const auto& bone : character->getObjects()) {
						if (bone->getBoundingInfo() & 0x0011) { // ĳ���� OBB
							DirectX::BoundingOrientedBox boneOBB;
							bone->getObjectOBB().Transform(boneOBB, DirectX::XMLoadFloat4x4(&bone->getWorldMatrix()));
							if (mapOBB.Intersects(boneOBB)) {
								// �浹 ó��
							}
						}
					}
				}
			}
		}

		// ĳ����-ĳ���� (OBB-OBB��)
		if constexpr (HasSkinningObjectInterface<T>) {
			for (const auto& bone1 : obj1->getObjects()) {
				if (bone1->getBoundingInfo() & 0x0011) { // ĳ���� OBB
					DirectX::BoundingOrientedBox boneOBB1;
					bone1->getObjectOBB().Transform(boneOBB1, DirectX::XMLoadFloat4x4(&bone1->getWorldMatrix()));

					for (const auto& character : object2) {
						if (obj1 != character) { // ���� ĳ���� ����
							for (const auto& bone2 : character->getObjects()) {
								if (bone2->getBoundingInfo() & 0x0011) { // ĳ���� OBB
									DirectX::BoundingOrientedBox boneOBB2;
									bone2->getObjectOBB().Transform(boneOBB2, DirectX::XMLoadFloat4x4(&bone2->getWorldMatrix()));
									if (boneOBB1.Intersects(boneOBB2)) {
										// �浹 ó��
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void CRaytracingScene::TestCollision(const std::vector<std::unique_ptr<CGameObject>>& mapObjects, const std::vector<std::unique_ptr<CSkinningObject>>& characters)
{
	auto& meshes = m_pResourceManager->getMeshList();

	struct CollisionInfo {
		XMFLOAT3 normal;
		float depth;
		float meshHeight;
	};

	for (const auto& character : characters) {
		std::vector<CollisionInfo> collisions;

		for (const auto& mapObj : mapObjects) {
			int meshIndex = mapObj->getMeshIndex();
			if (meshIndex == -1 || meshIndex >= meshes.size()) continue;

			auto& mesh = meshes[meshIndex];
			if (!mesh->getHasVertex() || !mesh->getHasBoundingBox()) continue;

			BoundingOrientedBox mapOBB;
			mesh->getOBB().Transform(mapOBB, XMLoadFloat4x4(&mapObj->getWorldMatrix()));

			float meshHeight = mapOBB.Extents.y * 2.0f;

			for (const auto& bone : character->getObjects()) {
				if (bone->getBoundingInfo() & 0x1100) { // Sphere
					BoundingSphere boneSphere = bone->getObjectSphere();
					boneSphere.Transform(boneSphere, XMLoadFloat4x4(&bone->getWorldMatrix()));
					if (mapOBB.Intersects(boneSphere)) {
						XMFLOAT3 norm = CalculateCollisionNormal(mapOBB, boneSphere);
						float depth = CalculateDepth(mapOBB, boneSphere);
						collisions.push_back({ norm, depth, meshHeight });
					}
				}
			}
		}

		// 다중 충돌 처리: 가장 큰 침투 깊이 선택
		if (!collisions.empty()) {
			auto maxCollision = std::max_element(collisions.begin(), collisions.end(),[](const CollisionInfo& a, const CollisionInfo& b) { return a.depth < b.depth; });

			XMFLOAT3 norm = maxCollision->normal;
			float depth = maxCollision->depth;
			float meshHeight = maxCollision->meshHeight;

			XMVECTOR moveDir = XMLoadFloat3(&character->getMoveDirection());
			XMVECTOR normal = XMLoadFloat3(&norm);
			float dotProduct = XMVectorGetX(XMVector3Dot(moveDir, normal));
			if (dotProduct < 0.0f) { // 이동 방향이 법선과 반대
				character->sliding(depth, norm, meshHeight);
			}
		}
	}
}

void CRaytracingScene::TestShootCollision(const std::vector<std::unique_ptr<CProjectile>>& Objects, const std::vector<std::unique_ptr<CSkinningObject>>& characters)
{
	for (const auto& projectile : Objects) {
		BoundingOrientedBox projectileOBB = projectile->getObjects().getObjectOBB();
		projectileOBB.Transform(projectileOBB, XMLoadFloat4x4(&projectile->getObjects().getWorldMatrix()));

		for (size_t i = 1; i < characters.size(); ++i) {
			const auto& character = characters[i];
			for (const auto& bone : character->getObjects()) {
				if (bone->getBoundingInfo() & 0x1100) { // Sphere
					BoundingSphere boneSphere = bone->getObjectSphere();
					boneSphere.Transform(boneSphere, XMLoadFloat4x4(&bone->getWorldMatrix()));
					if (projectileOBB.Intersects(boneSphere)) {
						m_pResourceManager->getAnimationManagers()[i]->ChangeAnimation(0, true);
						m_pResourceManager->getAnimationManagers()[i]->IsCollision();
						projectile->setActive(false);
						projectile->setTime(0.0f);
					}
				}
			}
		}
	}
}

XMFLOAT3 CRaytracingScene::CalculateCollisionNormal(const BoundingOrientedBox& obb, const BoundingSphere& sphere)
{

	XMVECTOR sphereCenter = XMLoadFloat3(&sphere.Center);
	XMVECTOR obbCenter = XMLoadFloat3(&obb.Center);
	XMVECTOR orientation = XMLoadFloat4(&obb.Orientation);
	XMMATRIX rotation = XMMatrixRotationQuaternion(orientation);

	// OBB의 각 축 추출
	XMVECTOR axes[3] = {
		rotation.r[0],
		rotation.r[1],
		rotation.r[2]
	};

	// 가장 가까운 점 계산
	XMVECTOR closestPoint = obbCenter;
	XMVECTOR d = sphereCenter - obbCenter;

	for (int i = 0; i < 3; ++i) {
		float distance = XMVectorGetX(XMVector3Dot(d, axes[i]));
		float extent = (&obb.Extents.x)[i];
		distance = std::clamp(distance, -extent, extent);
		closestPoint += axes[i] * distance;
	}

	XMVECTOR normalVec = XMVector3Normalize(sphereCenter - closestPoint);
	XMFLOAT3 normal;
	XMStoreFloat3(&normal, normalVec);
	return normal;
}

float CRaytracingScene::CalculateDepth(const BoundingOrientedBox& obb, const BoundingSphere& sphere)
{
	XMVECTOR sphereCenter = XMLoadFloat3(&sphere.Center);
	XMVECTOR obbCenter = XMLoadFloat3(&obb.Center);
	XMVECTOR orientation = XMLoadFloat4(&obb.Orientation);
	XMMATRIX rotation = XMMatrixRotationQuaternion(orientation);

	XMVECTOR axes[3] = {
		rotation.r[0],
		rotation.r[1],
		rotation.r[2]
	};

	XMVECTOR closestPoint = obbCenter;
	XMVECTOR d = sphereCenter - obbCenter;

	for (int i = 0; i < 3; ++i) {
		float distance = XMVectorGetX(XMVector3Dot(d, axes[i]));
		float extent = (&obb.Extents.x)[i];
		distance = std::clamp(distance, -extent, extent);
		closestPoint += axes[i] * distance;
	}

	float dist = XMVectorGetX(XMVector3Length(sphereCenter - closestPoint));
	return sphere.Radius - dist;
}

void CRaytracingScene::CreateComputeShader()
{
	ID3DBlob* pBlob{};
	ID3DBlob* errorb{};
	D3DCompileFromFile(L"AnimationComputeShader.hlsl", nullptr, nullptr, "main", "cs_5_1", 0, 0, &pBlob, &errorb);
	if (errorb)
		OutputDebugStringA((char*)errorb->GetBufferPointer());


	//D3D12_CACHED_PIPELINE_STATE tempState{};
	D3D12_COMPUTE_PIPELINE_STATE_DESC desc{};
	desc.pRootSignature = m_pComputeRootSignature.Get();
	desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	desc.CS.BytecodeLength = pBlob->GetBufferSize();
	desc.CS.pShaderBytecode = pBlob->GetBufferPointer();

	g_DxResource.device->CreateComputePipelineState(&desc, IID_PPV_ARGS(m_pAnimationComputeShader.GetAddressOf()));

	pBlob->Release();
}

// =====================================================================================

void CRaytracingTestScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline)
{
	m_pOutputBuffer = outputBuffer;

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	m_pRaytracingPipeline = pipeline;

	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	m_pResourceManager->SetUp(3);								// LightBufferReady
	// Read File Here ========================================	! All Files Are Read Once !
	//m_pResourceManager->AddResourceFromFile(L"src\\model\\City.bin", "src\\texture\\City\\");
	//m_pResourceManager->AddResourceFromFile(L"src\\model\\WinterLand.bin", "src\\texture\\Map\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\swordman_lv1.bin", "src\\texture\\Luna\\");
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Luna_Firemantle_33.bin", "src\\texture\\Luna\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Feroptere.bin", "src\\texture\\Feroptere\\");

	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Xenokarce.bin", "src\\texture\\Xenokarce\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Lion.bin", "src\\texture\\Lion\\");
	m_pResourceManager->AddLightsFromFile(L"src\\Light\\Light_ETP.bin");
	m_pResourceManager->ReadyLightBufferContent();
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// Create new Objects, Copy SkinningObject here ========================================

	/*skinned.emplace_back(std::make_unique<CRayTracingSkinningObject>());
	skinned[2]->CopyFromOtherObject(skinned[1].get());
	aManagers.emplace_back(std::make_unique<CAnimationManager>(*aManagers[1].get()));
	aManagers[2]->SetFramesPointerFromSkinningObject(skinned[2]->getObjects());
	aManagers[2]->MakeAnimationMatrixIndex(skinned[2].get());*/


	UINT finalindex = normalObjects.size();
	UINT finalmesh = meshes.size();

	/*skinned[0]->getTextures().emplace_back(std::make_shared<CTexture>(L"src\\texture\\Swordman\\@Dex Studio_soullike_style.dds"));
	for (auto& p : skinned[0]->getObjects()) {
		auto& v = p->getMaterials();
		if (v.size()) {
			v[0].m_bHasAlbedoMap = true;
			v[0].m_nAlbedoMapIndex = 0;
		}
	}*/

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(300.0f, 50.0f, 1.0f), "box"));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[finalindex]->SetMeshIndex(finalmesh);
	normalObjects[finalindex]->getMaterials().emplace_back();
	normalObjects[finalindex]->getMaterials()[0].m_bHasAlbedoColor = true;
	normalObjects[finalindex]->getMaterials()[0].m_xmf4AlbedoColor = XMFLOAT4(0.0, 1.0, 0.0, 1.0);
	normalObjects[finalindex]->SetPosition(XMFLOAT3(0.0, 0.0, 20.0));

	// Copy Example
	//skinned.emplace_back(std::make_unique<CRayTracingSkinningObject>());
	//skinned[1]->CopyFromOtherObject(skinned[0].get());
	//aManagers.emplace_back(std::make_unique<CAnimationManager>(*aManagers[0].get()));
	//aManagers[1]->SetFramesPointerFromSkinningObject(skinned[1]->getObjects());
	//aManagers[1]->MakeAnimationMatrixIndex(skinned[1].get());
	//aManagers[1]->UpdateAnimation(0.5f);		// Not Need

	// Create new Object Example
	/*m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\asdf.raw", 2049, 2049, XMFLOAT3(1.0f, 0.03f, 1.0f));

	UINT finalindex = normalObjects.size();
	UINT finalmesh = meshes.size();
	Material tMaterial{};
	meshes.emplace_back(std::make_unique<Mesh>(m_pHeightMap.get(), "terrain"));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[finalindex]->SetMeshIndex(finalmesh);

	UINT txtIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\SnowGround00_Albedo.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\SnowGround00_NORM.dds"));

	tMaterial.m_bHasAlbedoMap = true; tMaterial.m_nAlbedoMapIndex = txtIndex;
	tMaterial.m_bHasNormalMap = true; tMaterial.m_nNormalMapIndex = txtIndex + 1;
	normalObjects[finalindex]->getMaterials().emplace_back(tMaterial);
	normalObjects[finalindex]->SetScale(XMFLOAT3(-1.0f, 1.0f, 1.0f));
	normalObjects[finalindex]->Rotate(XMFLOAT3(0.0f, 180.0f, 0.0f));
	normalObjects[finalindex]->SetPosition(XMFLOAT3(-1024.0, 0.0, 1024.0));*/

	/*UINT finalindex = normalObjects.size();
	UINT finalmesh = meshes.size();
	Material tMaterial{};
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), XMFLOAT3(5.0f, 20.0f, 5.0f)));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[finalindex]->SetMeshIndex(finalmesh);
	tMaterial.m_bHasAlbedoColor = true; tMaterial.m_xmf4AlbedoColor = XMFLOAT4(0.2, 0.4, 0.2, 1.0);
	tMaterial.m_bHasGlossyReflection = true; tMaterial.m_fGlossyReflection = 1.0f;
	normalObjects[finalindex]->getMaterials().emplace_back(tMaterial);
	normalObjects[finalindex]->SetPosition(XMFLOAT3(0.0, 30.0, 0.0)); */
	/*skinned[0]->getObjects()[98]->getMaterials()[0].m_bHasEmissionMap = true;
	skinned[0]->getObjects()[98]->getMaterials()[0].m_nEmissionMapIndex = skinned[0]->getTextures().size();
	skinned[0]->getTextures().emplace_back(std::make_shared<CTexture>(L"src\\texture\\Gorhorrid\\T_Gorhorrid_Emissive.dds"));*/

	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\ETPSky.dds", true));
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();	// CBV RAII
	m_pResourceManager->PrepareObject();	// Ready OutputBuffer to  SkinningObject


	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// Normal Object Copy & Manipulation Matrix Here ================================
	skinned[0]->SetPosition(XMFLOAT3(0.0f, 0.0f, 10.0f));
	skinned[0]->setPreTransform(2.0, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3());
	skinned[0]->Rotate(XMFLOAT3(0.0, 135.0, 0.0));
	// ==============================================================================

	m_pResourceManager->PrepareObject();

	// Setting Camera ==============================================================
	//m_pCamera->SetTarget(skinned[0]->getObjects()[0].get());
	m_pCamera->SetHOffset(3.5f);
	m_pCamera->SetCameraLength(15.0f);
	m_pCamera->SetMapNumber(SCENE_WINTERLAND);
	// ==========================================================================

	// AccelerationStructure
	m_pAccelerationStructureManager = std::make_unique<CAccelerationStructureManager>();
	m_pAccelerationStructureManager->Setup(m_pResourceManager.get(), 1);
	m_pAccelerationStructureManager->InitBLAS();
	m_pAccelerationStructureManager->InitTLAS();
}

void CRaytracingTestScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
		switch (wParam) {
		case '3':
			break;
		case '4':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(26);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '5':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(27);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '6':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(28);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '7':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(29);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '8':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(30);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case 'N':
			m_pCamera->toggleNormalMapping();
			break;
		case 'M':
			m_pCamera->toggleAlbedoColor();
			break;
		case 'B':
			m_pCamera->toggleReflection();
			break;
		case '9':
			m_pCamera->SetThirdPersonMode(false);
			break;
		case '0':
			m_pCamera->SetThirdPersonMode(true);
			break;
		}
		break;
	}
}

void CRaytracingTestScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_LBUTTONDOWN:
		m_bHold = true;
		GetCursorPos(&oldCursor);
		break;
	case WM_LBUTTONUP:
		m_bHold = false;
		break;
	case WM_MOUSEMOVE:
	{
		POINT cursorpos;
		if (m_bHold) {
			GetCursorPos(&cursorpos);
			m_pCamera->Rotate(cursorpos.x - oldCursor.x, cursorpos.y - oldCursor.y);
			SetCursorPos(oldCursor.x, oldCursor.y);
		}
		break;
	}
	}
}

void CRaytracingTestScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);
	bool shiftDown = false;
	if (keyBuffer[VK_SHIFT] & 0x80)
		shiftDown = true;
	if (keyBuffer['W'] & 0x80)
		m_pCamera->Move(0, fElapsedTime, shiftDown);
	if (keyBuffer['S'] & 0x80)
		m_pCamera->Move(5, fElapsedTime, shiftDown);
	if (keyBuffer['D'] & 0x80)
		m_pCamera->Move(3, fElapsedTime, shiftDown);
	if (keyBuffer['A'] & 0x80)
		m_pCamera->Move(4, fElapsedTime, shiftDown);
	if (keyBuffer[VK_SPACE] & 0x80)
		m_pCamera->Move(1, fElapsedTime, shiftDown);
	if (keyBuffer[VK_CONTROL] & 0x80)
		m_pCamera->Move(2, fElapsedTime, shiftDown);
}

void CRaytracingTestScene::UpdateObject(float fElapsedTime)
{
	// compute shader & rootSignature set
	g_DxResource.cmdList->SetPipelineState(m_pAnimationComputeShader.Get());
	g_DxResource.cmdList->SetComputeRootSignature(m_pComputeRootSignature.Get());

	m_pResourceManager->UpdateSkinningMesh(fElapsedTime);
	// particle update
	// SetRootSignature
	// particle update

	Flush();
	// Skinning Object BLAS ReBuild
	m_pResourceManager->ReBuildBLAS();

	m_pResourceManager->UpdateWorldMatrix();

	if (m_pCamera->getThirdPersonState()) {
		XMFLOAT3& EYE = m_pCamera->getEyeCalculateOffset();
		float cHeight = m_pHeightMap->GetHeightinWorldSpace(EYE.x + 1024.0f, EYE.z + 1024.0f);
		if (EYE.z >= -500.0f) {
			if (cHeight < 10.5f)
				cHeight = 10.5f;
		}
		if (EYE.y < cHeight + 0.5f) {
			m_pCamera->UpdateViewMatrix(cHeight + 0.5f);
		}
		else
			m_pCamera->UpdateViewMatrix();
	}
	else
		m_pCamera->UpdateViewMatrix();
	m_pAccelerationStructureManager->UpdateScene(m_pCamera->getEye());
}

// ==============================================================================

void CRaytracingMaterialTestScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer)
{
	// Create Global & Local Root Signature
	//CreateRootSignature();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	/*m_pRaytracingPipeline = std::make_unique<CRayTracingPipeline>();
	m_pRaytracingPipeline->Setup(1 + 2 + 1 + 2 + 1 + 1);
	m_pRaytracingPipeline->AddLibrarySubObject(compiledShader, std::size(compiledShader));
	m_pRaytracingPipeline->AddHitGroupSubObject(L"HitGroup", L"RadianceClosestHit", L"RadianceAnyHit");
	m_pRaytracingPipeline->AddHitGroupSubObject(L"ShadowHit", L"ShadowClosestHit", L"ShadowAnyHit");
	m_pRaytracingPipeline->AddShaderConfigSubObject(8, 20);
	m_pRaytracingPipeline->AddLocalRootAndAsoociationSubObject(m_pLocalRootSignature.Get());
	m_pRaytracingPipeline->AddGlobalRootSignatureSubObject(m_pGlobalRootSignature.Get());
	m_pRaytracingPipeline->AddPipelineConfigSubObject(6);
	m_pRaytracingPipeline->MakePipelineState();*/

	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	m_pResourceManager->SetUp(3);
	// Read File Here ========================================	! All Files Are Read Once !
	//m_pResourceManager->AddResourceFromFile(L"src\\model\\w.bin", "src\\texture\\Lion\\");
	m_pResourceManager->AddResourceFromFile(L"src\\model\\City.bin", "src\\texture\\City\\");
	//m_pResourceManager->AddResourceFromFile(L"src\\model\\WinterLand1.bin", "src\\texture\\Map\\");
	//m_pResourceManager->AddResourceFromFile(L"src\\model\\portal_low.bin", "src\\texture\\Map\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Lion.bin", "src\\texture\\Lion\\");
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Gorhorrid_tongue.bin", "src\\texture\\Gorhorrid\\");

	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Monster.bin", "src\\texture\\monster\\");
	// ���� �߰�
	m_pResourceManager->AddLightsFromFile(L"src\\Light\\LightingV2.bin");
	m_pResourceManager->ReadyLightBufferContent();
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// Create new Objects, Copy SkinningObject here  ========================================

	//UINT finalindex = normalObjects.size();
	//UINT finalmesh = meshes.size();
	//meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 10.0f));
	//normalObjects.emplace_back(std::make_unique<CGameObject>());
	//normalObjects[finalindex]->SetMeshIndex(finalmesh);
	//normalObjects[finalindex]->getMaterials().emplace_back();
	//Material& tMaterial = normalObjects[finalindex]->getMaterials()[0];
	//tMaterial.m_bHasAlbedoColor = true; tMaterial.m_xmf4AlbedoColor = XMFLOAT4(0.0, 1.0, 0.0, 0.5);
	//tMaterial.m_bHasSpecularColor = true; tMaterial.m_xmf4SpecularColor = XMFLOAT4(0.04, 0.04, 0.04, 1.0);
	////tMaterial.m_bHasMetallic = true; tMaterial.m_fMetallic = 0.0f;
	//tMaterial.m_bHasGlossiness = true; tMaterial.m_fGlossiness = 0.8;
	//tMaterial.m_bHasSpecularHighlight = true; tMaterial.m_fSpecularHighlight = 1;
	//tMaterial.m_bHasGlossyReflection = true; tMaterial.m_fGlossyReflection = 1;
	//normalObjects[finalindex]->SetInstanceID(1);
	//normalObjects[finalindex]->SetPosition(XMFLOAT3(0.0, 0.0, 0.0)); 

	/*meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), XMFLOAT3(1000.0f, 0.0, 1000.0f)));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[finalindex]->SetMeshIndex(finalmesh);
	normalObjects[finalindex]->getMaterials().emplace_back();
	Material& tt = normalObjects[finalindex]->getMaterials()[0];
	tt.m_bHasAlbedoColor = true; tt.m_xmf4AlbedoColor = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	tt.m_bHasSpecularColor = true; tt.m_xmf4SpecularColor = XMFLOAT4(0.04, 0.04, 0.04, 1.0);
	tt.m_bHasSpecularHighlight = true; tt.m_fSpecularHighlight = 1;
	tt.m_bHasGlossiness = true; tt.m_fGlossiness = 0.5;
	tt.m_bHasGlossyReflection = true; tt.m_fGlossyReflection = 1.0f;
	normalObjects[finalindex]->SetPosition(XMFLOAT3(0.0, 0.0, 0.0));*/

	//std::unique_ptr<CHeightMapImage> m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\terrain.raw", 2049, 2049, XMFLOAT3(1.0f, 0.0312f, 1.0f));
	////std::unique_ptr<CHeightMapImage> m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\Terrain_WinterLands_heightmap.raw", 4096, 4096, XMFLOAT3(1.0f, 0.025f, 1.0f));
	//UINT finalindex = normalObjects.size();
	//UINT finalmesh = meshes.size();
	//Material tMaterial{};
	//meshes.emplace_back(std::make_unique<Mesh>(m_pHeightMap.get(), "terrain"));
	//normalObjects.emplace_back(std::make_unique<CGameObject>());
	//normalObjects[finalindex]->SetMeshIndex(finalmesh);
	//normalObjects[finalindex]->SetInstanceID(10);

	//UINT txtIndex = textures.size();
	//textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\SnowGround00_Albedo.dds"));
	////textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\SnowGround00_NORM.dds"));

	//tMaterial.m_bHasAlbedoMap = true; tMaterial.m_nAlbedoMapIndex = txtIndex;
	////tMaterial.m_bHasNormalMap = true; tMaterial.m_nNormalMapIndex = txtIndex + 1;
	//tMaterial.m_bHasAlbedoColor = true; tMaterial.m_xmf4AlbedoColor = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	//tMaterial.m_bHasSpecularColor = true; tMaterial.m_xmf4SpecularColor = XMFLOAT4(0.04, 0.04, 0.04, 1.0);
	//tMaterial.m_bHasGlossiness = true; tMaterial.m_fGlossiness = 0.2;

	//normalObjects[finalindex]->getMaterials().emplace_back(tMaterial);
	//normalObjects[finalindex]->SetPosition(XMFLOAT3(-1024.0, 0.0, -1024.0));

	/*UINT finalindex = normalObjects.size();
	UINT finalmesh = meshes.size();

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 0.5f));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[finalindex]->SetMeshIndex(finalmesh);
	normalObjects[finalindex]->getMaterials().emplace_back();
	Material& dMaterial = normalObjects[finalindex]->getMaterials()[0];
	dMaterial.m_bHasAlbedoColor = true; dMaterial.m_xmf4AlbedoColor = XMFLOAT4(20.0, 0.0, 20.0, 1.0);
	dMaterial.m_bHasSpecularColor = true; dMaterial.m_xmf4SpecularColor = XMFLOAT4(0.04, 0.04, 0.04, 1.0);
	dMaterial.m_bHasGlossiness = true; dMaterial.m_fGlossiness = 0.1;

	normalObjects[finalindex]->SetPosition(XMFLOAT3(0.0, 20.0, 15.0));*/

	//meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), XMFLOAT3(50, 50, 2)));
	//normalObjects.emplace_back(std::make_unique<CGameObject>());
	//normalObjects[finalindex]->SetMeshIndex(finalmesh);
	//normalObjects[finalindex]->getMaterials().emplace_back();
	//Material& tMaterial = normalObjects[finalindex]->getMaterials()[0];
	//tMaterial.m_bHasAlbedoColor = true; tMaterial.m_xmf4AlbedoColor = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	//tMaterial.m_bHasSpecularColor = true; tMaterial.m_xmf4SpecularColor = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	////tMaterial.m_bHasMetallic = true; tMaterial.m_fMetallic = 0.0f;
	//tMaterial.m_bHasGlossiness = true; tMaterial.m_fGlossiness = 0.5;
	//tMaterial.m_bHasSpecularHighlight = true; tMaterial.m_fSpecularHighlight = 1;
	//tMaterial.m_bHasGlossyReflection = true; tMaterial.m_fGlossyReflection = 1;
	//normalObjects[finalindex]->SetPosition(XMFLOAT3(0.0, 0.0, -10.0)); 

	//meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), 0.5f));
	//normalObjects.emplace_back(std::make_unique<CGameObject>());
	//normalObjects[finalindex + 1]->SetMeshIndex(finalmesh + 1);
	//normalObjects[finalindex + 1]->getMaterials().emplace_back();
	//Material& dMaterial = normalObjects[finalindex + 1]->getMaterials()[0];
	//dMaterial.m_bHasAlbedoColor = true; dMaterial.m_xmf4AlbedoColor = XMFLOAT4(20.0, 0.0, 20.0, 1.0);
	//dMaterial.m_bHasSpecularColor = true; dMaterial.m_xmf4SpecularColor = XMFLOAT4(0.04, 0.04, 0.04, 1.0);
	//dMaterial.m_bHasGlossiness = true; dMaterial.m_fGlossiness = 0.1;

	//normalObjects[finalindex + 1]->SetPosition(XMFLOAT3(0.0, 20.0, 15.0));
	//std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();

	//textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02.dds"));
	//textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02_NORM.dds"));
	//textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02_MNS.dds"));
	//auto p = std::find_if(normalObjects.begin(), normalObjects.end(), [](std::unique_ptr<CGameObject>& p) {
	//	return p->getFrameName() == "Water";
	//	});
	//if (p != normalObjects.end()) {
	//	(*p)->SetInstanceID(1);
	//	(*p)->getMaterials().emplace_back();
	//	Material& mt = (*p)->getMaterials()[0];
	//	mt.m_bHasAlbedoColor = true; mt.m_xmf4AlbedoColor = XMFLOAT4(0.1613118, 0.2065666, 0.2358491, 0.2);
	//	//mt.m_bHasAlbedoColor = true; mt.m_xmf4AlbedoColor = XMFLOAT4(0.0, 0.0, 1.0, 0.7);
	//	//mt.m_bHasSpecularColor = true; mt.m_xmf4SpecularColor = XMFLOAT4(0.04, 0.04, 0.04, 1.0);
	//	mt.m_bHasMetallicMap = true; mt.m_nMetallicMapIndex = textures.size() - 1;
	//	//mt.m_bHasAlbedoMap = true; mt.m_nAlbedoMapIndex = textures.size() - 3;
	//	mt.m_bHasNormalMap = true; mt.m_nNormalMapIndex = textures.size() - 2;

	//	void* tempptr{};
	//	std::vector<XMFLOAT2> tex0 = meshes[(*p)->getMeshIndex()]->getTex0();
	//	for (XMFLOAT2& xmf : tex0) {
	//		xmf.x *= 10.0f; xmf.y *= 10.0f;
	//	}
	//	meshes[(*p)->getMeshIndex()]->getTexCoord0Buffer()->Map(0, nullptr, &tempptr);
	//	memcpy(tempptr, tex0.data(), sizeof(XMFLOAT2) * tex0.size());
	//	meshes[(*p)->getMeshIndex()]->getTexCoord0Buffer()->Unmap(0, nullptr);
	//}

	//PrepareTerrainTexture();

	// cubeMap Ready
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\WinterLandSky.dds", true));
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();	// CBV RAII
	m_pResourceManager->PrepareObject();	// Ready OutputBuffer to  SkinningObject


	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// Normal Object Copy & Manipulation Matrix Here  ===============================

	// ==============================================================================

	// Setting Camera ==============================================================
	//m_pCamera->SetTarget(normalObjects[0].get());
	//m_pCamera->SetCameraLength(20.0f);
	// ==========================================================================
	//m_pResourceManager->getAnimationManagers()[0]->UpdateAnimation(0.2);
	// AccelerationStructure
	m_pAccelerationStructureManager = std::make_unique<CAccelerationStructureManager>();
	m_pAccelerationStructureManager->Setup(m_pResourceManager.get(), 1);
	m_pAccelerationStructureManager->InitBLAS();
	m_pAccelerationStructureManager->InitTLAS();
}

void CRaytracingMaterialTestScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
		switch (wParam) {
		case '1':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(0);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '2':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(1);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '3':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(2);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '4':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(3);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '5':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(4);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '6':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(5);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '7':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(6);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '8':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(7);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case '9':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(8);
			m_pResourceManager->getAnimationManagers()[0]->setTimeZero();
			break;
		case 'n':
		case 'N':
			m_pCamera->toggleNormalMapping();
			break;
		case 'm':
		case 'M':
			m_pCamera->toggleAlbedoColor();
			break;
		}
		break;
	case WM_KEYUP:
		break;
	}
}

void CRaytracingMaterialTestScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);

	bool shiftDown = false;
	if (keyBuffer[VK_SHIFT] & 0x80)
		shiftDown = true;

	if (keyBuffer['W'] & 0x80)
		m_pCamera->Move(0, fElapsedTime, shiftDown);
	if (keyBuffer['S'] & 0x80)
		m_pCamera->Move(5, fElapsedTime, shiftDown);
	if (keyBuffer['D'] & 0x80)
		m_pCamera->Move(3, fElapsedTime, shiftDown);
	if (keyBuffer['A'] & 0x80)
		m_pCamera->Move(4, fElapsedTime, shiftDown);
	if (keyBuffer[VK_SPACE] & 0x80)
		m_pCamera->Move(1, fElapsedTime, shiftDown);
	if (keyBuffer[VK_CONTROL] & 0x80)
		m_pCamera->Move(2, fElapsedTime, shiftDown);

	if (keyBuffer['I'] & 0x80) {
		m_pResourceManager->getSkinningObjectList()[0]->move(fElapsedTime, 0);
	}

	if (keyBuffer['K'] & 0x80) {
		m_pResourceManager->getSkinningObjectList()[0]->move(fElapsedTime, 1);
	}

	if (keyBuffer['J'] & 0x80) {
		m_pResourceManager->getSkinningObjectList()[0]->Rotate(XMFLOAT3(0.0, -90.0f * fElapsedTime, 0.0f));
	}
	if (keyBuffer['L'] & 0x80) {
		m_pResourceManager->getSkinningObjectList()[0]->Rotate(XMFLOAT3(0.0, 90.0f * fElapsedTime, 0.0f));
	}

	if (keyBuffer['U'] & 0x80) {
		m_pResourceManager->getSkinningObjectList()[0]->move(fElapsedTime, 2);
	}

	if (keyBuffer['O'] & 0x80) {
		m_pResourceManager->getSkinningObjectList()[0]->move(fElapsedTime, 3);
	}

	if (keyBuffer[VK_RIGHT] & 0x80)
		m_pResourceManager->getAnimationManagers()[0]->TimeIncrease(fElapsedTime);
}

void CRaytracingMaterialTestScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	/*switch (nMessage) {
	case WM_LBUTTONDOWN:
		m_bHold = true;
		GetCursorPos(&oldCursor);
		break;
	case WM_LBUTTONUP:
		m_bHold = false;
		break;
	case WM_MOUSEMOVE:
	{
		POINT cursorpos;
		if (m_bHold) {
			GetCursorPos(&cursorpos);
			m_pCamera->Rotate(cursorpos.x - oldCursor.x, cursorpos.y - oldCursor.y);
			SetCursorPos(oldCursor.x, oldCursor.y);
		}
		break;
	}
	}*/
}

void CRaytracingMaterialTestScene::Render()
{
	m_pCamera->SetShaderVariable();
	m_pAccelerationStructureManager->SetScene();
	m_pResourceManager->SetLights();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	g_DxResource.cmdList->SetComputeRootDescriptorTable(4, textures[textures.size() - 1]->getView()->GetGPUDescriptorHandleForHeapStart());
	//g_DxResource.cmdList->SetComputeRootDescriptorTable(5, m_pTerrainDescriptor->GetGPUDescriptorHandleForHeapStart());

	D3D12_DISPATCH_RAYS_DESC raydesc{};
	raydesc.Depth = 1;
	raydesc.Width = DEFINED_UAV_BUFFER_WIDTH;
	raydesc.Height = DEFINED_UAV_BUFFER_HEIGHT;

	raydesc.RayGenerationShaderRecord.StartAddress = m_pShaderBindingTable->getRayGenTable()->GetGPUVirtualAddress();
	raydesc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	raydesc.MissShaderTable.StartAddress = m_pShaderBindingTable->getMissTable()->GetGPUVirtualAddress();
	raydesc.MissShaderTable.SizeInBytes = m_pShaderBindingTable->getMissSize();
	raydesc.MissShaderTable.StrideInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	raydesc.HitGroupTable.StartAddress = m_pShaderBindingTable->getHitGroupTable()->GetGPUVirtualAddress();
	raydesc.HitGroupTable.SizeInBytes = m_pShaderBindingTable->getHitGroupSize();
	raydesc.HitGroupTable.StrideInBytes = m_pShaderBindingTable->getHitGroupStride();

	g_DxResource.cmdList->DispatchRays(&raydesc);
}

// =====================================================================================


void CRaytracingWinterLandScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline)
{
	m_pOutputBuffer = outputBuffer;
	// CreateUISetup
	CreateOrthoMatrixBuffer();
	CreateRTVDSV();
	CreateUIRootSignature();
	CreateUIPipelineState();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	m_pRaytracingPipeline = pipeline;

	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	m_pResourceManager->SetUp(3);
	// Object File Read ========================================	! !
	m_pResourceManager->AddResourceFromFile(L"src\\model\\Map\\WinterLand\\WinterLand_Final.bin", "src\\texture\\Map\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\Greycloak\\", JOB_MAGE);
	CreateMageCharacter();
	m_pPlayer = std::make_unique<CPlayer>(m_vPlayers[m_vPlayers.size() - 1].get(), m_pCamera);

	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Gorhorrid.bin", "src\\texture\\Gorhorrid\\");
	// Light Read
	m_pResourceManager->AddLightsFromFile(L"src\\Light\\WinterLand_Light_Final.bin");
	m_pResourceManager->ReadyLightBufferContent();
	m_pResourceManager->WinterLand_LightSetup();
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// Create Normal Object & skinning Object Copy ========================================

	for (auto& o : skinned[1]->getObjects()) {
		for (auto& ma : o->getMaterials())
			ma.m_bHasEmissiveColor = false;
	}

	UINT finalindex = normalObjects.size();
	UINT finalmesh = meshes.size();

	// terrian
	m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\Map\\WinterLand\\Terrain_Final.raw", 2049, 2049, XMFLOAT3(1.0f, 0.0312f, 1.0f));
	meshes.emplace_back(std::make_unique<Mesh>(m_pHeightMap.get(), "terrain"));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[normalObjects.size() - 1]->SetMeshIndex(meshes.size() - 1);

	normalObjects[normalObjects.size() - 1]->SetInstanceID(10);
	normalObjects[normalObjects.size() - 1]->getMaterials().emplace_back();
	normalObjects[normalObjects.size() - 1]->SetPosition(XMFLOAT3(-1024.0, 0.0, -1024.0));
	m_pRoadTerrain = std::make_unique<CHeightMapImage>(L"src\\model\\Map\\WinterLand\\Terrain_Road.raw", 2049, 2049, XMFLOAT3(1.0f, 0.0312f, 1.0f));
	m_pCollisionHMap = std::make_unique<CHeightMapImage>(L"src\\model\\Map\\WinterLand\\Terrain_Collision.raw", 2049, 2049, XMFLOAT3(1.0f, 0.0312f, 1.0f));

	/*meshes.emplace_back(std::make_unique<Mesh>(m_pCollisionHMap.get(), "terrain2"));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[normalObjects.size() - 1]->SetMeshIndex(meshes.size() - 1);

	normalObjects[normalObjects.size() - 1]->getMaterials().emplace_back();
	normalObjects[normalObjects.size() - 1]->getMaterials()[0].m_bHasAlbedoColor = true;
	normalObjects[normalObjects.size() - 1]->getMaterials()[0].m_xmf4AlbedoColor = XMFLOAT4(0.5, 0.5, 0.5, 0.5);
	normalObjects[normalObjects.size() - 1]->SetPosition(XMFLOAT3(-1024.0, 0.0, -1024.0));*/


	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02_NORM.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02_MNS.dds"));
	auto p = std::find_if(normalObjects.begin(), normalObjects.end(), [](std::unique_ptr<CGameObject>& p) {
		return p->getFrameName() == "Water";
		});
	if (p != normalObjects.end()) {
		(*p)->SetInstanceID(2);
		(*p)->getMaterials().emplace_back();
		Material& mt = (*p)->getMaterials()[0];
		mt.m_bHasAlbedoColor = true; mt.m_xmf4AlbedoColor = XMFLOAT4(0.1613118, 0.2065666, 0.2358491, 0.2);
		mt.m_bHasMetallicMap = true; mt.m_nMetallicMapIndex = textures.size() - 1;
		mt.m_bHasNormalMap = true; mt.m_nNormalMapIndex = textures.size() - 2;

		void* tempptr{};
		std::vector<XMFLOAT2> tex0 = meshes[(*p)->getMeshIndex()]->getTex0();
		for (XMFLOAT2& xmf : tex0) {
			xmf.x *= 10.0f; xmf.y *= 10.0f;
		}
		meshes[(*p)->getMeshIndex()]->getTexCoord0Buffer()->Map(0, nullptr, &tempptr);
		memcpy(tempptr, tex0.data(), sizeof(XMFLOAT2) * tex0.size());
		meshes[(*p)->getMeshIndex()]->getTexCoord0Buffer()->Unmap(0, nullptr);
	}

	PrepareTerrainTexture();

	// cubeMap Ready
	m_nSkyboxIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\WinterLandSky2.dds", true));
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();
	m_pResourceManager->PrepareObject();	// Ready OutputBuffer to  SkinningObject


	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// Copy(normalObject) & SetPreMatrix ===============================

	skinned[0]->setPreTransform(2.5f, XMFLOAT3(), XMFLOAT3());
	//skinned[0]->SetPosition(XMFLOAT3(-72.5f, 0.0f, -998.0f));
	skinned[0]->SetPosition(XMFLOAT3(-72.5f, 0.0f, -500.0f));
	skinned[1]->setPreTransform(5.0f, XMFLOAT3(), XMFLOAT3());
	skinned[1]->SetPosition(XMFLOAT3(-28.0f, 0.0f, -245.0f));
	skinned[1]->Rotate(XMFLOAT3(0.0f, 180.0f, 0.0f));

	// ==============================================================================

	// Camera Setting ==============================================================
	m_pCamera->SetTarget(skinned[0]->getObjects()[0].get());
	m_pCamera->SetHOffset(3.5f);
	m_pCamera->SetCameraLength(15.0f);
	m_pCamera->SetMapNumber(SCENE_WINTERLAND);
	// ==========================================================================

	// AccelerationStructure
	m_pAccelerationStructureManager = std::make_unique<CAccelerationStructureManager>();
	m_pAccelerationStructureManager->Setup(m_pResourceManager.get(), 1);
	m_pAccelerationStructureManager->InitBLAS();
	m_pAccelerationStructureManager->InitTLAS();

	// UISetup ========================================================================
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT));
	m_vUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get()));
	m_vUIs[m_vUIs.size() - 1]->setPositionInViewport(0, 0);
	m_vUIs[m_vUIs.size() - 1]->setColor(0.0, 0.0, 0.0, 1.0);

	PlayerUISetup(JOB_MAGE);		// input player job
}

void CRaytracingWinterLandScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
		switch (wParam) {
		case 'N':
			m_pCamera->toggleNormalMapping();
			break;
		case 'M':
			m_pCamera->toggleAlbedoColor();
			break;
		case 'B':
			m_pCamera->toggleReflection();
			break;
		case '9':
			m_pCamera->SetThirdPersonMode(false);
			break;
		case '0':
			m_pCamera->SetThirdPersonMode(true);
			break;
		case '8':
			if (m_nState == IS_GAMING) {
				startTime = 0.0f;
				m_nState = IS_FINISH;
			}
			break;
		case 'U':
			cHPs[0] += 10;
			if (maxHPs[0] < cHPs[0])
				cHPs[0] = maxHPs[0];
			break;
		case 'J':
			cHPs[0] -= 10;
			if (0 > cHPs[0])
				cHPs[0] = 0;
			break;
		case '1':
			m_BuffState[0][0] = !m_BuffState[0][0];
			break;
		case '2':
			m_BuffState[0][1] = !m_BuffState[0][1];
			break;
		case '3':
			m_BuffState[0][2] = !m_BuffState[0][2];
			break;
		case '4':
			cMP = 100;
			break;
		case 'Z':
			m_vItemUIs[cItem]->setRenderState(false);
			--cItem;
			if (cItem < 0) cItem = 3;
			m_vItemUIs[cItem]->setRenderState(true);
			break;
		case 'C':
			m_vItemUIs[cItem]->setRenderState(false);
			++cItem;
			if (cItem > 3) cItem = 0;
			m_vItemUIs[cItem]->setRenderState(true);
			break;
		case 'X':
			if (itemNum[cItem] > 0 && m_nState == IS_GAMING) {
				--itemNum[cItem];
			}
			break;
		case 'Q':
			if (cMP >= 30 && curCTime[0] <= 0) {
				cMP -= 30;
				curCTime[0] = coolTime[0];
			}
			break;
		case 'E':
			if (cMP >= 40 && curCTime[1] <= 0) {
				cMP -= 40;
				curCTime[1] = coolTime[1];
			}
			break;
		case 'R':
			if (cMP >= 60 && curCTime[2] <= 0) {
				cMP -= 60;
				curCTime[2] = coolTime[2];
			}
			break;
		case 'P':
			if (m_nState == IS_GAMING) {
				m_bOpenShop = !m_bOpenShop;
				ShowCursor(m_bOpenShop);
				m_pShopUI->setRenderState(m_bOpenShop);
			}
			break;
		}
		break;
	case WM_KEYUP:
		break;
	}
}

void CRaytracingWinterLandScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	if (m_nState == IS_GAMING) {
		if (m_bOpenShop) {

		}
		else
			m_pPlayer->MouseProcess(hWnd, nMessage, wParam, lParam);
	}
}

void CRaytracingWinterLandScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);

	if (m_nState == IS_GAMING) {
		if (!m_pCamera->getThirdPersonState()) {
			bool shiftDown = false;
			if (keyBuffer[VK_SHIFT] & 0x80)
				shiftDown = true;
			if (keyBuffer['W'] & 0x80)
				m_pCamera->Move(0, fElapsedTime, shiftDown);
			if (keyBuffer['S'] & 0x80)
				m_pCamera->Move(5, fElapsedTime, shiftDown);
			if (keyBuffer['D'] & 0x80)
				m_pCamera->Move(3, fElapsedTime, shiftDown);
			if (keyBuffer['A'] & 0x80)
				m_pCamera->Move(4, fElapsedTime, shiftDown);
			if (keyBuffer[VK_SPACE] & 0x80)
				m_pCamera->Move(1, fElapsedTime, shiftDown);
			if (keyBuffer[VK_CONTROL] & 0x80)
				m_pCamera->Move(2, fElapsedTime, shiftDown);
		}
		else {
			m_pPlayer->ProcessInput(keyBuffer, fElapsedTime);
		}
	}
}

void CRaytracingWinterLandScene::CreateUIRootSignature()
{
	D3D12_DESCRIPTOR_RANGE tRange{};
	tRange.BaseShaderRegister = 0;
	tRange.NumDescriptors = 1;
	tRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	D3D12_ROOT_PARAMETER params[3]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[0].Descriptor.RegisterSpace = 0;
	params[0].Descriptor.ShaderRegister = 0;

	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[1].Descriptor.RegisterSpace = 0;
	params[1].Descriptor.ShaderRegister = 1;

	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 1;
	params[2].DescriptorTable.pDescriptorRanges = &tRange;

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};								// s0
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rtDesc{};
	rtDesc.NumParameters = 3;
	rtDesc.NumStaticSamplers = 1;
	rtDesc.pParameters = params;
	rtDesc.pStaticSamplers = &samplerDesc;
	rtDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* pBlob{};
	D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
	g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_UIRootSignature.GetAddressOf()));
	pBlob->Release();
}
void CRaytracingWinterLandScene::CreateUIPipelineState()
{
	ID3DBlob* pd3dVBlob{ nullptr };
	ID3DBlob* pd3dPBlob{ nullptr };
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineState{};
	d3dPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineState.pRootSignature = m_UIRootSignature.Get();

	D3D12_INPUT_ELEMENT_DESC ldesc[3]{};
	ldesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3dPipelineState.InputLayout.pInputElementDescs = ldesc;
	d3dPipelineState.InputLayout.NumElements = 3;

	d3dPipelineState.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dPipelineState.DepthStencilState.StencilEnable = FALSE;

	d3dPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	d3dPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	d3dPipelineState.RasterizerState.AntialiasedLineEnable = FALSE;
	d3dPipelineState.RasterizerState.FrontCounterClockwise = FALSE;
	d3dPipelineState.RasterizerState.MultisampleEnable = FALSE;
	d3dPipelineState.RasterizerState.DepthClipEnable = FALSE;

	d3dPipelineState.BlendState.AlphaToCoverageEnable = FALSE;
	d3dPipelineState.BlendState.IndependentBlendEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendEnable = TRUE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dPipelineState.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	d3dPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineState.NumRenderTargets = 1;
	d3dPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineState.SampleDesc.Count = 1;
	d3dPipelineState.SampleMask = UINT_MAX;

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &pd3dVBlob, nullptr);
	d3dPipelineState.VS.BytecodeLength = pd3dVBlob->GetBufferSize();
	d3dPipelineState.VS.pShaderBytecode = pd3dVBlob->GetBufferPointer();

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", 0, 0, &pd3dPBlob, nullptr);
	d3dPipelineState.PS.BytecodeLength = pd3dPBlob->GetBufferSize();
	d3dPipelineState.PS.pShaderBytecode = pd3dPBlob->GetBufferPointer();

	g_DxResource.device->CreateGraphicsPipelineState(&d3dPipelineState, IID_PPV_ARGS(m_UIPipelineState.GetAddressOf()));

	if (pd3dVBlob)
		pd3dVBlob->Release();
	if (pd3dPBlob)
		pd3dPBlob->Release();
}

void CRaytracingWinterLandScene::CreateMageCharacter()
{
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\Greycloak\\", JOB_MAGE);
	m_vPlayers.emplace_back(std::make_unique<CPlayerMage>(
		m_pResourceManager->getSkinningObjectList()[m_pResourceManager->getSkinningObjectList().size() - 1].get(),
		m_pResourceManager->getAnimationManagers()[m_pResourceManager->getAnimationManagers().size() - 1].get(), false));

	// Create Mage's own objects and Set
	// ex) bullet, particle, barrier  etc...
}

void CRaytracingWinterLandScene::PrepareTerrainTexture()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 14;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	g_DxResource.device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pTerrainDescriptor.GetAddressOf()));

	struct alignas(16) terrainINFO {
		int numLayer{};
		float padding[3]{};
		int bHasDiffuse[4]{};
		int bHasNormal[4]{};
		int bHasMask[4]{};
	};

	auto rdesc = BASIC_BUFFER_DESC;
	rdesc.Width = Align(sizeof(terrainINFO), 256);

	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &rdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(m_pTerrainCB.GetAddressOf()));

	terrainINFO* pMap{};
	m_pTerrainCB->Map(0, nullptr, reinterpret_cast<void**>(&pMap));
	pMap->numLayer = 4;
	pMap->bHasDiffuse[0] = pMap->bHasDiffuse[1] = pMap->bHasDiffuse[2] = pMap->bHasDiffuse[3] = 1;
	pMap->bHasNormal[0] = pMap->bHasNormal[1] = pMap->bHasNormal[2] = pMap->bHasNormal[3] = 0;
	pMap->bHasMask[0] = pMap->bHasMask[2] = pMap->bHasMask[3] = 0;
	m_pTerrainCB->Unmap(0, nullptr);

	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	size_t textureIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Terrain_WinterLands_splatmap.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\RockStalagmites00_terrain2.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\Stonerock03_Metallic.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\SnowGround00_Albedo.dds"));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	D3D12_RESOURCE_DESC d3dRD;

	UINT increment = g_DxResource.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_CPU_DESCRIPTOR_HANDLE  handle = m_pTerrainDescriptor->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cdesc{};
	cdesc.BufferLocation = m_pTerrainCB->GetGPUVirtualAddress();
	cdesc.SizeInBytes = rdesc.Width;

	g_DxResource.device->CreateConstantBufferView(&cdesc, handle);
	handle.ptr += increment;

	// splat
	d3dRD = textures[textureIndex]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;

	g_DxResource.device->CreateShaderResourceView(textures[textureIndex]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 0 ===============================================================

	d3dRD = textures[textureIndex + 1]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 1]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 1 ===============================================================

	d3dRD = textures[textureIndex + 2]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 2]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 2 ===============================================================

	d3dRD = textures[textureIndex + 3]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 3]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 3 ===============================================================

	d3dRD = textures[textureIndex + 4]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 4]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;
}

void CRaytracingWinterLandScene::UpdateObject(float fElapsedTime)
{
	// compute shader & rootSignature set
	g_DxResource.cmdList->SetPipelineState(m_pAnimationComputeShader.Get());
	g_DxResource.cmdList->SetComputeRootSignature(m_pComputeRootSignature.Get());

	m_pResourceManager->UpdateSkinningMesh(fElapsedTime);
	// particle update
	// SetRootSignature
	// particle update

	Flush();
	// Skinning Object BLAS ReBuild
	m_pResourceManager->ReBuildBLAS();

	m_pResourceManager->UpdateWorldMatrix();

	for (auto& p : m_vPlayers)
		p->UpdateObject(fElapsedTime);

	m_pPlayer->CollisionCheck(m_pRoadTerrain.get(), m_pCollisionHMap.get(), fElapsedTime, -1024.0f, 0.0f, -1024.0f, SCENE_WINTERLAND);
	m_pPlayer->HeightCheck(m_pRoadTerrain.get(), fElapsedTime, -1024.0f, 0.0f, -1024.0f, SCENE_WINTERLAND);
	
	if (m_pCamera->getThirdPersonState()) {
		XMFLOAT3& EYE = m_pCamera->getEyeCalculateOffset();
		float cHeight = m_pRoadTerrain->GetHeightinWorldSpace(EYE.x + 1024.0f, EYE.z + 1024.0f);
		if (EYE.z >= -500.0f) {
			if (cHeight < 10.5f)
				cHeight = 10.5f;
		}
		if (EYE.y < cHeight + 0.5f) {
			m_pCamera->UpdateViewMatrix(cHeight + 0.5f);
		}
		else
			m_pCamera->UpdateViewMatrix();
	}
	else
		m_pCamera->UpdateViewMatrix();
	m_pAccelerationStructureManager->UpdateScene(m_pCamera->getEye());

	switch (m_nState) {
	case IS_LOADING: {
			wOpacity -= 0.5 * fElapsedTime;
			if (wOpacity < 0.0f) {
				m_nState = IS_GAMING;
				wOpacity = 0.0f;
			}
			m_vUIs[0]->setColor(0.0, 0.0, 0.0, wOpacity);
		break;
	}
	case IS_GAMING: {
		break;
	}
	case IS_FINISH: {
			wOpacity += 0.2 * fElapsedTime;
			if (wOpacity > 1.0f) {
				ShowCursor(TRUE);
				m_nNextScene = SCENE_TITLE;
				wOpacity = 1.0f;
			}
			m_vUIs[0]->setColor(0.0, 0.0, 0.0, wOpacity);
		break;
	}
	}

	// Player UI ==================================================
	int buffstart = 20; int bstride = 40;
	for (int i = 0; i < m_numUser; ++i) {
		int t{};
		// hp/mp
		m_vStatusUIs[i][1]->setScaleX(cHPs[i] / maxHPs[i]);
		if (i == 0) {
			m_vStatusUIs[i][3]->setScaleXWithUV(cMP / maxMP);
		}
		if (i > 0) {
			buffstart = 15; bstride = 30;
		}
		for (int j = 0; j < 3; ++j) {
			if (m_BuffState[i][j]) {
				m_vStatusUIs[i][j + 4]->setRenderState(true);
				m_vStatusUIs[i][j + 4]->setPositionInViewport(buffstart + (t * bstride), m_buffpixelHeight[i]);
				++t;
			}
			else {
				m_vStatusUIs[i][j + 4]->setRenderState(false);
			}
		}
	}

	{
		if (cMP < 30)
			m_vSkillUIs[6]->setRenderState(true);
		else
			m_vSkillUIs[6]->setRenderState(false);

		if (cMP < 40)
			m_vSkillUIs[7]->setRenderState(true);
		else
			m_vSkillUIs[7]->setRenderState(false);

		if (cMP < 60)
			m_vSkillUIs[8]->setRenderState(true);
		else
			m_vSkillUIs[8]->setRenderState(false);

		for (int i = 0; i < 3; ++i) {
			curCTime[i] -= fElapsedTime;
			if (curCTime[i] < 0) curCTime[i] = 0.0f;
			m_vSkillUIs[i + 3]->setScaleY(curCTime[i] / coolTime[i]);
		}
	}
	// =================================================================

}

void CRaytracingWinterLandScene::Render()
{
	m_pCamera->SetShaderVariable();
	m_pAccelerationStructureManager->SetScene();
	m_pResourceManager->SetLights();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	g_DxResource.cmdList->SetComputeRootDescriptorTable(4, textures[m_nSkyboxIndex]->getView()->GetGPUDescriptorHandleForHeapStart());
	g_DxResource.cmdList->SetComputeRootDescriptorTable(5, m_pTerrainDescriptor->GetGPUDescriptorHandleForHeapStart());

	D3D12_DISPATCH_RAYS_DESC raydesc{};
	raydesc.Depth = 1;
	raydesc.Width = DEFINED_UAV_BUFFER_WIDTH;
	raydesc.Height = DEFINED_UAV_BUFFER_HEIGHT;

	raydesc.RayGenerationShaderRecord.StartAddress = m_pShaderBindingTable->getRayGenTable()->GetGPUVirtualAddress();
	raydesc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	raydesc.MissShaderTable.StartAddress = m_pShaderBindingTable->getMissTable()->GetGPUVirtualAddress();
	raydesc.MissShaderTable.SizeInBytes = m_pShaderBindingTable->getMissSize();
	raydesc.MissShaderTable.StrideInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	raydesc.HitGroupTable.StartAddress = m_pShaderBindingTable->getHitGroupTable()->GetGPUVirtualAddress();
	raydesc.HitGroupTable.SizeInBytes = m_pShaderBindingTable->getHitGroupSize();
	raydesc.HitGroupTable.StrideInBytes = m_pShaderBindingTable->getHitGroupStride();

	g_DxResource.cmdList->DispatchRays(&raydesc);

	// UI Render ==================================================================================

	ID3D12GraphicsCommandList4* cmdList = g_DxResource.cmdList;
	auto barrier = [&](ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		{
			D3D12_RESOURCE_BARRIER resBarrier{};
			resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resBarrier.Transition.pResource = pResource;
			resBarrier.Transition.StateBefore = before;
			resBarrier.Transition.StateAfter = after;
			resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmdList->ResourceBarrier(1, &resBarrier);
		};

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_VIEWPORT vv{};
	vv.Width = DEFINED_UAV_BUFFER_WIDTH; vv.Height = DEFINED_UAV_BUFFER_HEIGHT; vv.MinDepth = 0.0f; vv.MaxDepth = 1.0f;
	cmdList->RSSetViewports(1, &vv);
	D3D12_RECT ss{ 0, 0, DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT };
	cmdList->RSSetScissorRects(1, &ss);
	cmdList->OMSetRenderTargets(1, &m_RTV->GetCPUDescriptorHandleForHeapStart(), FALSE, &m_DSV->GetCPUDescriptorHandleForHeapStart());
	cmdList->SetGraphicsRootSignature(m_UIRootSignature.Get());
	cmdList->SetPipelineState(m_UIPipelineState.Get());
	cmdList->SetGraphicsRootConstantBufferView(0, m_cameraCB->GetGPUVirtualAddress());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// player UI ====================================
	for (short i = 0; i < m_numUser; ++i) {
		for (auto& p : m_vStatusUIs[i])
			p->Render();
	}
	for (auto& p : m_vItemUIs)
		p->Render();

	for (auto& p : m_vSkillUIs)
		p->Render();

	m_pShopUI->Render();
	// ===============================================


	for (auto& p : m_vUIs)	// black plane
		p->Render();

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void CRaytracingWinterLandScene::PlayerUISetup(short job)
{
	size_t mindex{};
	size_t tindex{};
	size_t uindex{};

	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();

	// status UI ===================================================================
	maxHPs[0] = 1200; maxHPs[1] = 1000; maxHPs[2] = 800;
	cHPs[0] = 1200; cHPs[1] = 800; cHPs[2] = 730;

	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 30, 30));		// buff icon
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 340, 28));		// hp/mp bar
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 330, 18));		// hp/mp

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 255, 12));		// coop hp/mp

	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_HPbar.dds"));	// HPbar
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MPbar.dds"));	// MPbar
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MP.dds"));	// MP

	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Buff0.dds"));	// buff0
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Buff1.dds"));	// buff1
	{
		uindex = m_vStatusUIs[0].size();			// 0 - hpbar
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 20);

		uindex = m_vStatusUIs[0].size();			// 1 - hp
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 2].get()));
		m_vStatusUIs[0][uindex]->setColor(1.0, 0.0, 0.0, 1.0);
		m_vStatusUIs[0][uindex]->setPositionInViewport(25, 25);

		uindex = m_vStatusUIs[0].size();			// 2 - mp bar
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex + 1].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 60);

		uindex = m_vStatusUIs[0].size();			// 2 - mp
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 2].get(), textures[tindex + 2].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(25, 65);

		m_buffpixelHeight[0] = 100;
		uindex = m_vStatusUIs[0].size();			// 3 ~ 5 buff
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 3].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 100);
		uindex = m_vStatusUIs[0].size();			// 3 ~ 5 buff
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 4].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 100);
		uindex = m_vStatusUIs[0].size();			// 3 ~ 5 buff
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 100);
		m_vStatusUIs[0][uindex]->setColor(0.7, 1.0, 0.0, 1.0);
	}

	//for (int i = 0; i < 2; ++i) {
	//	uindex = m_vStatusUIs[i + 1].size();			// 1 - hp
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get()));
	//	m_vStatusUIs[i + 1][uindex]->setPositionInViewport(15, (i * 115) + 150 + 15);
	//	m_vStatusUIs[i + 1][uindex]->setColor(1.0, 0.0, 0.0, 1.0);

	//	uindex = m_vStatusUIs[i + 1].size();			// 2 - mp
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get()));
	//	m_vStatusUIs[i + 1][uindex]->setPositionInViewport(15, (i * 115) + 150 + 15 + 30);
	//	m_vStatusUIs[i + 1][uindex]->setColor(0.0, 0.0, 1.0, 1.0);

	//	m_buffpixelHeight[i + 1] = (i * 115) + 150 + 15 + 30 + 30;
	//	uindex = m_vStatusUIs[i + 1].size();			// 3 ~ 5 buff
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	//	m_vStatusUIs[i + 1][uindex]->setScale(0.75);
	//	m_vStatusUIs[i + 1][uindex]->setColor(0.0, 1.0, 1.0, 1.0);
	//	uindex = m_vStatusUIs[i + 1].size();			// 3 ~ 5 buff
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	//	m_vStatusUIs[i + 1][uindex]->setScale(0.75);
	//	m_vStatusUIs[i + 1][uindex]->setColor(1.0, 0.5, 1.0, 1.0);
	//	uindex = m_vStatusUIs[i + 1].size();			// 3 ~ 5 buff
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	//	m_vStatusUIs[i + 1][uindex]->setScale(0.75);
	//	m_vStatusUIs[i + 1][uindex]->setColor(0.7, 1.0, 0.0, 1.0);
	//}
	// =============================================================================

	// item ========================================================================
	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 140, 175));

	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item0.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item1.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item2.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item3.dds"));

	for (int i = 0; i < 4; ++i) {
		uindex = m_vItemUIs.size();
		m_vItemUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + i].get()));
		m_vItemUIs[uindex]->setColor(0.2 * (i + 1), 0.3, 0.2 * (i + 1), 1.0);
		m_vItemUIs[uindex]->setPositionInViewport(20, 525);
		m_vItemUIs[uindex]->setRenderState(false);
	}
	m_vItemUIs[0]->setRenderState(true);

	// =============================================================================

	// skills ======================================================================

	coolTime[0] = 5.0f; coolTime[1] = 10.0f; coolTime[2] = 20.0f;

	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 100, 100));

	tindex = textures.size();
	switch (job) {
	case JOB_MAGE:
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician0.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician1.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician2.dds"));
		break;
	}
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_MP_Less.dds"));
	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + i].get()));
		//m_vSkillUIs[uindex]->setColor(1.0, 0.5, 0.5, 1.0);
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
		m_vSkillUIs[uindex]->setColor(0.0, 0.0, 0.0, 0.5);
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 3].get()));
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	// Shop ============================================================================
	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 400, 500));
	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Shop.dds"));
	m_pShopUI = std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex].get());
	m_pShopUI->setPositionInViewport(780, 50);
	m_pShopUI->setRenderState(false);
}

// ==========================================================================

void CRaytracingCaveScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline)
{
	m_pOutputBuffer = outputBuffer;
	// CreateUISetup
	CreateOrthoMatrixBuffer();
	CreateRTVDSV();
	CreateUIRootSignature();
	CreateUIPipelineState();

	// Create Global & Local Root Signature
	//CreateRootSignature();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	/*m_pRaytracingPipeline = std::make_unique<CRayTracingPipeline>();
	m_pRaytracingPipeline->Setup(1 + 2 + 1 + 2 + 1 + 1);
	m_pRaytracingPipeline->AddLibrarySubObject(compiledShader, std::size(compiledShader));
	m_pRaytracingPipeline->AddHitGroupSubObject(L"HitGroup", L"RadianceClosestHit", L"RadianceAnyHit");
	m_pRaytracingPipeline->AddHitGroupSubObject(L"ShadowHit", L"ShadowClosestHit", L"ShadowAnyHit");
	m_pRaytracingPipeline->AddShaderConfigSubObject(8, 20);
	m_pRaytracingPipeline->AddLocalRootAndAsoociationSubObject(m_pLocalRootSignature.Get());
	m_pRaytracingPipeline->AddGlobalRootSignatureSubObject(m_pGlobalRootSignature.Get());
	m_pRaytracingPipeline->AddPipelineConfigSubObject(6);
	m_pRaytracingPipeline->MakePipelineState();*/
	m_pRaytracingPipeline = pipeline;
	
	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	m_pResourceManager->SetUp(3);
	// Object File Read ========================================	! !
	m_pResourceManager->AddResourceFromFile(L"src\\model\\Map\\Cave\\Cave.bin", "src\\texture\\Map\\");

	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\Greycloak\\", JOB_MAGE);
	CreateMageCharacter();
	m_pPlayer = std::make_unique<CPlayer>(m_vPlayers[m_vPlayers.size() - 1].get(), m_pCamera);

	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Gorhorrid.bin", "src\\texture\\Gorhorrid\\");
	// Light Read
	m_pResourceManager->AddLightsFromFile(L"src\\Light\\Light_Cave.bin");
	m_pResourceManager->ReadyLightBufferContent();
	//m_pResourceManager->LightTest();
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// Create Normal Object & skinning Object Copy ========================================

	/*for (auto& o : skinned[1]->getObjects()) {
		for (auto& ma : o->getMaterials())
			ma.m_bHasEmissiveColor = false;
	}*/

	UINT finalindex = normalObjects.size();
	UINT finalmesh = meshes.size();

	// terrian
	m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\Map\\Cave\\CaveHeightMap.raw", 512, 512, XMFLOAT3(500.0f / 512.0f, 0.0092f, 500.0f / 512.0f));
	m_pCollisionHMap = std::make_unique<CHeightMapImage>(L"src\\model\\Map\\Cave\\CaveCollisionHMap.raw", 512, 512, XMFLOAT3(500.0f / 512.0f, 1.0f, 500.0f / 512.0f));
	/*meshes.emplace_back(std::make_unique<Mesh>(m_pCollisionHMap.get(), "terrain"));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[normalObjects.size() - 1]->SetMeshIndex(meshes.size() - 1);

	normalObjects[normalObjects.size() - 1]->getMaterials().emplace_back();
	normalObjects[normalObjects.size() - 1]->getMaterials()[0].m_bHasAlbedoColor = true;
	normalObjects[normalObjects.size() - 1]->getMaterials()[0].m_xmf4AlbedoColor = XMFLOAT4(0.5, 0.5, 0.5, 1.0);
	normalObjects[normalObjects.size() - 1]->SetPosition(XMFLOAT3(-200.0, -10.0, -66.5));*/

	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\dlnk_Water_01_nrm.dds"));
	std::for_each(normalObjects.begin(), normalObjects.end(), [&](std::unique_ptr<CGameObject>& p) {
		if (p->getFrameName().contains("Plane")) {
			p->SetInstanceID(1);
			p->getMaterials().emplace_back();
			Material& mt = p->getMaterials()[0];
			mt.m_bHasAlbedoColor = true; mt.m_xmf4AlbedoColor = XMFLOAT4(0.1613118, 0.2065666, 0.2358491, 0.2);
			mt.m_bHasNormalMap = true; mt.m_nNormalMapIndex = textures.size() - 1;

			void* tempptr{};
			std::vector<XMFLOAT2> tex0 = meshes[p->getMeshIndex()]->getTex0();
			for (XMFLOAT2& xmf : tex0) {
				xmf.x *= 10.0f; xmf.y *= 10.0f;
			}
			meshes[p->getMeshIndex()]->getTexCoord0Buffer()->Map(0, nullptr, &tempptr);
			memcpy(tempptr, tex0.data(), sizeof(XMFLOAT2) * tex0.size());
			meshes[p->getMeshIndex()]->getTexCoord0Buffer()->Unmap(0, nullptr);
		}
		});


	// cubeMap Ready
	m_nSkyboxIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\WinterLandSky2.dds", true));
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();
	m_pResourceManager->PrepareObject();	// Ready OutputBuffer to  SkinningObject


	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// Copy(normalObject) & SetPreMatrix ===============================

	skinned[0]->setPreTransform(2.5f, XMFLOAT3(), XMFLOAT3());
	skinned[0]->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));

	// ==============================================================================

	// Camera Setting ==============================================================
	m_pCamera->SetTarget(skinned[0]->getObjects()[0].get());
	m_pCamera->SetHOffset(3.5f);
	m_pCamera->SetCameraLength(15.0f);
	m_pCamera->SetMapNumber(SCENE_CAVE);
	// ==========================================================================

	// AccelerationStructure
	m_pAccelerationStructureManager = std::make_unique<CAccelerationStructureManager>();
	m_pAccelerationStructureManager->Setup(m_pResourceManager.get(), 1);
	m_pAccelerationStructureManager->InitBLAS();
	m_pAccelerationStructureManager->InitTLAS();

	// UISetup ========================================================================
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT));
	m_vUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get()));
	m_vUIs[m_vUIs.size() - 1]->setPositionInViewport(0, 0);
	m_vUIs[m_vUIs.size() - 1]->setColor(0.0, 0.0, 0.0, 1.0);

	PlayerUISetup(JOB_MAGE);
}

void CRaytracingCaveScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
		switch (wParam) {
		case 'N':
			m_pCamera->toggleNormalMapping();
			break;
		case 'M':
			m_pCamera->toggleAlbedoColor();
			break;
		case 'B':
			m_pCamera->toggleReflection();
			break;
		case '9':
			m_pCamera->SetThirdPersonMode(false);
			break;
		case '0':
			m_pCamera->SetThirdPersonMode(true);
			break;
		case '8':
			if (m_nState == IS_GAMING) {
				startTime = 0.0f;
				m_nState = IS_FINISH;
			}
			break;
		case 'U':
			cHPs[0] += 10;
			if (maxHPs[0] < cHPs[0])
				cHPs[0] = maxHPs[0];
			break;
		case 'J':
			cHPs[0] -= 10;
			if (0 > cHPs[0])
				cHPs[0] = 0;
			break;
		case '1':
			m_BuffState[0][0] = !m_BuffState[0][0];
			break;
		case '2':
			m_BuffState[0][1] = !m_BuffState[0][1];
			break;
		case '3':
			m_BuffState[0][2] = !m_BuffState[0][2];
			break;
		case '4':
			cMP = 100;
			break;
		case 'Z':
			m_vItemUIs[cItem]->setRenderState(false);
			--cItem;
			if (cItem < 0) cItem = 3;
			m_vItemUIs[cItem]->setRenderState(true);
			break;
		case 'C':
			m_vItemUIs[cItem]->setRenderState(false);
			++cItem;
			if (cItem > 3) cItem = 0;
			m_vItemUIs[cItem]->setRenderState(true);
			break;
		case 'X':
			if (itemNum[cItem] > 0 && m_nState == IS_GAMING) {
				--itemNum[cItem];
			}
			break;
		case 'Q':
			if (cMP >= 30 && curCTime[0] <= 0) {
				cMP -= 30;
				curCTime[0] = coolTime[0];
			}
			break;
		case 'E':
			if (cMP >= 40 && curCTime[1] <= 0) {
				cMP -= 40;
				curCTime[1] = coolTime[1];
			}
			break;
		case 'R':
			if (cMP >= 60 && curCTime[2] <= 0) {
				cMP -= 60;
				curCTime[2] = coolTime[2];
			}
			break;
		case 'P':
			if (m_nState == IS_GAMING) {
				m_bOpenShop = !m_bOpenShop;
				ShowCursor(m_bOpenShop);
				m_pShopUI->setRenderState(m_bOpenShop);
			}
			break;
		}
		break;
	case WM_KEYUP:
		break;
	}
}

void CRaytracingCaveScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	m_pPlayer->MouseProcess(hWnd, nMessage, wParam, lParam);
}

void CRaytracingCaveScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);

	if (m_nState == IS_GAMING) {
		if (!m_pCamera->getThirdPersonState()) {
			bool shiftDown = false;
			if (keyBuffer[VK_SHIFT] & 0x80)
				shiftDown = true;
			if (keyBuffer['W'] & 0x80)
				m_pCamera->Move(0, fElapsedTime, shiftDown);
			if (keyBuffer['S'] & 0x80)
				m_pCamera->Move(5, fElapsedTime, shiftDown);
			if (keyBuffer['D'] & 0x80)
				m_pCamera->Move(3, fElapsedTime, shiftDown);
			if (keyBuffer['A'] & 0x80)
				m_pCamera->Move(4, fElapsedTime, shiftDown);
			if (keyBuffer[VK_SPACE] & 0x80)
				m_pCamera->Move(1, fElapsedTime, shiftDown);
			if (keyBuffer[VK_CONTROL] & 0x80)
				m_pCamera->Move(2, fElapsedTime, shiftDown);
		}
		else {
			m_pPlayer->ProcessInput(keyBuffer, fElapsedTime);
		}
	}
}

void CRaytracingCaveScene::CreateUIRootSignature()
{
	D3D12_DESCRIPTOR_RANGE tRange{};
	tRange.BaseShaderRegister = 0;
	tRange.NumDescriptors = 1;
	tRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	D3D12_ROOT_PARAMETER params[3]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[0].Descriptor.RegisterSpace = 0;
	params[0].Descriptor.ShaderRegister = 0;

	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[1].Descriptor.RegisterSpace = 0;
	params[1].Descriptor.ShaderRegister = 1;

	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 1;
	params[2].DescriptorTable.pDescriptorRanges = &tRange;

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};								// s0
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rtDesc{};
	rtDesc.NumParameters = 3;
	rtDesc.NumStaticSamplers = 1;
	rtDesc.pParameters = params;
	rtDesc.pStaticSamplers = &samplerDesc;
	rtDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* pBlob{};
	D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
	g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_UIRootSignature.GetAddressOf()));
	pBlob->Release();
}
void CRaytracingCaveScene::CreateUIPipelineState()
{
	ID3DBlob* pd3dVBlob{ nullptr };
	ID3DBlob* pd3dPBlob{ nullptr };
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineState{};
	d3dPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineState.pRootSignature = m_UIRootSignature.Get();

	D3D12_INPUT_ELEMENT_DESC ldesc[3]{};
	ldesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3dPipelineState.InputLayout.pInputElementDescs = ldesc;
	d3dPipelineState.InputLayout.NumElements = 3;

	d3dPipelineState.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dPipelineState.DepthStencilState.StencilEnable = FALSE;

	d3dPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	d3dPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	d3dPipelineState.RasterizerState.AntialiasedLineEnable = FALSE;
	d3dPipelineState.RasterizerState.FrontCounterClockwise = FALSE;
	d3dPipelineState.RasterizerState.MultisampleEnable = FALSE;
	d3dPipelineState.RasterizerState.DepthClipEnable = FALSE;

	d3dPipelineState.BlendState.AlphaToCoverageEnable = FALSE;
	d3dPipelineState.BlendState.IndependentBlendEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendEnable = TRUE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dPipelineState.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	d3dPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineState.NumRenderTargets = 1;
	d3dPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineState.SampleDesc.Count = 1;
	d3dPipelineState.SampleMask = UINT_MAX;

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &pd3dVBlob, nullptr);
	d3dPipelineState.VS.BytecodeLength = pd3dVBlob->GetBufferSize();
	d3dPipelineState.VS.pShaderBytecode = pd3dVBlob->GetBufferPointer();

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", 0, 0, &pd3dPBlob, nullptr);
	d3dPipelineState.PS.BytecodeLength = pd3dPBlob->GetBufferSize();
	d3dPipelineState.PS.pShaderBytecode = pd3dPBlob->GetBufferPointer();

	g_DxResource.device->CreateGraphicsPipelineState(&d3dPipelineState, IID_PPV_ARGS(m_UIPipelineState.GetAddressOf()));

	if (pd3dVBlob)
		pd3dVBlob->Release();
	if (pd3dPBlob)
		pd3dPBlob->Release();
}

void CRaytracingCaveScene::CreateMageCharacter()
{
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\Greycloak\\", JOB_MAGE);
	m_vPlayers.emplace_back(std::make_unique<CPlayerMage>(
		m_pResourceManager->getSkinningObjectList()[m_pResourceManager->getSkinningObjectList().size() - 1].get(),
		m_pResourceManager->getAnimationManagers()[m_pResourceManager->getAnimationManagers().size() - 1].get(), false));

	// Create Mage's own objects and Set
	// ex) bullet, particle, barrier  etc...
}

void CRaytracingCaveScene::UpdateObject(float fElapsedTime)
{
	// compute shader & rootSignature set
	g_DxResource.cmdList->SetPipelineState(m_pAnimationComputeShader.Get());
	g_DxResource.cmdList->SetComputeRootSignature(m_pComputeRootSignature.Get());

	m_pResourceManager->UpdateSkinningMesh(fElapsedTime);
	// particle update
	// SetRootSignature
	// particle update

	Flush();
	// Skinning Object BLAS ReBuild
	m_pResourceManager->ReBuildBLAS();

	m_pResourceManager->UpdateWorldMatrix();

	for (auto& p : m_vPlayers)
		p->UpdateObject(fElapsedTime);

	m_pPlayer->CollisionCheck(m_pCollisionHMap.get(), fElapsedTime, -200.0f, 0.0, -66.5f, SCENE_CAVE);
	m_pPlayer->HeightCheck(m_pHeightMap.get(), fElapsedTime, -200.0f, -10.0f, -66.5f, SCENE_CAVE);

	if (m_pCamera->getThirdPersonState()) {
		XMFLOAT3& EYE = m_pCamera->getEyeCalculateOffset();
		float cHeight = m_pHeightMap->GetHeightinWorldSpace(EYE.x + 200.0f, EYE.z + 66.5f);
		if (EYE.y < cHeight - 10.0f + 0.5f) {
			m_pCamera->UpdateViewMatrix(cHeight - 10.0f + 0.5f);
		}
		else
			m_pCamera->UpdateViewMatrix();
	}
	else
		m_pCamera->UpdateViewMatrix();
	m_pAccelerationStructureManager->UpdateScene(m_pCamera->getEye());

	switch (m_nState) {
	case IS_LOADING: {
		wOpacity -= 0.5 * fElapsedTime;
		if (wOpacity < 0.0f) {
			m_nState = IS_GAMING;
			wOpacity = 0.0f;
		}
		m_vUIs[0]->setColor(0.0, 0.0, 0.0, wOpacity);
		break;
	}
	case IS_GAMING: {
		break;
	}
	case IS_FINISH: {
		wOpacity += 0.2 * fElapsedTime;
		if (wOpacity > 1.0f) {
			ShowCursor(TRUE);
			m_nNextScene = SCENE_WINTERLAND;
			wOpacity = 1.0f;
		}
		m_vUIs[0]->setColor(0.0, 0.0, 0.0, wOpacity);
		break;
	}
	}
	// Player UI ==================================================
	int buffstart = 20; int bstride = 40;
	for (int i = 0; i < m_numUser; ++i) {
		int t{};
		// hp/mp
		m_vStatusUIs[i][1]->setScaleX(cHPs[i] / maxHPs[i]);
		if (i == 0) {
			m_vStatusUIs[i][3]->setScaleXWithUV(cMP / maxMP);
		}
		if (i > 0) {
			buffstart = 15; bstride = 30;
		}
		for (int j = 0; j < 3; ++j) {
			if (m_BuffState[i][j]) {
				m_vStatusUIs[i][j + 4]->setRenderState(true);
				m_vStatusUIs[i][j + 4]->setPositionInViewport(buffstart + (t * bstride), m_buffpixelHeight[i]);
				++t;
			}
			else {
				m_vStatusUIs[i][j + 4]->setRenderState(false);
			}
		}
	}

	{
		if (cMP < 30)
			m_vSkillUIs[6]->setRenderState(true);
		else
			m_vSkillUIs[6]->setRenderState(false);

		if (cMP < 40)
			m_vSkillUIs[7]->setRenderState(true);
		else
			m_vSkillUIs[7]->setRenderState(false);

		if (cMP < 60)
			m_vSkillUIs[8]->setRenderState(true);
		else
			m_vSkillUIs[8]->setRenderState(false);

		for (int i = 0; i < 3; ++i) {
			curCTime[i] -= fElapsedTime;
			if (curCTime[i] < 0) curCTime[i] = 0.0f;
			m_vSkillUIs[i + 3]->setScaleY(curCTime[i] / coolTime[i]);
		}
	}
	// =================================================================
}

void CRaytracingCaveScene::Render()
{
	m_pCamera->SetShaderVariable();
	m_pAccelerationStructureManager->SetScene();
	m_pResourceManager->SetLights();

	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	g_DxResource.cmdList->SetComputeRootDescriptorTable(4, textures[m_nSkyboxIndex]->getView()->GetGPUDescriptorHandleForHeapStart());

	D3D12_DISPATCH_RAYS_DESC raydesc{};
	raydesc.Depth = 1;
	raydesc.Width = DEFINED_UAV_BUFFER_WIDTH;
	raydesc.Height = DEFINED_UAV_BUFFER_HEIGHT;

	raydesc.RayGenerationShaderRecord.StartAddress = m_pShaderBindingTable->getRayGenTable()->GetGPUVirtualAddress();
	raydesc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	raydesc.MissShaderTable.StartAddress = m_pShaderBindingTable->getMissTable()->GetGPUVirtualAddress();
	raydesc.MissShaderTable.SizeInBytes = m_pShaderBindingTable->getMissSize();
	raydesc.MissShaderTable.StrideInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	raydesc.HitGroupTable.StartAddress = m_pShaderBindingTable->getHitGroupTable()->GetGPUVirtualAddress();
	raydesc.HitGroupTable.SizeInBytes = m_pShaderBindingTable->getHitGroupSize();
	raydesc.HitGroupTable.StrideInBytes = m_pShaderBindingTable->getHitGroupStride();

	g_DxResource.cmdList->DispatchRays(&raydesc);

	// UI Render ==================================================================================

	ID3D12GraphicsCommandList4* cmdList = g_DxResource.cmdList;
	auto barrier = [&](ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		{
			D3D12_RESOURCE_BARRIER resBarrier{};
			resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resBarrier.Transition.pResource = pResource;
			resBarrier.Transition.StateBefore = before;
			resBarrier.Transition.StateAfter = after;
			resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmdList->ResourceBarrier(1, &resBarrier);
		};

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_VIEWPORT vv{};
	vv.Width = DEFINED_UAV_BUFFER_WIDTH; vv.Height = DEFINED_UAV_BUFFER_HEIGHT; vv.MinDepth = 0.0f; vv.MaxDepth = 1.0f;
	cmdList->RSSetViewports(1, &vv);
	D3D12_RECT ss{ 0, 0, DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT };
	cmdList->RSSetScissorRects(1, &ss);
	cmdList->OMSetRenderTargets(1, &m_RTV->GetCPUDescriptorHandleForHeapStart(), FALSE, &m_DSV->GetCPUDescriptorHandleForHeapStart());
	cmdList->SetGraphicsRootSignature(m_UIRootSignature.Get());
	cmdList->SetPipelineState(m_UIPipelineState.Get());
	cmdList->SetGraphicsRootConstantBufferView(0, m_cameraCB->GetGPUVirtualAddress());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// player UI ====================================
	for (short i = 0; i < m_numUser; ++i) {
		for (auto& p : m_vStatusUIs[i])
			p->Render();
	}
	for (auto& p : m_vItemUIs)
		p->Render();

	for (auto& p : m_vSkillUIs)
		p->Render();

	m_pShopUI->Render();
	// ===============================================

	for (auto& p : m_vUIs)
		p->Render();

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void CRaytracingCaveScene::PlayerUISetup(short job)
{
	size_t mindex{};
	size_t tindex{};
	size_t uindex{};

	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();

	// status UI ===================================================================
	maxHPs[0] = 1200; maxHPs[1] = 1000; maxHPs[2] = 800;
	cHPs[0] = 1200; cHPs[1] = 800; cHPs[2] = 730;

	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 30, 30));		// buff icon
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 340, 28));		// hp/mp bar
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 330, 18));		// hp/mp

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 255, 12));		// coop hp/mp

	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_HPbar.dds"));	// HPbar
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MPbar.dds"));	// MPbar
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MP.dds"));	// MP

	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Buff0.dds"));	// buff0
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Buff1.dds"));	// buff1
	{
		uindex = m_vStatusUIs[0].size();			// 0 - hpbar
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 20);

		uindex = m_vStatusUIs[0].size();			// 1 - hp
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 2].get()));
		m_vStatusUIs[0][uindex]->setColor(1.0, 0.0, 0.0, 1.0);
		m_vStatusUIs[0][uindex]->setPositionInViewport(25, 25);

		uindex = m_vStatusUIs[0].size();			// 2 - mp bar
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex + 1].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 60);

		uindex = m_vStatusUIs[0].size();			// 2 - mp
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 2].get(), textures[tindex + 2].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(25, 65);

		m_buffpixelHeight[0] = 100;
		uindex = m_vStatusUIs[0].size();			// 3 ~ 5 buff
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 3].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 100);
		uindex = m_vStatusUIs[0].size();			// 3 ~ 5 buff
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 4].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 100);
		uindex = m_vStatusUIs[0].size();			// 3 ~ 5 buff
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 100);
		m_vStatusUIs[0][uindex]->setColor(0.7, 1.0, 0.0, 1.0);
	}

	//for (int i = 0; i < 2; ++i) {
	//	uindex = m_vStatusUIs[i + 1].size();			// 1 - hp
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get()));
	//	m_vStatusUIs[i + 1][uindex]->setPositionInViewport(15, (i * 115) + 150 + 15);
	//	m_vStatusUIs[i + 1][uindex]->setColor(1.0, 0.0, 0.0, 1.0);

	//	uindex = m_vStatusUIs[i + 1].size();			// 2 - mp
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get()));
	//	m_vStatusUIs[i + 1][uindex]->setPositionInViewport(15, (i * 115) + 150 + 15 + 30);
	//	m_vStatusUIs[i + 1][uindex]->setColor(0.0, 0.0, 1.0, 1.0);

	//	m_buffpixelHeight[i + 1] = (i * 115) + 150 + 15 + 30 + 30;
	//	uindex = m_vStatusUIs[i + 1].size();			// 3 ~ 5 buff
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	//	m_vStatusUIs[i + 1][uindex]->setScale(0.75);
	//	m_vStatusUIs[i + 1][uindex]->setColor(0.0, 1.0, 1.0, 1.0);
	//	uindex = m_vStatusUIs[i + 1].size();			// 3 ~ 5 buff
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	//	m_vStatusUIs[i + 1][uindex]->setScale(0.75);
	//	m_vStatusUIs[i + 1][uindex]->setColor(1.0, 0.5, 1.0, 1.0);
	//	uindex = m_vStatusUIs[i + 1].size();			// 3 ~ 5 buff
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	//	m_vStatusUIs[i + 1][uindex]->setScale(0.75);
	//	m_vStatusUIs[i + 1][uindex]->setColor(0.7, 1.0, 0.0, 1.0);
	//}
	// =============================================================================

	// item ========================================================================
	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 140, 175));

	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item0.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item1.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item2.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item3.dds"));

	for (int i = 0; i < 4; ++i) {
		uindex = m_vItemUIs.size();
		m_vItemUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + i].get()));
		m_vItemUIs[uindex]->setColor(0.2 * (i + 1), 0.3, 0.2 * (i + 1), 1.0);
		m_vItemUIs[uindex]->setPositionInViewport(20, 525);
		m_vItemUIs[uindex]->setRenderState(false);
	}
	m_vItemUIs[0]->setRenderState(true);

	// =============================================================================

	// skills ======================================================================

	coolTime[0] = 5.0f; coolTime[1] = 10.0f; coolTime[2] = 20.0f;

	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 100, 100));

	tindex = textures.size();
	switch (job) {
	case JOB_MAGE:
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician0.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician1.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician2.dds"));
		break;
	}
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_MP_Less.dds"));
	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + i].get()));
		//m_vSkillUIs[uindex]->setColor(1.0, 0.5, 0.5, 1.0);
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
		m_vSkillUIs[uindex]->setColor(0.0, 0.0, 0.0, 0.5);
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 3].get()));
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	// Shop ============================================================================
	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 400, 500));
	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Shop.dds"));
	m_pShopUI = std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex].get());
	m_pShopUI->setPositionInViewport(780, 50);
	m_pShopUI->setRenderState(false);
}

// ==========================================================================

void CRaytracingETPScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer, std::shared_ptr<CRayTracingPipeline> pipeline)
{
	m_pOutputBuffer = outputBuffer;

	// CreateUISetup
	CreateOrthoMatrixBuffer();
	CreateRTVDSV();
	CreateUIRootSignature();
	CreateUIPipelineState();

	// Create Global & Local Root Signature
	//CreateRootSignature();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	/*m_pRaytracingPipeline = std::make_unique<CRayTracingPipeline>();
	m_pRaytracingPipeline->Setup(1 + 2 + 1 + 2 + 1 + 1);
	m_pRaytracingPipeline->AddLibrarySubObject(compiledShader, std::size(compiledShader));
	m_pRaytracingPipeline->AddHitGroupSubObject(L"HitGroup", L"RadianceClosestHit", L"RadianceAnyHit");
	m_pRaytracingPipeline->AddHitGroupSubObject(L"ShadowHit", L"ShadowClosestHit", L"ShadowAnyHit");
	m_pRaytracingPipeline->AddShaderConfigSubObject(8, 20);
	m_pRaytracingPipeline->AddLocalRootAndAsoociationSubObject(m_pLocalRootSignature.Get());
	m_pRaytracingPipeline->AddGlobalRootSignatureSubObject(m_pGlobalRootSignature.Get());
	m_pRaytracingPipeline->AddPipelineConfigSubObject(6);
	m_pRaytracingPipeline->MakePipelineState();*/
	m_pRaytracingPipeline = pipeline;

	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	m_pResourceManager->SetUp(3);
	// Object File Read ========================================	! !
	m_pResourceManager->AddResourceFromFile(L"src\\model\\Map\\ETP\\ETP.bin", "src\\texture\\Map\\");
	m_pResourceManager->AddResourceFromFile(L"src\\model\\Map\\ETP\\Water.bin", "src\\texture\\Map\\");

	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\Greycloak\\", JOB_MAGE);
	CreateMageCharacter();
	m_pPlayer = std::make_unique<CPlayer>(m_vPlayers[m_vPlayers.size() - 1].get(), m_pCamera);

	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Gorhorrid.bin", "src\\texture\\Gorhorrid\\");
	// Light Read
	m_pResourceManager->AddLightsFromFile(L"src\\Light\\Light_ETP.bin");
	m_pResourceManager->ReadyLightBufferContent();
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// Create Normal Object & skinning Object Copy ========================================

	for (auto& o : skinned[1]->getObjects()) {
		for (auto& ma : o->getMaterials())
			ma.m_bHasEmissiveColor = false;
	}

	{		// Water
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\WaterTurbulent00_NORM.dds"));
		auto p = normalObjects[normalObjects.size() - 1].get();
		p->SetInstanceID(2);
		p->getMaterials().emplace_back();
		Material& mt = p->getMaterials()[0];
		mt.m_bHasAlbedoColor = true; mt.m_xmf4AlbedoColor = XMFLOAT4(0.1613118, 0.2065666, 0.2358491, 0.2);
		//mt.m_bHasMetallicMap = true; mt.m_nMetallicMapIndex = textures.size() - 1;
		mt.m_bHasNormalMap = true; mt.m_nNormalMapIndex = textures.size() - 1;

		/*void* tempptr{};
		std::vector<XMFLOAT2> tex0 = meshes[p->getMeshIndex()]->getTex0();
		for (XMFLOAT2& xmf : tex0) {
			xmf.x *= 10.0f; xmf.y *= 10.0f;
		}
		meshes[p->getMeshIndex()]->getTexCoord0Buffer()->Map(0, nullptr, &tempptr);
		memcpy(tempptr, tex0.data(), sizeof(XMFLOAT2) * tex0.size());
		meshes[p->getMeshIndex()]->getTexCoord0Buffer()->Unmap(0, nullptr);*/
	}

	m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\Map\\ETP\\ETP_Terrain.raw", 1024, 1024, XMFLOAT3(1.0f, 0.0156, 1.0f));
	meshes.emplace_back(std::make_unique<Mesh>(m_pHeightMap.get(), "terrain"));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[normalObjects.size() - 1]->SetMeshIndex(meshes.size() - 1);

	normalObjects[normalObjects.size() - 1]->SetInstanceID(10);
	normalObjects[normalObjects.size() - 1]->getMaterials().emplace_back();
	normalObjects[normalObjects.size() - 1]->getMaterials()[0].m_bHasMetallic = true;
	normalObjects[normalObjects.size() - 1]->getMaterials()[0].m_bHasGlossiness = true;
	normalObjects[normalObjects.size() - 1]->getMaterials()[0].m_fGlossiness = 0.0f;
	normalObjects[normalObjects.size() - 1]->SetPosition(XMFLOAT3(-512.0, 0.0, -512.0));

	m_TerrainRoad = std::make_unique<CHeightMapImage>(L"src\\model\\Map\\ETP\\ETP_Terrain_Road.raw", 1024, 1024, XMFLOAT3(1.0f, 0.0156, 1.0f));
	m_CollisionHMap = std::make_unique<CHeightMapImage>(L"src\\model\\Map\\ETP\\ETP_CollisionMap.raw", 1024, 1024, XMFLOAT3(1.0f, 0.0156, 1.0f));
	PrepareTerrainTexture();

	// cubeMap Ready
	m_nSkyboxIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\ETPSky.dds", true));
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();
	m_pResourceManager->PrepareObject();	// Ready OutputBuffer to  SkinningObject


	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// Copy(normalObject) & SetPreMatrix ===============================

	skinned[0]->setPreTransform(2.5f, XMFLOAT3(), XMFLOAT3());
	skinned[0]->SetPosition(XMFLOAT3(-312.0, 0.0, -112.0));


	// ==============================================================================

	// Camera Setting ==============================================================
	m_pCamera->SetTarget(skinned[0]->getObjects()[0].get());
	m_pCamera->SetHOffset(3.5f);
	m_pCamera->SetCameraLength(15.0f);
	m_pCamera->SetMapNumber(SCENE_PLAIN);
	// ==========================================================================

	// AccelerationStructure
	m_pAccelerationStructureManager = std::make_unique<CAccelerationStructureManager>();
	m_pAccelerationStructureManager->Setup(m_pResourceManager.get(), 1);
	m_pAccelerationStructureManager->InitBLAS();
	m_pAccelerationStructureManager->InitTLAS();

	// UISetup ========================================================================
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT));
	m_vUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get()));
	m_vUIs[m_vUIs.size() - 1]->setPositionInViewport(0, 0);
	m_vUIs[m_vUIs.size() - 1]->setColor(0.0, 0.0, 0.0, 1.0);

	PlayerUISetup(user_job[m_local_id]);
}

void CRaytracingETPScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
		switch (wParam) {
		case 'N':
			m_pCamera->toggleNormalMapping();
			break;
		case 'M':
			m_pCamera->toggleAlbedoColor();
			break;
		case 'B':
			m_pCamera->toggleReflection();
			break;
		case '9':
			m_pCamera->SetThirdPersonMode(false);
			break;
		case '0':
			m_pCamera->SetThirdPersonMode(true);
			break;
		case '8':
			if (m_nState == IS_GAMING) {
				startTime = 0.0f;
				m_nState = IS_FINISH;
			}
			break;
		case 'U':
			cHPs[0] += 10;
			if (maxHPs[0] < cHPs[0])
				cHPs[0] = maxHPs[0];
			break;
		case 'J':
			cHPs[0] -= 10;
			if (0 > cHPs[0])
				cHPs[0] = 0;
			break;
		case '1':
			m_BuffState[0] = !m_BuffState[0];
			break;
		case '2':
			m_BuffState[1] = !m_BuffState[1];
			break;
		case '3':
			m_BuffState[2] = !m_BuffState[2];
			break;
		case '4':
			cMPs[m_local_id] = 100;
			break;
		case 'Z':
			m_vItemUIs[cItem]->setRenderState(false);
			--cItem;
			if (cItem < 0) cItem = 3;
			m_vItemUIs[cItem]->setRenderState(true);
			//m_pTextManager->UpdateText(ItemNumTextIndex, std::to_wstring(itemNum[cItem]).c_str(), 125, 665, true);
			break;
		case 'C':
			m_vItemUIs[cItem]->setRenderState(false);
			++cItem;
			if (cItem > 3) cItem = 0;
			m_vItemUIs[cItem]->setRenderState(true);
			//m_pTextManager->UpdateText(ItemNumTextIndex, std::to_wstring(itemNum[cItem]).c_str(), 125, 665, true);
			break;
		case 'X':
			if (itemNum[cItem] > 0 && m_nState == IS_GAMING) {
				--itemNum[cItem];
				//m_pTextManager->UpdateText(ItemNumTextIndex, std::to_wstring(itemNum[cItem]).c_str(), 125, 665, true);
			}
			break;
		case 'Q':
			if (cMPs[m_local_id] >= 30 && curCTime[0] <= 0) {
				cMPs[m_local_id] -= 30;
				curCTime[0] = coolTime[0];
			}
			break;
		case 'E':
			if (cMPs[m_local_id] >= 40 && curCTime[1] <= 0) {
				cMPs[m_local_id] -= 40;
				curCTime[1] = coolTime[1];
			}
			break;
		case 'R':
			if (cMPs[m_local_id] >= 60 && curCTime[2] <= 0) {
				cMPs[m_local_id] -= 60;
				curCTime[2] = coolTime[2];
			}
			break;
		case 'P':
			m_bUIOnOff = !m_bUIOnOff;
			break;
		}
		break;
	case WM_KEYUP:
		break;
	}
}

void CRaytracingETPScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	m_pPlayer->MouseProcess(hWnd, nMessage, wParam, lParam);
}

void CRaytracingETPScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);

	if (m_nState == IS_GAMING) {
		if (!m_pCamera->getThirdPersonState()) {
			bool shiftDown = false;
			if (keyBuffer[VK_SHIFT] & 0x80)
				shiftDown = true;
			if (keyBuffer['W'] & 0x80)
				m_pCamera->Move(0, fElapsedTime, shiftDown);
			if (keyBuffer['S'] & 0x80)
				m_pCamera->Move(5, fElapsedTime, shiftDown);
			if (keyBuffer['D'] & 0x80)
				m_pCamera->Move(3, fElapsedTime, shiftDown);
			if (keyBuffer['A'] & 0x80)
				m_pCamera->Move(4, fElapsedTime, shiftDown);
			if (keyBuffer[VK_SPACE] & 0x80)
				m_pCamera->Move(1, fElapsedTime, shiftDown);
			if (keyBuffer[VK_CONTROL] & 0x80)
				m_pCamera->Move(2, fElapsedTime, shiftDown);
		}
		else {
			m_pPlayer->ProcessInput(keyBuffer, fElapsedTime);
		}
	}
}

void CRaytracingETPScene::CreateUIRootSignature()
{
	D3D12_DESCRIPTOR_RANGE tRange{};
	tRange.BaseShaderRegister = 0;
	tRange.NumDescriptors = 1;
	tRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	D3D12_ROOT_PARAMETER params[3]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[0].Descriptor.RegisterSpace = 0;
	params[0].Descriptor.ShaderRegister = 0;

	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[1].Descriptor.RegisterSpace = 0;
	params[1].Descriptor.ShaderRegister = 1;

	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 1;
	params[2].DescriptorTable.pDescriptorRanges = &tRange;

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};								// s0
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rtDesc{};
	rtDesc.NumParameters = 3;
	rtDesc.NumStaticSamplers = 1;
	rtDesc.pParameters = params;
	rtDesc.pStaticSamplers = &samplerDesc;
	rtDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* pBlob{};
	D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
	g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_UIRootSignature.GetAddressOf()));
	pBlob->Release();
}
void CRaytracingETPScene::CreateUIPipelineState()
{
	ID3DBlob* pd3dVBlob{ nullptr };
	ID3DBlob* pd3dPBlob{ nullptr };
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineState{};
	d3dPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineState.pRootSignature = m_UIRootSignature.Get();

	D3D12_INPUT_ELEMENT_DESC ldesc[3]{};
	ldesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3dPipelineState.InputLayout.pInputElementDescs = ldesc;
	d3dPipelineState.InputLayout.NumElements = 3;

	d3dPipelineState.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dPipelineState.DepthStencilState.StencilEnable = FALSE;

	d3dPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	d3dPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	d3dPipelineState.RasterizerState.AntialiasedLineEnable = FALSE;
	d3dPipelineState.RasterizerState.FrontCounterClockwise = FALSE;
	d3dPipelineState.RasterizerState.MultisampleEnable = FALSE;
	d3dPipelineState.RasterizerState.DepthClipEnable = FALSE;

	d3dPipelineState.BlendState.AlphaToCoverageEnable = FALSE;
	d3dPipelineState.BlendState.IndependentBlendEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendEnable = TRUE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dPipelineState.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	d3dPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineState.NumRenderTargets = 1;
	d3dPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineState.SampleDesc.Count = 1;
	d3dPipelineState.SampleMask = UINT_MAX;

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &pd3dVBlob, nullptr);
	d3dPipelineState.VS.BytecodeLength = pd3dVBlob->GetBufferSize();
	d3dPipelineState.VS.pShaderBytecode = pd3dVBlob->GetBufferPointer();

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", 0, 0, &pd3dPBlob, nullptr);
	d3dPipelineState.PS.BytecodeLength = pd3dPBlob->GetBufferSize();
	d3dPipelineState.PS.pShaderBytecode = pd3dPBlob->GetBufferPointer();

	g_DxResource.device->CreateGraphicsPipelineState(&d3dPipelineState, IID_PPV_ARGS(m_UIPipelineState.GetAddressOf()));

	if (pd3dVBlob)
		pd3dVBlob->Release();
	if (pd3dPBlob)
		pd3dPBlob->Release();
}

void CRaytracingETPScene::CreateMageCharacter()
{
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\Greycloak\\", JOB_MAGE);
	m_vPlayers.emplace_back(std::make_unique<CPlayerMage>(
		m_pResourceManager->getSkinningObjectList()[m_pResourceManager->getSkinningObjectList().size() - 1].get(),
		m_pResourceManager->getAnimationManagers()[m_pResourceManager->getAnimationManagers().size() - 1].get(), false));

	// Create Mage's own objects and Set
	// ex) bullet, particle, barrier  etc...
}

void CRaytracingETPScene::PrepareTerrainTexture()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 14;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	g_DxResource.device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pTerrainDescriptor.GetAddressOf()));

	struct alignas(16) terrainINFO {
		int numLayer{};
		float padding[3]{};
		int bHasDiffuse[4]{};
		int bHasNormal[4]{};
		int bHasMask[4]{};
	};

	auto rdesc = BASIC_BUFFER_DESC;
	rdesc.Width = Align(sizeof(terrainINFO), 256);

	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &rdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(m_pTerrainCB.GetAddressOf()));

	terrainINFO* pMap{};
	m_pTerrainCB->Map(0, nullptr, reinterpret_cast<void**>(&pMap));
	pMap->numLayer = 4;
	pMap->bHasDiffuse[0] = pMap->bHasDiffuse[1] = pMap->bHasDiffuse[2] = pMap->bHasDiffuse[3] = 1;
	pMap->bHasNormal[0] = pMap->bHasNormal[1] = pMap->bHasNormal[2] = pMap->bHasNormal[3] = 0;
	pMap->bHasMask[0] = pMap->bHasMask[2] = pMap->bHasMask[3] = 0;
	m_pTerrainCB->Unmap(0, nullptr);

	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	size_t textureIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Terrain_ETP_splatmap.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\SandRiver00_Terrain.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\Grassgreen00_Albedo.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\GroundSoil00_Terrain.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\dlnk_RockWall_00_Terrain.dds"));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	D3D12_RESOURCE_DESC d3dRD;

	UINT increment = g_DxResource.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_CPU_DESCRIPTOR_HANDLE  handle = m_pTerrainDescriptor->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cdesc{};
	cdesc.BufferLocation = m_pTerrainCB->GetGPUVirtualAddress();
	cdesc.SizeInBytes = rdesc.Width;

	g_DxResource.device->CreateConstantBufferView(&cdesc, handle);
	handle.ptr += increment;

	// splat
	d3dRD = textures[textureIndex]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;

	g_DxResource.device->CreateShaderResourceView(textures[textureIndex]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 0 ===============================================================

	d3dRD = textures[textureIndex + 1]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 1]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 1 ===============================================================

	d3dRD = textures[textureIndex + 2]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 2]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 2 ===============================================================

	d3dRD = textures[textureIndex + 3]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 3]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 3 ===============================================================

	d3dRD = textures[textureIndex + 4]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 4]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;
}

void CRaytracingETPScene::UpdateObject(float fElapsedTime)
{
	// compute shader & rootSignature set
	g_DxResource.cmdList->SetPipelineState(m_pAnimationComputeShader.Get());
	g_DxResource.cmdList->SetComputeRootSignature(m_pComputeRootSignature.Get());

	m_pResourceManager->UpdateSkinningMesh(fElapsedTime);
	// particle update
	// SetRootSignature
	// particle update

	Flush();
	// Skinning Object BLAS ReBuild
	m_pResourceManager->ReBuildBLAS();

	m_pResourceManager->UpdateWorldMatrix();

	for (auto& p : m_vPlayers)
		p->UpdateObject(fElapsedTime);

	m_pPlayer->CollisionCheck(m_TerrainRoad.get(), m_CollisionHMap.get(), fElapsedTime, -512.0f, 0.0f, -512.0f, SCENE_PLAIN);
	m_pPlayer->HeightCheck(m_TerrainRoad.get(), fElapsedTime, -512.0f, 0.0f, -512.0f, SCENE_PLAIN);

	if (m_pCamera->getThirdPersonState()) {
		XMFLOAT3& EYE = m_pCamera->getEyeCalculateOffset();
		float cHeight = m_TerrainRoad->GetHeightinWorldSpace(EYE.x + 512.0f, EYE.z + 512.0f);
		if (EYE.y < cHeight + 0.5f) {
			m_pCamera->UpdateViewMatrix(cHeight + 0.5f);
		}
		else
			m_pCamera->UpdateViewMatrix();
	}
	else
		m_pCamera->UpdateViewMatrix();
	m_pAccelerationStructureManager->UpdateScene(m_pCamera->getEye());

	switch (m_nState) {
	case IS_LOADING: {
		wOpacity -= 0.5 * fElapsedTime;
		if (wOpacity < 0.0f) {
			m_nState = IS_GAMING;
			wOpacity = 0.0f;
		}
		m_vUIs[0]->setColor(0.0, 0.0, 0.0, wOpacity);
		break;
	}
	case IS_GAMING: {
		break;
	}
	case IS_FINISH: {
		wOpacity += 0.2 * fElapsedTime;
		if (wOpacity > 1.0f) {
			ShowCursor(TRUE);
			m_nNextScene = SCENE_CAVE;
			wOpacity = 1.0f;
		}
		m_vUIs[0]->setColor(0.0, 0.0, 0.0, wOpacity);
		break;
	}
	}

	// Player UI ==================================================
	int buffstart = 20; int bstride = 40;

	for (int i = 0; i < m_numUser; ++i) {
		int t{};
		// hp/mp
		m_vPlayersStatUI[i][1]->setScaleX(cHPs[i] / maxHPs[i]);
		m_vPlayersStatUI[i][3]->setScaleXWithUV(cMPs[i] / maxMPs[i]);
		if (i == m_local_id) {
			for (int j = 0; j < 3; ++j) {
				if (m_BuffState[j]) {
					m_vPlayersStatUI[i][j + 4]->setRenderState(true);
					m_vPlayersStatUI[i][j + 4]->setPositionInViewport(buffstart + (t * bstride), 100);
					++t;
				}
				else {
					m_vPlayersStatUI[i][j + 4]->setRenderState(false);
				}
			}
		}
	}

	{
		if (cMPs[m_local_id] < 30)
			m_vSkillUIs[6]->setRenderState(true);
		else
			m_vSkillUIs[6]->setRenderState(false);

		if (cMPs[m_local_id] < 40)
			m_vSkillUIs[7]->setRenderState(true);
		else
			m_vSkillUIs[7]->setRenderState(false);

		if (cMPs[m_local_id] < 60)
			m_vSkillUIs[8]->setRenderState(true);
		else
			m_vSkillUIs[8]->setRenderState(false);

		for (int i = 0; i < 3; ++i) {
			curCTime[i] -= fElapsedTime;
			if (curCTime[i] < 0) curCTime[i] = 0.0f;
			m_vSkillUIs[i + 3]->setScaleY(curCTime[i] / coolTime[i]);
		}
	}
	// =================================================================
}

void CRaytracingETPScene::Render()
{
	m_pCamera->SetShaderVariable();
	m_pAccelerationStructureManager->SetScene();
	m_pResourceManager->SetLights();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	g_DxResource.cmdList->SetComputeRootDescriptorTable(4, textures[m_nSkyboxIndex]->getView()->GetGPUDescriptorHandleForHeapStart());
	g_DxResource.cmdList->SetComputeRootDescriptorTable(5, m_pTerrainDescriptor->GetGPUDescriptorHandleForHeapStart());

	D3D12_DISPATCH_RAYS_DESC raydesc{};
	raydesc.Depth = 1;
	raydesc.Width = DEFINED_UAV_BUFFER_WIDTH;
	raydesc.Height = DEFINED_UAV_BUFFER_HEIGHT;

	raydesc.RayGenerationShaderRecord.StartAddress = m_pShaderBindingTable->getRayGenTable()->GetGPUVirtualAddress();
	raydesc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	raydesc.MissShaderTable.StartAddress = m_pShaderBindingTable->getMissTable()->GetGPUVirtualAddress();
	raydesc.MissShaderTable.SizeInBytes = m_pShaderBindingTable->getMissSize();
	raydesc.MissShaderTable.StrideInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	raydesc.HitGroupTable.StartAddress = m_pShaderBindingTable->getHitGroupTable()->GetGPUVirtualAddress();
	raydesc.HitGroupTable.SizeInBytes = m_pShaderBindingTable->getHitGroupSize();
	raydesc.HitGroupTable.StrideInBytes = m_pShaderBindingTable->getHitGroupStride();

	g_DxResource.cmdList->DispatchRays(&raydesc);

	// UI Render ==================================================================================

	ID3D12GraphicsCommandList4* cmdList = g_DxResource.cmdList;
	auto barrier = [&](ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		{
			D3D12_RESOURCE_BARRIER resBarrier{};
			resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resBarrier.Transition.pResource = pResource;
			resBarrier.Transition.StateBefore = before;
			resBarrier.Transition.StateAfter = after;
			resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmdList->ResourceBarrier(1, &resBarrier);
		};

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_VIEWPORT vv{};
	vv.Width = DEFINED_UAV_BUFFER_WIDTH; vv.Height = DEFINED_UAV_BUFFER_HEIGHT; vv.MinDepth = 0.0f; vv.MaxDepth = 1.0f;
	cmdList->RSSetViewports(1, &vv);
	D3D12_RECT ss{ 0, 0, DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT };
	cmdList->RSSetScissorRects(1, &ss);
	cmdList->OMSetRenderTargets(1, &m_RTV->GetCPUDescriptorHandleForHeapStart(), FALSE, &m_DSV->GetCPUDescriptorHandleForHeapStart());
	cmdList->SetGraphicsRootSignature(m_UIRootSignature.Get());
	cmdList->SetPipelineState(m_UIPipelineState.Get());
	cmdList->SetGraphicsRootConstantBufferView(0, m_cameraCB->GetGPUVirtualAddress());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// player UI ====================================
	if (m_bUIOnOff) {
		for (int i = 0; i < m_numUser; ++i) {
			for (auto& p : m_vPlayersStatUI[i])
				p->Render();
		}

		for (auto& p : m_vItemUIs)
			p->Render();

		for (auto& p : m_vSkillUIs)
			p->Render();
	}
	// ===============================================


	for (auto& p : m_vUIs)	// black plane
		p->Render();

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

void CRaytracingETPScene::PlayerUISetup(short job)
{
	size_t mindex{};
	size_t tindex{};
	size_t uindex{};

	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();

	// status UI ===================================================================
	maxHPs[0] = 1200; maxHPs[1] = 1000; maxHPs[2] = 800;
	cHPs[0] = 1200; cHPs[1] = 800; cHPs[2] = 730;

	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 30, 30));		// buff icon
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 340, 28));		// hp/mp bar
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 330, 18));		// hp/mp

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 264, 14.4));		// coop hp/mp 
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 60, 60));		// coop player face

	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_HPbar.dds"));	// HPbar
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MPbar.dds"));	// MPbar
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MP.dds"));	// MP

	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Buff0.dds"));	// buff0
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Buff1.dds"));	// buff1
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Buff2.dds"));	// buff2

	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MiniPlayer0.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MiniPlayer1.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MiniPlayer2.dds"));

	int otherPlayer = 0;
	for (int i = 0; i < m_numUser; ++i) {
		if (i == m_local_id) {
			uindex = m_vPlayersStatUI[i].size();			// 0 - hpbar
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex].get()));
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(20, 20);

			uindex = m_vPlayersStatUI[i].size();			// 1 - hp
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 2].get()));
			m_vPlayersStatUI[i][uindex]->setColor(1.0, 0.0, 0.0, 1.0);
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(25, 25);

			uindex = m_vPlayersStatUI[i].size();			// 2 - mp bar
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex + 1].get()));
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(20, 60);

			uindex = m_vPlayersStatUI[i].size();			// 2 - mp
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 2].get(), textures[tindex + 2].get()));
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(25, 65);

			uindex = m_vPlayersStatUI[i].size();			// 3 ~ 5 buff
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 3].get()));
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(20, 100);
			uindex = m_vPlayersStatUI[i].size();			// 3 ~ 5 buff
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 4].get()));
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(20, 100);
			uindex = m_vPlayersStatUI[i].size();			// 3 ~ 5 buff
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 5].get()));
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(20, 100);
		}
		else {
			uindex = m_vPlayersStatUI[i].size();			// 0 - hpbar
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex].get()));
			m_vPlayersStatUI[i][uindex]->setScale(0.8);
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(88, 100 + 50 + (otherPlayer * 80));

			uindex = m_vPlayersStatUI[i].size();			// 1 - hp
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 3].get()));
			m_vPlayersStatUI[i][uindex]->setColor(1.0, 0.0, 0.0, 1.0);
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(92, 100 + 50 + (otherPlayer * 80) + 4);

			uindex = m_vPlayersStatUI[i].size();			// 2 - mp bar
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex + 1].get()));
			m_vPlayersStatUI[i][uindex]->setScale(0.8);
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(88, 138 + 50 + (otherPlayer * 80));

			uindex = m_vPlayersStatUI[i].size();			// 2 - mp
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 3].get(), textures[tindex + 2].get()));
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(92, 138 + 50 + (otherPlayer * 80) + 4);

			uindex = m_vPlayersStatUI[i].size();
			m_vPlayersStatUI[i].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 4].get(), textures[tindex + user_job[i] + 5].get()));
			m_vPlayersStatUI[i][uindex]->setPositionInViewport(20, 100 + 50 + (otherPlayer * 80));
			m_vPlayersStatUI[i][uindex]->setColor(1.0, 1.0, 1.0, 0.5);
			++otherPlayer;
		}
	}
	// =============================================================================

	// item ========================================================================
	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 140, 175));

	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item0.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item1.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item2.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item3.dds"));

	for (int i = 0; i < 4; ++i) {
		uindex = m_vItemUIs.size();
		m_vItemUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + i].get()));
		m_vItemUIs[uindex]->setColor(0.2 * (i + 1), 0.3, 0.2 * (i + 1), 1.0);
		m_vItemUIs[uindex]->setPositionInViewport(20, 525);
		m_vItemUIs[uindex]->setRenderState(false);
	}
	m_vItemUIs[0]->setRenderState(true);

	// =============================================================================

	// skills ======================================================================

	coolTime[0] = 5.0f; coolTime[1] = 10.0f; coolTime[2] = 20.0f;

	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 100, 100));

	tindex = textures.size();
	switch (job) {
	case JOB_MAGE:
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician0.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician1.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician2.dds"));
		break;
	case JOB_WARRIOR:
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Warrior0.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Warrior1.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Warrior2.dds"));
		break;
	case JOB_HEALER:
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Buffer0.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Buffer1.dds"));
		textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Buffer2.dds"));
		break;
	}
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_MP_Less.dds"));
	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + i].get()));
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
		m_vSkillUIs[uindex]->setColor(0.0, 0.0, 0.0, 0.5);
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 3].get()));
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}
}

// ==========================================================================

void CRaytracingParticleTestScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer)
{
	m_pOutputBuffer = outputBuffer;
	// CreateUISetup
	CreateOrthoMatrixBuffer();
	CreateRTVDSV();

	CreateParticleRS();
	CreateOnePath();
	CreateTwoPath();

	// Create Global & Local Root Signature
	//CreateRootSignature();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	/*m_pRaytracingPipeline = std::make_unique<CRayTracingPipeline>();
	m_pRaytracingPipeline->Setup(1 + 2 + 1 + 2 + 1 + 1);
	m_pRaytracingPipeline->AddLibrarySubObject(compiledShader, std::size(compiledShader));
	m_pRaytracingPipeline->AddHitGroupSubObject(L"HitGroup", L"RadianceClosestHit", L"RadianceAnyHit");
	m_pRaytracingPipeline->AddHitGroupSubObject(L"ShadowHit", L"ShadowClosestHit", L"ShadowAnyHit");
	m_pRaytracingPipeline->AddShaderConfigSubObject(8, 20);
	m_pRaytracingPipeline->AddLocalRootAndAsoociationSubObject(m_pLocalRootSignature.Get());
	m_pRaytracingPipeline->AddGlobalRootSignatureSubObject(m_pGlobalRootSignature.Get());
	m_pRaytracingPipeline->AddPipelineConfigSubObject(6);
	m_pRaytracingPipeline->MakePipelineState();*/

	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	m_pResourceManager->SetUp(3);
	// Object File Read ========================================	! !
	m_pResourceManager->AddResourceFromFile(L"src\\model\\City.bin", "src\\texture\\City\\");
	// Light Read
	m_pResourceManager->AddLightsFromFile(L"src\\Light\\LightingV2.bin");
	m_pResourceManager->ReadyLightBufferContent();
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<CParticle>>& particles = m_pResourceManager->getParticleList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// Create Normal Object & skinning Object Copy ========================================


	particles.emplace_back(std::make_unique<CRaytracingParticle>());
	particles[0]->setOnePathPipeline(m_OnePathPS);
	particles[0]->setTwoPathPipeline(m_TwoPathPS);
	particles[0]->ParticleSetting(0.0f, 3.0f);
	Material pmaterial{};
	pmaterial.m_bHasAlbedoColor = pmaterial.m_bHasAlbedoMap = true;
	pmaterial.m_xmf4AlbedoColor = XMFLOAT4(1.0, 1.0, 1.0, 1.0);
	pmaterial.m_nAlbedoMapIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Particle\\GreenCross.dds"));
	particles[0]->setMaterial(pmaterial);
	particles[0]->setPosition(XMFLOAT3(0.0, 0.0, 0.0));


	// cubeMap Ready
	m_nSkyboxIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\WinterLandSky2.dds", true));
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();
	m_pResourceManager->PrepareObject();	// Ready OutputBuffer to  SkinningObject


	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// Copy(normalObject) & SetPreMatrix ===============================


	// ==============================================================================

	// Camera Setting ==============================================================
	
	// ==========================================================================

	// AccelerationStructure
	m_pAccelerationStructureManager = std::make_unique<CAccelerationStructureManager>();
	m_pAccelerationStructureManager->Setup(m_pResourceManager.get(), 1);
	m_pAccelerationStructureManager->InitBLAS();
	m_pAccelerationStructureManager->InitTLAS();

}

void CRaytracingParticleTestScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
		switch (wParam) {
		case 'N':
			m_pCamera->toggleNormalMapping();
			break;
		case 'M':
			m_pCamera->toggleAlbedoColor();
			break;
		case 'B':
			m_pCamera->toggleReflection();
			break;
		case '9': {
			auto& particle = m_pResourceManager->getParticleList();
			particle[0]->Start();
		}
			break;
		case '0': {
			auto& particle = m_pResourceManager->getParticleList();
			particle[0]->Stop();
		}
			break;
		}
		break;
	case WM_KEYUP:
		break;
	}
}

void CRaytracingParticleTestScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_LBUTTONDOWN:
		m_bHold = true;
		GetCursorPos(&oldCursor);
		break;
	case WM_LBUTTONUP:
		m_bHold = false;
		break;
	case WM_MOUSEMOVE:
	{
		POINT cursorpos;
		if (m_bHold) {
			GetCursorPos(&cursorpos);
			m_pCamera->Rotate(cursorpos.x - oldCursor.x, cursorpos.y - oldCursor.y);
			SetCursorPos(oldCursor.x, oldCursor.y);
		}
		break;
	}
	}
}

void CRaytracingParticleTestScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);

	bool shiftDown = false;
	if (keyBuffer[VK_SHIFT] & 0x80)
		shiftDown = true;
	if (keyBuffer['W'] & 0x80)
		m_pCamera->Move(0, fElapsedTime, shiftDown);
	if (keyBuffer['S'] & 0x80)
		m_pCamera->Move(5, fElapsedTime, shiftDown);
	if (keyBuffer['D'] & 0x80)
		m_pCamera->Move(3, fElapsedTime, shiftDown);
	if (keyBuffer['A'] & 0x80)
		m_pCamera->Move(4, fElapsedTime, shiftDown);
	if (keyBuffer[VK_SPACE] & 0x80)
		m_pCamera->Move(1, fElapsedTime, shiftDown);
	if (keyBuffer[VK_CONTROL] & 0x80)
		m_pCamera->Move(2, fElapsedTime, shiftDown);
}

void CRaytracingParticleTestScene::UpdateObject(float fElapsedTime)
{
	// compute shader & rootSignature set
	g_DxResource.cmdList->SetPipelineState(m_pAnimationComputeShader.Get());
	g_DxResource.cmdList->SetComputeRootSignature(m_pComputeRootSignature.Get());

	m_pResourceManager->UpdateSkinningMesh(fElapsedTime);
	// particle update
	// SetRootSignature
	// particle update

	g_DxResource.cmdList->SetGraphicsRootSignature(m_ParticleRS.Get());
	m_pCamera->SetElapsedTimeAndShader(fElapsedTime, 0);

	m_pResourceManager->UpdateParticles(fElapsedTime);

	//Flush();
	// Skinning Object BLAS ReBuild
	m_pResourceManager->ReBuildBLAS();

	m_pResourceManager->UpdateWorldMatrix();


	m_pCamera->UpdateViewMatrix();
	m_pAccelerationStructureManager->UpdateScene(m_pCamera->getEye());
}

void CRaytracingParticleTestScene::Render()
{
	m_pCamera->SetShaderVariable();
	m_pAccelerationStructureManager->SetScene();
	m_pResourceManager->SetLights();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	g_DxResource.cmdList->SetComputeRootDescriptorTable(4, textures[m_nSkyboxIndex]->getView()->GetGPUDescriptorHandleForHeapStart());

	D3D12_DISPATCH_RAYS_DESC raydesc{};
	raydesc.Depth = 1;
	raydesc.Width = DEFINED_UAV_BUFFER_WIDTH;
	raydesc.Height = DEFINED_UAV_BUFFER_HEIGHT;

	raydesc.RayGenerationShaderRecord.StartAddress = m_pShaderBindingTable->getRayGenTable()->GetGPUVirtualAddress();
	raydesc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	raydesc.MissShaderTable.StartAddress = m_pShaderBindingTable->getMissTable()->GetGPUVirtualAddress();
	raydesc.MissShaderTable.SizeInBytes = m_pShaderBindingTable->getMissSize();
	raydesc.MissShaderTable.StrideInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	raydesc.HitGroupTable.StartAddress = m_pShaderBindingTable->getHitGroupTable()->GetGPUVirtualAddress();
	raydesc.HitGroupTable.SizeInBytes = m_pShaderBindingTable->getHitGroupSize();
	raydesc.HitGroupTable.StrideInBytes = m_pShaderBindingTable->getHitGroupStride();

	g_DxResource.cmdList->DispatchRays(&raydesc);
}

void CRaytracingParticleTestScene::CreateParticleRS()
{
	D3D12_ROOT_PARAMETER rp{};
	rp.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rp.Descriptor.ShaderRegister = 0;
	rp.Descriptor.RegisterSpace = 0;

	D3D12_ROOT_SIGNATURE_DESC desc{};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
	desc.NumParameters = 1;
	desc.NumStaticSamplers = 0;
	desc.pParameters = &rp;
	desc.pStaticSamplers = nullptr;

	ID3DBlob* pBlob{};
	D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
	g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_ParticleRS.GetAddressOf()));
	pBlob->Release();
}

void CRaytracingParticleTestScene::CreateOnePath()
{
	ID3DBlob* pd3dVBlob{ nullptr };
	ID3DBlob* pd3dGBlob{ nullptr };
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineState{};
	d3dPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineState.pRootSignature = m_ParticleRS.Get();

	D3D12_INPUT_ELEMENT_DESC ldesc[4]{};
	ldesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[1] = { "DIRECTION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[2] = { "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[3] = { "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3dPipelineState.InputLayout.pInputElementDescs = ldesc;
	d3dPipelineState.InputLayout.NumElements = 4;

	d3dPipelineState.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dPipelineState.DepthStencilState.StencilEnable = FALSE;

	d3dPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	d3dPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	d3dPipelineState.RasterizerState.AntialiasedLineEnable = FALSE;
	d3dPipelineState.RasterizerState.FrontCounterClockwise = FALSE;
	d3dPipelineState.RasterizerState.MultisampleEnable = FALSE;
	d3dPipelineState.RasterizerState.DepthClipEnable = FALSE;

	d3dPipelineState.BlendState.AlphaToCoverageEnable = FALSE;
	d3dPipelineState.BlendState.IndependentBlendEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendEnable = TRUE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dPipelineState.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	d3dPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineState.NumRenderTargets = 0;
	d3dPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineState.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	d3dPipelineState.SampleDesc.Count = 1;
	d3dPipelineState.SampleMask = UINT_MAX;

	//ID3DBlob* errorb;
	D3DCompileFromFile(L"ParticleShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &pd3dVBlob, nullptr);
	//OutputDebugStringA((char*)errorb->GetBufferPointer());
	d3dPipelineState.VS.BytecodeLength = pd3dVBlob->GetBufferSize();
	d3dPipelineState.VS.pShaderBytecode = pd3dVBlob->GetBufferPointer();

	D3DCompileFromFile(L"ParticleShader.hlsl", nullptr, nullptr, "GS_Buff_OnePath", "gs_5_1", 0, 0, &pd3dGBlob, nullptr);
	d3dPipelineState.GS.BytecodeLength = pd3dGBlob->GetBufferSize();
	d3dPipelineState.GS.pShaderBytecode = pd3dGBlob->GetBufferPointer();

	D3D12_SO_DECLARATION_ENTRY soEntry[4]{};
	soEntry[0] = { 0, "POSITION", 0, 0, 3, 0 };
	soEntry[1] = { 0, "DIRECTION", 0, 0, 3, 0 };
	soEntry[2] = { 0, "LIFETIME", 0, 0, 1, 0 };
	soEntry[3] = { 0, "TYPE", 0, 0, 1, 0 };

	UINT stride[1] = { sizeof(ParticleVertex) };

	d3dPipelineState.StreamOutput.NumEntries = 4;
	d3dPipelineState.StreamOutput.pSODeclaration = soEntry;
	d3dPipelineState.StreamOutput.NumStrides = 1;
	d3dPipelineState.StreamOutput.pBufferStrides = stride;
	d3dPipelineState.StreamOutput.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;

	g_DxResource.device->CreateGraphicsPipelineState(&d3dPipelineState, IID_PPV_ARGS(m_OnePathPS.GetAddressOf()));

	if (pd3dVBlob)
		pd3dVBlob->Release();
	if (pd3dGBlob)
		pd3dGBlob->Release();
}

void CRaytracingParticleTestScene::CreateTwoPath()
{
	ID3DBlob* pd3dVBlob{ nullptr };
	ID3DBlob* pd3dGBlob{ nullptr };
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineState{};
	d3dPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineState.pRootSignature = m_ParticleRS.Get();

	D3D12_INPUT_ELEMENT_DESC ldesc[4]{};
	ldesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[1] = { "DIRECTION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[2] = { "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[3] = { "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3dPipelineState.InputLayout.pInputElementDescs = ldesc;
	d3dPipelineState.InputLayout.NumElements = 4;

	d3dPipelineState.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dPipelineState.DepthStencilState.StencilEnable = FALSE;

	d3dPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	d3dPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	d3dPipelineState.RasterizerState.AntialiasedLineEnable = FALSE;
	d3dPipelineState.RasterizerState.FrontCounterClockwise = FALSE;
	d3dPipelineState.RasterizerState.MultisampleEnable = FALSE;
	d3dPipelineState.RasterizerState.DepthClipEnable = FALSE;

	d3dPipelineState.BlendState.AlphaToCoverageEnable = FALSE;
	d3dPipelineState.BlendState.IndependentBlendEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendEnable = TRUE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dPipelineState.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	d3dPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineState.NumRenderTargets = 0;
	d3dPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	d3dPipelineState.SampleDesc.Count = 1;
	d3dPipelineState.SampleMask = UINT_MAX;

	D3DCompileFromFile(L"ParticleShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &pd3dVBlob, nullptr);
	d3dPipelineState.VS.BytecodeLength = pd3dVBlob->GetBufferSize();
	d3dPipelineState.VS.pShaderBytecode = pd3dVBlob->GetBufferPointer();

	D3DCompileFromFile(L"ParticleShader.hlsl", nullptr, nullptr, "GS_Buff_TwoPath", "gs_5_1", 0, 0, &pd3dGBlob, nullptr);
	d3dPipelineState.GS.BytecodeLength = pd3dGBlob->GetBufferSize();
	d3dPipelineState.GS.pShaderBytecode = pd3dGBlob->GetBufferPointer();

	D3D12_SO_DECLARATION_ENTRY soEntry[3]{};
	soEntry[0] = { 0, "POSITION", 0, 0, 3, 0 };
	soEntry[1] = { 1, "TEXCOORD", 0, 0, 2, 1 };
	soEntry[2] = { 2, "COLOR", 0, 0, 4, 2 };

	UINT stride[3] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2), sizeof(XMFLOAT4)};

	d3dPipelineState.StreamOutput.NumEntries = 3;
	d3dPipelineState.StreamOutput.pSODeclaration = soEntry;
	d3dPipelineState.StreamOutput.NumStrides = 3;
	d3dPipelineState.StreamOutput.pBufferStrides = stride;
	d3dPipelineState.StreamOutput.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;

	g_DxResource.device->CreateGraphicsPipelineState(&d3dPipelineState, IID_PPV_ARGS(m_TwoPathPS.GetAddressOf()));

	if (pd3dVBlob)
		pd3dVBlob->Release();
	if (pd3dGBlob)
		pd3dGBlob->Release();
}

void CRaytracingParticleTestScene::PostProcess()
{
	m_pResourceManager->PostProcess();
}

void CRaytracingParticleTestScene::TextRender()
{
	//m_pTextManager->Render();
}


// =====================================================


void UITestScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer)
{
	m_pOutputBuffer = outputBuffer;

	CreateRTVDSV();
	CreateRootSignature();
	CreatePipelineState();

	CreateOrthoMatrixBuffer();

	size_t mindex{};
	size_t tindex{};
	size_t uindex{};

	m_pResourceManager = std::make_unique<CResourceManager>();

	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();

	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT));
	m_background = std::make_unique<UIObject>(1, 2, meshes[mindex].get());
	m_background->setColor(1.0, 1.0, 1.0, 1.0); m_background->setPositionInViewport(0, 0);

	// status UI ===================================================================
	maxHPs[0] = 1200; maxHPs[1] = 1000; maxHPs[2] = 800;
	cHPs[0] = 1200; cHPs[1] = 800; cHPs[2] = 730;

	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 30, 30));		// buff icon
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 340, 28));		// hp/mp bar
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 330, 18));		// hp/mp

	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 255, 12));		// coop hp/mp

	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_HPbar.dds"));	// HPbar
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MPbar.dds"));	// MPbar
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_MP.dds"));	// MP

	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Buff0.dds"));	// buff0
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Buff1.dds"));	// buff1
	{
		uindex = m_vStatusUIs[0].size();			// 0 - hpbar
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 20);

		uindex = m_vStatusUIs[0].size();			// 1 - hp
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 2].get()));
		m_vStatusUIs[0][uindex]->setColor(1.0, 0.0, 0.0, 1.0);
		m_vStatusUIs[0][uindex]->setPositionInViewport(25, 25);

		uindex = m_vStatusUIs[0].size();			// 2 - mp bar
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get(), textures[tindex + 1].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 60);

		uindex = m_vStatusUIs[0].size();			// 2 - mp
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 2].get(), textures[tindex + 2].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(25, 65);

		m_buffpixelHeight[0] = 100;
		uindex = m_vStatusUIs[0].size();			// 3 ~ 5 buff
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 3].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 100);
		uindex = m_vStatusUIs[0].size();			// 3 ~ 5 buff
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 4].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 100);
		uindex = m_vStatusUIs[0].size();			// 3 ~ 5 buff
		m_vStatusUIs[0].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
		m_vStatusUIs[0][uindex]->setPositionInViewport(20, 100);
		m_vStatusUIs[0][uindex]->setColor(0.7, 1.0, 0.0, 1.0);
	}

	//for (int i = 0; i < 2; ++i) {
	//	uindex = m_vStatusUIs[i + 1].size();			// 1 - hp
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get()));
	//	m_vStatusUIs[i + 1][uindex]->setPositionInViewport(15, (i * 115) + 150 + 15);
	//	m_vStatusUIs[i + 1][uindex]->setColor(1.0, 0.0, 0.0, 1.0);

	//	uindex = m_vStatusUIs[i + 1].size();			// 2 - mp
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex + 1].get()));
	//	m_vStatusUIs[i + 1][uindex]->setPositionInViewport(15, (i * 115) + 150 + 15 + 30);
	//	m_vStatusUIs[i + 1][uindex]->setColor(0.0, 0.0, 1.0, 1.0);

	//	m_buffpixelHeight[i + 1] = (i * 115) + 150 + 15 + 30 + 30;
	//	uindex = m_vStatusUIs[i + 1].size();			// 3 ~ 5 buff
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	//	m_vStatusUIs[i + 1][uindex]->setScale(0.75);
	//	m_vStatusUIs[i + 1][uindex]->setColor(0.0, 1.0, 1.0, 1.0);
	//	uindex = m_vStatusUIs[i + 1].size();			// 3 ~ 5 buff
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	//	m_vStatusUIs[i + 1][uindex]->setScale(0.75);
	//	m_vStatusUIs[i + 1][uindex]->setColor(1.0, 0.5, 1.0, 1.0);
	//	uindex = m_vStatusUIs[i + 1].size();			// 3 ~ 5 buff
	//	m_vStatusUIs[i + 1].emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	//	m_vStatusUIs[i + 1][uindex]->setScale(0.75);
	//	m_vStatusUIs[i + 1][uindex]->setColor(0.7, 1.0, 0.0, 1.0);
	//}
	// =============================================================================

	// item ========================================================================
	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 140, 175));

	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item0.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item1.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item2.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Item3.dds"));

	for (int i = 0; i < 4; ++i) {
		uindex = m_vItemUIs.size();
		m_vItemUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + i].get()));
		m_vItemUIs[uindex]->setColor(0.2 * (i + 1), 0.3, 0.2 * (i + 1), 1.0);
		m_vItemUIs[uindex]->setPositionInViewport(20, 525);
		m_vItemUIs[uindex]->setRenderState(false);
	}
	m_vItemUIs[0]->setRenderState(true);

	uindex = m_vItemUIs.size();
	m_vItemUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
	m_vItemUIs[uindex]->setColor(0.0, 0.0, 0.0, 0.0);
	m_vItemUIs[uindex]->setPositionInViewport(20, 525);
	// =============================================================================

	// skills ======================================================================

	coolTime[0] = 5.0f; coolTime[1] = 10.0f; coolTime[2] = 20.0f;

	mindex = meshes.size();
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(), 100, 100));

	tindex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician0.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician1.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_Magician2.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\UI\\InGame\\UI_Skill_MP_Less.dds"));
	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + i].get()));
		//m_vSkillUIs[uindex]->setColor(1.0, 0.5, 0.5, 1.0);
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get()));
		m_vSkillUIs[uindex]->setColor(0.0, 0.0, 0.0, 0.5);
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	for (int i = 0; i < 3; ++i) {
		uindex = m_vSkillUIs.size();
		m_vSkillUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[mindex].get(), textures[tindex + 3].get()));
		m_vSkillUIs[uindex]->setPositionInViewport(i * 110 + 940, 600);
	}

	// =============================================================================
}

void UITestScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN: {
		switch (wParam) {
		case 'U':
			cHPs[0] += 10;
			if (maxHPs[0] < cHPs[0])
				cHPs[0] = maxHPs[0];
			break;
		case 'J':
			cHPs[0] -= 10;
			if (0 > cHPs[0])
				cHPs[0] = 0;
			break;
		case '1':
			m_BuffState[0][0] = !m_BuffState[0][0];
			break;
		case '2':
			m_BuffState[0][1] = !m_BuffState[0][1];
			break;
		case '3':
			m_BuffState[0][2] = !m_BuffState[0][2];
			break;
		case '0':
			cMP = 100;
			break;
		case 'Z':
			m_vItemUIs[cItem]->setRenderState(false);
			--cItem;
			if (cItem < 0) cItem = 3;
			m_vItemUIs[cItem]->setRenderState(true);
			break;
		case 'C':
			m_vItemUIs[cItem]->setRenderState(false);
			++cItem;
			if (cItem > 3) cItem = 0;
			m_vItemUIs[cItem]->setRenderState(true);
			break;
		case 'X':
			itemUse = !itemUse;
			break;
		case 'Q':
			if (cMP >= 30 && curCTime[0] <= 0) {
				cMP -= 30;
				curCTime[0] = coolTime[0];
			}
			break;
		case 'E':
			if (cMP >= 40 && curCTime[1] <= 0) {
				cMP -= 40;
				curCTime[1] = coolTime[1];
			}
			break;
		case 'R':
			if (cMP >= 60 && curCTime[2] <= 0) {
				cMP -= 60;
				curCTime[2] = coolTime[2];
			}
			break;
		}
		break;
	}
	case WM_KEYUP:
		break;
	}
}
void UITestScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_LBUTTONDOWN: {
		int mx = LOWORD(lParam);
		int my = HIWORD(lParam);

		break;
	}
	case WM_LBUTTONUP: {
		break;
	}
	case WM_MOUSEMOVE: {
		break;
	}
	}
}

void UITestScene::CreateRootSignature()
{
	D3D12_DESCRIPTOR_RANGE tRange{};
	tRange.BaseShaderRegister = 0;
	tRange.NumDescriptors = 1;
	tRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	D3D12_ROOT_PARAMETER params[3]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[0].Descriptor.RegisterSpace = 0;
	params[0].Descriptor.ShaderRegister = 0;

	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[1].Descriptor.RegisterSpace = 0;
	params[1].Descriptor.ShaderRegister = 1;

	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 1;
	params[2].DescriptorTable.pDescriptorRanges = &tRange;

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};								// s0
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rtDesc{};
	rtDesc.NumParameters = 3;
	rtDesc.NumStaticSamplers = 1;
	rtDesc.pParameters = params;
	rtDesc.pStaticSamplers = &samplerDesc;
	rtDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* pBlob{};
	D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
	g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pGlobalRootSignature.GetAddressOf()));
	pBlob->Release();
}
void UITestScene::CreatePipelineState()
{
	ID3DBlob* pd3dVBlob{ nullptr };
	ID3DBlob* pd3dPBlob{ nullptr };
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineState{};
	d3dPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineState.pRootSignature = m_pGlobalRootSignature.Get();

	D3D12_INPUT_ELEMENT_DESC ldesc[3]{};
	ldesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3dPipelineState.InputLayout.pInputElementDescs = ldesc;
	d3dPipelineState.InputLayout.NumElements = 3;

	d3dPipelineState.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dPipelineState.DepthStencilState.StencilEnable = FALSE;

	d3dPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	d3dPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	d3dPipelineState.RasterizerState.AntialiasedLineEnable = FALSE;
	d3dPipelineState.RasterizerState.FrontCounterClockwise = FALSE;
	d3dPipelineState.RasterizerState.MultisampleEnable = FALSE;
	d3dPipelineState.RasterizerState.DepthClipEnable = FALSE;

	d3dPipelineState.BlendState.AlphaToCoverageEnable = FALSE;
	d3dPipelineState.BlendState.IndependentBlendEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendEnable = TRUE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dPipelineState.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	d3dPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineState.NumRenderTargets = 1;
	d3dPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineState.SampleDesc.Count = 1;
	d3dPipelineState.SampleMask = UINT_MAX;

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &pd3dVBlob, nullptr);
	d3dPipelineState.VS.BytecodeLength = pd3dVBlob->GetBufferSize();
	d3dPipelineState.VS.pShaderBytecode = pd3dVBlob->GetBufferPointer();

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", 0, 0, &pd3dPBlob, nullptr);
	d3dPipelineState.PS.BytecodeLength = pd3dPBlob->GetBufferSize();
	d3dPipelineState.PS.pShaderBytecode = pd3dPBlob->GetBufferPointer();

	g_DxResource.device->CreateGraphicsPipelineState(&d3dPipelineState, IID_PPV_ARGS(m_UIPipelineState.GetAddressOf()));

	if (pd3dVBlob)
		pd3dVBlob->Release();
	if (pd3dPBlob)
		pd3dPBlob->Release();
}

void UITestScene::UpdateObject(float fElapsedTime)
{
	int buffstart = 20; int bstride = 40;
	for (int i = 0; i < m_numUser; ++i) {
		int t{};
		// hp/mp
		m_vStatusUIs[i][1]->setScaleX(cHPs[i] / maxHPs[i]);
		if (i == 0) {
			m_vStatusUIs[i][3]->setScaleXWithUV(cMP / maxMP);
		}
		if (i > 0) {
			buffstart = 15; bstride = 30;
		}
		for (int j = 0; j < 3; ++j) {
			if (m_BuffState[i][j]) {
				m_vStatusUIs[i][j + 4]->setRenderState(true);
				m_vStatusUIs[i][j + 4]->setPositionInViewport(buffstart + (t * bstride), m_buffpixelHeight[i]);
				++t;
			}
			else {
				m_vStatusUIs[i][j + 4]->setRenderState(false);
			}
		}
	}

	if (itemUse)
		m_vItemUIs[4]->setColor(0.0, 0.0, 0.0, 0.0);
	else
		m_vItemUIs[4]->setColor(0.0, 0.0, 0.0, 0.5);
	{
		if (cMP < 30)
			m_vSkillUIs[6]->setRenderState(true);
		else
			m_vSkillUIs[6]->setRenderState(false);

		if (cMP < 40)
			m_vSkillUIs[7]->setRenderState(true);
		else
			m_vSkillUIs[7]->setRenderState(false);

		if (cMP < 60)
			m_vSkillUIs[8]->setRenderState(true);
		else
			m_vSkillUIs[8]->setRenderState(false);

		for (int i = 0; i < 3; ++i) {
			curCTime[i] -= fElapsedTime;
			if (curCTime[i] < 0) curCTime[i] = 0.0f;
			m_vSkillUIs[i + 3]->setScaleY(curCTime[i] / coolTime[i]);
		}
	}

}
void UITestScene::Render()
{
	ID3D12GraphicsCommandList4* cmdList = g_DxResource.cmdList;
	auto barrier = [&](ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		{
			D3D12_RESOURCE_BARRIER resBarrier{};
			resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resBarrier.Transition.pResource = pResource;
			resBarrier.Transition.StateBefore = before;
			resBarrier.Transition.StateAfter = after;
			resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmdList->ResourceBarrier(1, &resBarrier);
		};

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_VIEWPORT vv{};
	vv.Width = DEFINED_UAV_BUFFER_WIDTH; vv.Height = DEFINED_UAV_BUFFER_HEIGHT; vv.MinDepth = 0.0f; vv.MaxDepth = 1.0f;
	cmdList->RSSetViewports(1, &vv);
	D3D12_RECT ss{ 0, 0, DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT };
	cmdList->RSSetScissorRects(1, &ss);
	cmdList->OMSetRenderTargets(1, &m_RTV->GetCPUDescriptorHandleForHeapStart(), FALSE, &m_DSV->GetCPUDescriptorHandleForHeapStart());
	cmdList->SetGraphicsRootSignature(m_pGlobalRootSignature.Get());
	cmdList->SetPipelineState(m_UIPipelineState.Get());
	cmdList->SetGraphicsRootConstantBufferView(0, m_cameraCB->GetGPUVirtualAddress());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_background->Render();

	for (short i = 0; i < m_numUser; ++i) {
		for (auto& p : m_vStatusUIs[i])
			p->Render();
	}

	for (auto& p : m_vItemUIs)
		p->Render();

	for (auto& p : m_vSkillUIs)
		p->Render();

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}

// ==============================================================================


void CRaytracingCollisionTestScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer)
{
	m_pOutputBuffer = outputBuffer;
	// CreateUISetup
	CreateOrthoMatrixBuffer();
	CreateRTVDSV();
	CreateUIRootSignature();
	CreateUIPipelineState();

	// Create Global & Local Root Signature
	//CreateRootSignature();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	/*m_pRaytracingPipeline = std::make_unique<CRayTracingPipeline>();
	m_pRaytracingPipeline->Setup(1 + 2 + 1 + 2 + 1 + 1);
	m_pRaytracingPipeline->AddLibrarySubObject(compiledShader, std::size(compiledShader));
	m_pRaytracingPipeline->AddHitGroupSubObject(L"HitGroup", L"RadianceClosestHit", L"RadianceAnyHit");
	m_pRaytracingPipeline->AddHitGroupSubObject(L"ShadowHit", L"ShadowClosestHit", L"ShadowAnyHit");
	m_pRaytracingPipeline->AddShaderConfigSubObject(8, 20);
	m_pRaytracingPipeline->AddLocalRootAndAsoociationSubObject(m_pLocalRootSignature.Get());
	m_pRaytracingPipeline->AddGlobalRootSignatureSubObject(m_pGlobalRootSignature.Get());
	m_pRaytracingPipeline->AddPipelineConfigSubObject(6);
	m_pRaytracingPipeline->MakePipelineState();*/

	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	m_pResourceManager->SetUp(3);
	// Object File Read ========================================	! !
	m_pResourceManager->AddResourceFromFile(L"src\\model\\City.bin", "src\\texture\\City\\");
	//m_pResourceManager->AddResourceFromFile(L"src\\model\\WinterLand3.bin", "src\\texture\\Map\\");
	//m_pResourceManager->AddResourceFromFile(L"src\\model\\Cave.bin", "src\\texture\\Map\\");

	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\Greycloak\\", JOB_MAGE);
	CreateMageCharacter();
	//CreateWarriorCharacter();
	//CreatePriestCharacter();
	m_pPlayer = std::make_unique<CPlayer>(m_vPlayers[m_vPlayers.size() - 1].get(), m_pCamera);

	Create3StageBoss();
	m_pMonster = std::make_unique<CMonster>(m_vMonsters[m_vMonsters.size() - 1].get());

	// Light Read
	m_pResourceManager->AddLightsFromFile(L"src\\Light\\LightingV2.bin");
	//m_pResourceManager->AddLightsFromFile(L"src\\Light\\LightingCave.bin");
	m_pResourceManager->ReadyLightBufferContent();
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// Create Normal Object & skinning Object Copy ========================================

	for (auto& o : skinned[1]->getObjects()) {
		for (auto& ma : o->getMaterials())
			ma.m_bHasEmissiveColor = false;
	}

	UINT finalindex = normalObjects.size();
	UINT finalmesh = meshes.size();

	// terrian
	//m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\terrain.raw", 2049, 2049, XMFLOAT3(1.0f, 0.0312f, 1.0f));
	////m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\terrainCave.raw", 2049, 2049, XMFLOAT3(1.0f, 1.0f, 1.0f));
	//meshes.emplace_back(std::make_unique<Mesh>(m_pHeightMap.get(), "terrain"));
	//normalObjects.emplace_back(std::make_unique<CGameObject>());
	//normalObjects[normalObjects.size() - 1]->SetMeshIndex(meshes.size() - 1);

	//normalObjects[normalObjects.size() - 1]->SetInstanceID(10);
	//normalObjects[normalObjects.size() - 1]->getMaterials().emplace_back();
	//normalObjects[normalObjects.size() - 1]->SetPosition(XMFLOAT3(-1024.0, 0.0, -1024.0));


	//textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02_NORM.dds"));
	//textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02_MNS.dds"));
	//auto p = std::find_if(normalObjects.begin(), normalObjects.end(), [](std::unique_ptr<CGameObject>& p) {
	//	return p->getFrameName() == "Water";
	//	});
	//if (p != normalObjects.end()) {
	//	(*p)->SetInstanceID(2);
	//	(*p)->getMaterials().emplace_back();
	//	Material& mt = (*p)->getMaterials()[0];
	//	mt.m_bHasAlbedoColor = true; mt.m_xmf4AlbedoColor = XMFLOAT4(0.1613118, 0.2065666, 0.2358491, 0.2);
	//	mt.m_bHasMetallicMap = true; mt.m_nMetallicMapIndex = textures.size() - 1;
	//	mt.m_bHasNormalMap = true; mt.m_nNormalMapIndex = textures.size() - 2;

	//	void* tempptr{};
	//	std::vector<XMFLOAT2> tex0 = meshes[(*p)->getMeshIndex()]->getTex0();
	//	for (XMFLOAT2& xmf : tex0) {
	//		xmf.x *= 10.0f; xmf.y *= 10.0f;
	//	}
	//	meshes[(*p)->getMeshIndex()]->getTexCoord0Buffer()->Map(0, nullptr, &tempptr);
	//	memcpy(tempptr, tex0.data(), sizeof(XMFLOAT2) * tex0.size());
	//	meshes[(*p)->getMeshIndex()]->getTexCoord0Buffer()->Unmap(0, nullptr);
	//}

	//PrepareTerrainTexture();

	// cubeMap Ready
	m_nSkyboxIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\WinterLandSky2.dds", true));
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();
	m_pResourceManager->PrepareObject();	// Ready OutputBuffer to  SkinningObject


	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// Copy(normalObject) & SetPreMatrix ===============================

	//skinned[0]->setPreTransform(2.5f, XMFLOAT3(90.0f, 0.0f, 0.0f), XMFLOAT3());
	skinned[0]->setPreTransform(2.5f, XMFLOAT3(), XMFLOAT3());
	//skinned[0]->SetPosition(XMFLOAT3(-72.5f, 67.0f, -998.0f));
	//skinned[0]->SetPosition(XMFLOAT3(-72.5f, 0.0f, -500.0f));
	skinned[0]->SetPosition(XMFLOAT3(-28.0f, 0.0f, -270.0f));
	//skinned[1]->setPreTransform(5.0f, XMFLOAT3(), XMFLOAT3());
	//skinned[1]->SetPosition(XMFLOAT3(-28.0f, 0.0f, -245.0f));
	//skinned[1]->Rotate(XMFLOAT3(0.0f, 180.0f, 0.0f));

	// ==============================================================================

	// Camera Setting ==============================================================
	//m_pCamera->SetTarget(skinned[0]->getObjects()[0].get());
	m_pCamera->SetTarget(skinned[0]->getObjects()[0].get());
	m_pCamera->SetHOffset(3.5f);
	m_pCamera->SetCameraLength(15.0f);
	// ==========================================================================

	// AccelerationStructure
	m_pAccelerationStructureManager = std::make_unique<CAccelerationStructureManager>();
	m_pAccelerationStructureManager->Setup(m_pResourceManager.get(), 1);
	m_pAccelerationStructureManager->InitBLAS();
	m_pAccelerationStructureManager->InitTLAS();

	// UISetup ========================================================================
	meshes.emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0, 0.0, 0.0), DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT));
	m_vUIs.emplace_back(std::make_unique<UIObject>(1, 2, meshes[meshes.size() - 1].get()));
	m_vUIs[m_vUIs.size() - 1]->setPositionInViewport(0, 0);
	m_vUIs[m_vUIs.size() - 1]->setColor(0.0, 0.0, 0.0, 1.0);
}

void CRaytracingCollisionTestScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
		switch (wParam) {
		case 'N':
			m_pCamera->toggleNormalMapping();
			break;
		case 'M':
			m_pCamera->toggleAlbedoColor();
			break;
		case 'B':
			m_pCamera->toggleReflection();
			break;
		case '9':
			m_pCamera->SetThirdPersonMode(false);
			break;
		case '0':
			m_pCamera->SetThirdPersonMode(true);
			break;
		case '8':
			if (m_nState == IS_GAMING) {
				startTime = 0.0f;
				m_nState = IS_FINISH;
			}
			break;
		}
		break;
	case WM_KEYUP:
		break;
	}
}

void CRaytracingCollisionTestScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	m_pPlayer->MouseProcess(hWnd, nMessage, wParam, lParam);
}

void CRaytracingCollisionTestScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);

	if (m_nState == IS_GAMING) {
		if (!m_pCamera->getThirdPersonState()) {
			bool shiftDown = false;
			if (keyBuffer[VK_SHIFT] & 0x80)
				shiftDown = true;
			if (keyBuffer['W'] & 0x80)
				m_pCamera->Move(0, fElapsedTime, shiftDown);
			if (keyBuffer['S'] & 0x80)
				m_pCamera->Move(5, fElapsedTime, shiftDown);
			if (keyBuffer['D'] & 0x80)
				m_pCamera->Move(3, fElapsedTime, shiftDown);
			if (keyBuffer['A'] & 0x80)
				m_pCamera->Move(4, fElapsedTime, shiftDown);
			if (keyBuffer[VK_SPACE] & 0x80)
				m_pCamera->Move(1, fElapsedTime, shiftDown);
			if (keyBuffer[VK_CONTROL] & 0x80)
				m_pCamera->Move(2, fElapsedTime, shiftDown);
		}
		else {
			m_pPlayer->ProcessInput(keyBuffer, fElapsedTime);
			m_pMonster->ProcessInput(keyBuffer, fElapsedTime);
		}
	}
}

void CRaytracingCollisionTestScene::CreateUIRootSignature()
{
	D3D12_DESCRIPTOR_RANGE tRange{};
	tRange.BaseShaderRegister = 0;
	tRange.NumDescriptors = 1;
	tRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

	D3D12_ROOT_PARAMETER params[3]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[0].Descriptor.RegisterSpace = 0;
	params[0].Descriptor.ShaderRegister = 0;

	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	params[1].Descriptor.RegisterSpace = 0;
	params[1].Descriptor.ShaderRegister = 1;

	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 1;
	params[2].DescriptorTable.pDescriptorRanges = &tRange;

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};								// s0
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC rtDesc{};
	rtDesc.NumParameters = 3;
	rtDesc.NumStaticSamplers = 1;
	rtDesc.pParameters = params;
	rtDesc.pStaticSamplers = &samplerDesc;
	rtDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* pBlob{};
	D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
	g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_UIRootSignature.GetAddressOf()));
	pBlob->Release();
}
void CRaytracingCollisionTestScene::CreateUIPipelineState()
{
	ID3DBlob* pd3dVBlob{ nullptr };
	ID3DBlob* pd3dPBlob{ nullptr };
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineState{};
	d3dPipelineState.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	d3dPipelineState.pRootSignature = m_UIRootSignature.Get();

	D3D12_INPUT_ELEMENT_DESC ldesc[3]{};
	ldesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	ldesc[2] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	d3dPipelineState.InputLayout.pInputElementDescs = ldesc;
	d3dPipelineState.InputLayout.NumElements = 3;

	d3dPipelineState.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineState.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	d3dPipelineState.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	d3dPipelineState.DepthStencilState.StencilEnable = FALSE;

	d3dPipelineState.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	d3dPipelineState.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	d3dPipelineState.RasterizerState.AntialiasedLineEnable = FALSE;
	d3dPipelineState.RasterizerState.FrontCounterClockwise = FALSE;
	d3dPipelineState.RasterizerState.MultisampleEnable = FALSE;
	d3dPipelineState.RasterizerState.DepthClipEnable = FALSE;

	d3dPipelineState.BlendState.AlphaToCoverageEnable = FALSE;
	d3dPipelineState.BlendState.IndependentBlendEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendEnable = TRUE;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dPipelineState.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dPipelineState.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dPipelineState.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dPipelineState.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	d3dPipelineState.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	d3dPipelineState.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineState.NumRenderTargets = 1;
	d3dPipelineState.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineState.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineState.SampleDesc.Count = 1;
	d3dPipelineState.SampleMask = UINT_MAX;

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "VSMain", "vs_5_1", 0, 0, &pd3dVBlob, nullptr);
	d3dPipelineState.VS.BytecodeLength = pd3dVBlob->GetBufferSize();
	d3dPipelineState.VS.pShaderBytecode = pd3dVBlob->GetBufferPointer();

	D3DCompileFromFile(L"UIShader.hlsl", nullptr, nullptr, "PSMain", "ps_5_1", 0, 0, &pd3dPBlob, nullptr);
	d3dPipelineState.PS.BytecodeLength = pd3dPBlob->GetBufferSize();
	d3dPipelineState.PS.pShaderBytecode = pd3dPBlob->GetBufferPointer();

	g_DxResource.device->CreateGraphicsPipelineState(&d3dPipelineState, IID_PPV_ARGS(m_UIPipelineState.GetAddressOf()));

	if (pd3dVBlob)
		pd3dVBlob->Release();
	if (pd3dPBlob)
		pd3dPBlob->Release();
}

void CRaytracingCollisionTestScene::CreateMageCharacter()
{
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\Greycloak\\", JOB_MAGE);
	m_vPlayers.emplace_back(std::make_unique<CPlayerMage>(
		m_pResourceManager->getSkinningObjectList()[m_pResourceManager->getSkinningObjectList().size() - 1].get(),
		m_pResourceManager->getAnimationManagers()[m_pResourceManager->getAnimationManagers().size() - 1].get(), false));

	// Create Mage's own objects and Set
	// ex) bullet, particle, barrier  etc...

	/*m_pResourceManager->getMeshList().emplace_back(std::make_unique<Mesh>(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), "box"));
	m_pResourceManager->getGameObjectList().emplace_back(std::make_unique<CGameObject>());
	m_pResourceManager->getGameObjectList().back()->SetMeshIndex(m_pResourceManager->getMeshList().size()-1);
	m_pResourceManager->getGameObjectList().back()->getMaterials().emplace_back();

	m_vPlayers.back()->GetBullets()[0].get()->setGameObject(m_pResourceManager->getGameObjectList().back().get());*/
}

void CRaytracingCollisionTestScene::CreateWarriorCharacter()
{
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\swordman_lv1.bin", "src\\texture\\Swordman\\", JOB_WARRIOR);
	m_vPlayers.emplace_back(std::make_unique<CPlayerWarrior>(
		m_pResourceManager->getSkinningObjectList()[m_pResourceManager->getSkinningObjectList().size() - 1].get(),
		m_pResourceManager->getAnimationManagers()[m_pResourceManager->getAnimationManagers().size() - 1].get(), false));
}

void CRaytracingCollisionTestScene::CreatePriestCharacter()
{
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Luna_Firemantle_33.bin", "src\\texture\\Luna\\", JOB_HEALER);
	m_vPlayers.emplace_back(std::make_unique<CPlayerPriest>(
		m_pResourceManager->getSkinningObjectList()[m_pResourceManager->getSkinningObjectList().size() - 1].get(),
		m_pResourceManager->getAnimationManagers()[m_pResourceManager->getAnimationManagers().size() - 1].get(), false));
}

void CRaytracingCollisionTestScene::Create3StageBoss()
{
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Gorhorrid.bin", "src\\texture\\Gorhorrid\\", MONSTER);
	m_vMonsters.emplace_back(std::make_unique<Stage3_Monster>(
		m_pResourceManager->getSkinningObjectList()[m_pResourceManager->getSkinningObjectList().size() - 1].get(),
		m_pResourceManager->getAnimationManagers()[m_pResourceManager->getAnimationManagers().size() - 1].get(), true));

	m_vMonsters[0]->getObject()->setPreTransform(5.0f, XMFLOAT3(), XMFLOAT3());
	m_vMonsters[0]->getObject()->SetPosition(XMFLOAT3(-28.0f, 0.0f, -245.0f));
	m_vMonsters[0]->getObject()->Rotate(XMFLOAT3(0.0f, 180.0f, 0.0f));
}

void CRaytracingCollisionTestScene::AttackCollision(const std::vector<std::unique_ptr<CPlayableCharacter>>& targets, const std::vector<std::unique_ptr<CPlayableCharacter>>& attackers)
{
	//스피어-박스
	/*for (const auto& target : targets) {
		if (target->IsOnceAttacked()) continue;
		for (const auto& targetBone : target->getObject()->getObjects()) {
			if (!(targetBone->getBoundingInfo() & 0x1100)) continue;
			BoundingSphere targetSphere = targetBone->getObjectSphere();
			BoundingSphere transformedTargetSphere;
			targetSphere.Transform(transformedTargetSphere, XMLoadFloat4x4(&targetBone->getWorldMatrix()));
			for (const auto& attacker : attackers) {
				if (!attacker->IsAttacking()) continue;
				for (const auto& attackerBone : attacker->getObject()->getObjects()) {
					if (!(attackerBone->getBoundingInfo() & 0x1000)) continue;
					BoundingOrientedBox attackerOBB = attackerBone->getObjectOBB();
					BoundingOrientedBox transformedAttackerOBB;
					attackerOBB.Transform(transformedAttackerOBB, XMLoadFloat4x4(&attackerBone->getWorldMatrix()));
					if (transformedAttackerOBB.Intersects(transformedTargetSphere)) {
						float damage = 0.0f;
						switch (attacker->getCurrentSkill()) {
						case 1: damage = 200.0f; break;
						case 2: damage = 400.0f; break;
						case 3: damage = 300.0f; break;
						}
						if (damage > 0.0f) {
							target->Attacked(damage);
						}
						return;
					}
				}
			}
		}
	}*/
	//스피어-스피어
	for (const auto& target : targets) {
		if (target->IsOnceAttacked()) continue;
		for (const auto& targetBone : target->getObject()->getObjects()) {
			if (!(targetBone->getBoundingInfo() & 0x1100)) continue;
			BoundingSphere targetSphere = targetBone->getObjectSphere();
			BoundingSphere transformedTargetSphere;
			targetSphere.Transform(transformedTargetSphere, XMLoadFloat4x4(&targetBone->getWorldMatrix()));
			for (const auto& attacker : attackers) {
				if (!attacker->IsAttacking()) continue;
				for (const auto& attackerBone : attacker->getObject()->getObjects()) {
					if (!(attackerBone->getBoundingInfo() & 0x1000)) continue;
					BoundingSphere attackerSphere = attackerBone->getObjectSphere();
					BoundingSphere transformedAttackerSphere;
					attackerSphere.Transform(transformedAttackerSphere, XMLoadFloat4x4(&attackerBone->getWorldMatrix()));
					if (transformedAttackerSphere.Intersects(transformedTargetSphere)) {
						float damage = 0.0f;
						switch (attacker->getCurrentSkill()) {
						case 1: damage = 200.0f; break;
						case 2: damage = 400.0f; break;
						case 3: damage = 300.0f; break;
						}
						if (damage > 0.0f) {
							target->Attacked(damage);
						}
						return;
					}
				}
			}
		}
	}
}


void CRaytracingCollisionTestScene::PrepareTerrainTexture()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 14;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	g_DxResource.device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pTerrainDescriptor.GetAddressOf()));

	struct alignas(16) terrainINFO {
		int numLayer{};
		float padding[3]{};
		int bHasDiffuse[4]{};
		int bHasNormal[4]{};
		int bHasMask[4]{};
	};

	auto rdesc = BASIC_BUFFER_DESC;
	rdesc.Width = Align(sizeof(terrainINFO), 256);

	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &rdesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(m_pTerrainCB.GetAddressOf()));

	terrainINFO* pMap{};
	m_pTerrainCB->Map(0, nullptr, reinterpret_cast<void**>(&pMap));
	pMap->numLayer = 4;
	pMap->bHasDiffuse[0] = pMap->bHasDiffuse[1] = pMap->bHasDiffuse[2] = pMap->bHasDiffuse[3] = 1;
	pMap->bHasNormal[0] = pMap->bHasNormal[1] = pMap->bHasNormal[2] = pMap->bHasNormal[3] = 0;
	pMap->bHasMask[0] = pMap->bHasMask[2] = pMap->bHasMask[3] = 0;
	m_pTerrainCB->Unmap(0, nullptr);

	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	size_t textureIndex = textures.size();
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Terrain_WinterLands_splatmap.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\RockStalagmites00_terrain2.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\Stonerock03_Metallic.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\SnowGround00_Albedo.dds"));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	D3D12_RESOURCE_DESC d3dRD;

	UINT increment = g_DxResource.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_CPU_DESCRIPTOR_HANDLE  handle = m_pTerrainDescriptor->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cdesc{};
	cdesc.BufferLocation = m_pTerrainCB->GetGPUVirtualAddress();
	cdesc.SizeInBytes = rdesc.Width;

	g_DxResource.device->CreateConstantBufferView(&cdesc, handle);
	handle.ptr += increment;

	// splat
	d3dRD = textures[textureIndex]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;

	g_DxResource.device->CreateShaderResourceView(textures[textureIndex]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 0 ===============================================================

	d3dRD = textures[textureIndex + 1]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 1]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 1 ===============================================================

	d3dRD = textures[textureIndex + 2]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 2]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 2 ===============================================================

	d3dRD = textures[textureIndex + 3]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 3]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	// layer 3 ===============================================================

	d3dRD = textures[textureIndex + 4]->getTexture()->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(textures[textureIndex + 4]->getTexture(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;

	d3dRD = g_DxResource.nullTexture->GetDesc();
	srvDesc.Format = d3dRD.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = -1;
	g_DxResource.device->CreateShaderResourceView(g_DxResource.nullTexture.Get(), &srvDesc, handle);
	handle.ptr += increment;
}

void CRaytracingCollisionTestScene::UpdateObject(float fElapsedTime)
{
	// compute shader & rootSignature set
	g_DxResource.cmdList->SetPipelineState(m_pAnimationComputeShader.Get());
	g_DxResource.cmdList->SetComputeRootSignature(m_pComputeRootSignature.Get());

	m_pResourceManager->UpdateSkinningMesh(fElapsedTime);
	Flush();
	// Skinning Object BLAS ReBuild
	m_pResourceManager->ReBuildBLAS();

	m_pResourceManager->UpdateWorldMatrix();

	for (auto& p : m_vPlayers)
		p->UpdateObject(fElapsedTime);

	for (auto& p : m_vMonsters)
		p->UpdateObject(fElapsedTime);

	switch (m_vMonsters[0]->getCurrentSkill())
	{
	case 1:
		if (m_vMonsters[0]->getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || m_vMonsters[0]->getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
		{
			AttackCollision(m_vPlayers, m_vMonsters);
		}
		break;
	case 2:
		if (m_vMonsters[0]->getAniManager()->IsAnimationInTimeRange(0.3f, 0.6f))
		{
			AttackCollision(m_vPlayers, m_vMonsters);
		}
		break;
	case 3:
		break;
	}
	/*m_pPlayer->HeightCheck(m_pHeightMap.get(), fElapsedTime);
	m_pMonster->HeightCheck(m_pHeightMap.get(), fElapsedTime);*/

	if (m_pCamera->getThirdPersonState()) {
		XMFLOAT3& EYE = m_pCamera->getEyeCalculateOffset();
		/*float cHeight = m_pHeightMap->GetHeightinWorldSpace(EYE.x + 1024.0f, EYE.z + 1024.0f);
		if (EYE.z >= -500.0f) {
			if (cHeight < 10.5f)
				cHeight = 10.5f;
		}
		if (EYE.y < cHeight + 0.5f) {
			m_pCamera->UpdateViewMatrix(cHeight + 0.5f);
		}
		else*/
		m_pCamera->UpdateViewMatrix();
	}
	else
		m_pCamera->UpdateViewMatrix();
	m_pAccelerationStructureManager->UpdateScene(m_pCamera->getEye());

	switch (m_nState) {
	case IS_LOADING: {
		wOpacity -= 0.5 * fElapsedTime;
		if (wOpacity < 0.0f) {
			m_nState = IS_GAMING;
			wOpacity = 0.0f;
		}
		m_vUIs[0]->setColor(0.0, 0.0, 0.0, wOpacity);
		break;
	}
	case IS_GAMING: {
		break;
	}
	case IS_FINISH: {
		wOpacity += 0.2 * fElapsedTime;
		if (wOpacity > 1.0f) {
			ShowCursor(TRUE);
			m_nNextScene = SCENE_TITLE;
			wOpacity = 1.0f;
		}
		m_vUIs[0]->setColor(0.0, 0.0, 0.0, wOpacity);
		break;
	}
	}
}

void CRaytracingCollisionTestScene::Render()
{
	m_pCamera->SetShaderVariable();
	m_pAccelerationStructureManager->SetScene();
	m_pResourceManager->SetLights();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	g_DxResource.cmdList->SetComputeRootDescriptorTable(4, textures[m_nSkyboxIndex]->getView()->GetGPUDescriptorHandleForHeapStart());
	//g_DxResource.cmdList->SetComputeRootDescriptorTable(5, m_pTerrainDescriptor->GetGPUDescriptorHandleForHeapStart());

	D3D12_DISPATCH_RAYS_DESC raydesc{};
	raydesc.Depth = 1;
	raydesc.Width = DEFINED_UAV_BUFFER_WIDTH;
	raydesc.Height = DEFINED_UAV_BUFFER_HEIGHT;

	raydesc.RayGenerationShaderRecord.StartAddress = m_pShaderBindingTable->getRayGenTable()->GetGPUVirtualAddress();
	raydesc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	raydesc.MissShaderTable.StartAddress = m_pShaderBindingTable->getMissTable()->GetGPUVirtualAddress();
	raydesc.MissShaderTable.SizeInBytes = m_pShaderBindingTable->getMissSize();
	raydesc.MissShaderTable.StrideInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	raydesc.HitGroupTable.StartAddress = m_pShaderBindingTable->getHitGroupTable()->GetGPUVirtualAddress();
	raydesc.HitGroupTable.SizeInBytes = m_pShaderBindingTable->getHitGroupSize();
	raydesc.HitGroupTable.StrideInBytes = m_pShaderBindingTable->getHitGroupStride();

	g_DxResource.cmdList->DispatchRays(&raydesc);

	// UI Render ==================================================================================

	ID3D12GraphicsCommandList4* cmdList = g_DxResource.cmdList;
	auto barrier = [&](ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		{
			D3D12_RESOURCE_BARRIER resBarrier{};
			resBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			resBarrier.Transition.pResource = pResource;
			resBarrier.Transition.StateBefore = before;
			resBarrier.Transition.StateAfter = after;
			resBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			cmdList->ResourceBarrier(1, &resBarrier);
		};

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RENDER_TARGET);

	D3D12_VIEWPORT vv{};
	vv.Width = DEFINED_GAME_WINDOW_WIDTH; vv.Height = DEFINED_GAME_WINDOW_HEIGHT; vv.MinDepth = 0.0f; vv.MaxDepth = 1.0f;
	cmdList->RSSetViewports(1, &vv);
	D3D12_RECT ss{ 0, 0, DEFINED_UAV_BUFFER_WIDTH, DEFINED_UAV_BUFFER_HEIGHT };
	cmdList->RSSetScissorRects(1, &ss);
	cmdList->OMSetRenderTargets(1, &m_RTV->GetCPUDescriptorHandleForHeapStart(), FALSE, &m_DSV->GetCPUDescriptorHandleForHeapStart());
	cmdList->SetGraphicsRootSignature(m_UIRootSignature.Get());
	cmdList->SetPipelineState(m_UIPipelineState.Get());
	cmdList->SetGraphicsRootConstantBufferView(0, m_cameraCB->GetGPUVirtualAddress());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& p : m_vUIs)
		p->Render();

	barrier(m_pOutputBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
}