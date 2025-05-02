#include "Scene.h"

constexpr unsigned short NUM_G_ROOTPARAMETER = 6;

void TitleScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer)
{
	m_pOutputBuffer = outputBuffer;

}

//void TitleScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
void TitleScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{

}

void TitleScene::CreateRootSignature()
{

}
void TitleScene::CreateRenderTargetView()
{

}

void TitleScene::UpdateObject(float fElapsedTime)
{

}
void TitleScene::Render()
{

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

	bool test = false;
	auto& animationManagers = m_pResourceManager->getAnimationManagers();
	for (auto& animationManager : animationManagers) {
		animationManager->UpdateCombo(fElapsedTime);
		if (!animationManager->IsInCombo() && animationManager->IsAnimationFinished()) {
			animationManager->ChangeAnimation(0, false); // idle�� ��ȯ
			test = true;
			m_LockAnimation1 = false;
		}
	}

	m_pResourceManager->UpdateWorldMatrix();

	if (test) {
		m_pResourceManager->UpdatePosition(fElapsedTime); //��ġ ������Ʈ
	}

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

void CRaytracingScene::CreateRootSignature()
{
	{
		// Global Root Signature
		D3D12_DESCRIPTOR_RANGE rootRange{};								// u0
		rootRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		rootRange.NumDescriptors = 1;
		rootRange.BaseShaderRegister = 0;
		rootRange.RegisterSpace = 0;

		D3D12_DESCRIPTOR_RANGE cubeMapRange{};							// t3
		cubeMapRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		cubeMapRange.NumDescriptors = 1;
		cubeMapRange.BaseShaderRegister = 3;
		cubeMapRange.RegisterSpace = 0;

		D3D12_DESCRIPTOR_RANGE terrainTRange[2]{};						// b0, space2
		terrainTRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		terrainTRange[0].NumDescriptors = 1;
		terrainTRange[0].BaseShaderRegister = 2;
		terrainTRange[0].RegisterSpace = 0;

		terrainTRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		terrainTRange[1].NumDescriptors = 13;
		terrainTRange[1].BaseShaderRegister = 4;
		terrainTRange[1].RegisterSpace = 0;
		terrainTRange[1].OffsetInDescriptorsFromTableStart = 1;

		// 0. uavBuffer, 1. AS, 2. camera, 3. Lights, 4. Enviorment(cubeMap), 5. TerrainInfo
		D3D12_ROOT_PARAMETER params[NUM_G_ROOTPARAMETER] = {};
		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	// u0
		params[0].DescriptorTable.NumDescriptorRanges = 1;
		params[0].DescriptorTable.pDescriptorRanges = &rootRange;

		params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;				// t0
		params[1].Descriptor.RegisterSpace = 0;
		params[1].Descriptor.ShaderRegister = 0;

		params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;				// b0, space0
		params[2].Descriptor.RegisterSpace = 0;
		params[2].Descriptor.ShaderRegister = 0;

		params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;				// b0, space1
		params[3].Descriptor.RegisterSpace = 1;
		params[3].Descriptor.ShaderRegister = 0;

		params[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[4].DescriptorTable.NumDescriptorRanges = 1;
		params[4].DescriptorTable.pDescriptorRanges = &cubeMapRange;

		params[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[5].DescriptorTable.NumDescriptorRanges = 2;
		params[5].DescriptorTable.pDescriptorRanges = terrainTRange;

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
		rtDesc.NumParameters = NUM_G_ROOTPARAMETER;
		rtDesc.NumStaticSamplers = 1;
		rtDesc.pParameters = params;
		rtDesc.pStaticSamplers = &samplerDesc;

		ID3DBlob* pBlob{};
		D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
		g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pGlobalRootSignature.GetAddressOf()));
		pBlob->Release();
	}
	{
		// LocalRootSignature
		D3D12_DESCRIPTOR_RANGE srvRange[7] = {};
		srvRange[0].BaseShaderRegister = 2;		// t2, space0
		srvRange[0].NumDescriptors = 1;
		srvRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange[0].RegisterSpace = 0;

		srvRange[1].BaseShaderRegister = 2;		// t2, space1
		srvRange[1].NumDescriptors = 1;
		srvRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange[1].RegisterSpace = 1;

		srvRange[2].BaseShaderRegister = 2;		// t2, space2
		srvRange[2].NumDescriptors = 1;
		srvRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange[2].RegisterSpace = 2;

		srvRange[3].BaseShaderRegister = 2;		// t2, space3
		srvRange[3].NumDescriptors = 1;
		srvRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange[3].RegisterSpace = 3;

		srvRange[4].BaseShaderRegister = 2;		// t2, space4
		srvRange[4].NumDescriptors = 1;
		srvRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange[4].RegisterSpace = 4;
		
		srvRange[5].BaseShaderRegister = 2;		// t2, space5
		srvRange[5].NumDescriptors = 1;
		srvRange[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange[5].RegisterSpace = 5;

		srvRange[6].BaseShaderRegister = 2;		// t2, space6
		srvRange[6].NumDescriptors = 1;
		srvRange[6].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange[6].RegisterSpace = 6;

		D3D12_ROOT_PARAMETER params[17] = {};
		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// b1, space0
		params[0].Descriptor.RegisterSpace = 0;
		params[0].Descriptor.ShaderRegister = 1;

		params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;	// b1, spcae1
		params[1].Descriptor.RegisterSpace = 1;
		params[1].Descriptor.ShaderRegister = 1;

		params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space0
		params[2].Descriptor.RegisterSpace = 0;
		params[2].Descriptor.ShaderRegister = 1;

		params[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space1
		params[3].Descriptor.RegisterSpace = 1;
		params[3].Descriptor.ShaderRegister = 1;

		params[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space2
		params[4].Descriptor.RegisterSpace = 2;
		params[4].Descriptor.ShaderRegister = 1;

		params[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space3
		params[5].Descriptor.RegisterSpace = 3;
		params[5].Descriptor.ShaderRegister = 1;

		params[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space4
		params[6].Descriptor.RegisterSpace = 4;
		params[6].Descriptor.ShaderRegister = 1;

		params[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space5
		params[7].Descriptor.RegisterSpace = 5;
		params[7].Descriptor.ShaderRegister = 1;

		params[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space6
		params[8].Descriptor.RegisterSpace = 6;
		params[8].Descriptor.ShaderRegister = 1;

		params[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;	// t1, space7
		params[9].Descriptor.RegisterSpace = 7;
		params[9].Descriptor.ShaderRegister = 1;

		// texture
		params[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[10].DescriptorTable.NumDescriptorRanges = 1;
		params[10].DescriptorTable.pDescriptorRanges = &srvRange[0];

		params[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[11].DescriptorTable.NumDescriptorRanges = 1;
		params[11].DescriptorTable.pDescriptorRanges = &srvRange[1];

		params[12].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[12].DescriptorTable.NumDescriptorRanges = 1;
		params[12].DescriptorTable.pDescriptorRanges = &srvRange[2];

		params[13].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[13].DescriptorTable.NumDescriptorRanges = 1;
		params[13].DescriptorTable.pDescriptorRanges = &srvRange[3];

		params[14].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[14].DescriptorTable.NumDescriptorRanges = 1;
		params[14].DescriptorTable.pDescriptorRanges = &srvRange[4];

		params[15].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[15].DescriptorTable.NumDescriptorRanges = 1;
		params[15].DescriptorTable.pDescriptorRanges = &srvRange[5];

		params[16].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		params[16].DescriptorTable.NumDescriptorRanges = 1;
		params[16].DescriptorTable.pDescriptorRanges = &srvRange[6];

		D3D12_ROOT_SIGNATURE_DESC rtDesc{};
		rtDesc.NumParameters = 17;
		rtDesc.NumStaticSamplers = 0;
		rtDesc.pParameters = params;
		rtDesc.pStaticSamplers = nullptr;
		rtDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

		ID3DBlob* pBlob{};
		D3D12SerializeRootSignature(&rtDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &pBlob, nullptr);
		g_DxResource.device->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(m_pLocalRootSignature.GetAddressOf()));
		pBlob->Release();
	}
}

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

void CRaytracingTestScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer)
{
	// Create Global & Local Root Signature
	CreateRootSignature();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	m_pRaytracingPipeline = std::make_unique<CRayTracingPipeline>();
	m_pRaytracingPipeline->Setup(1 + 2 + 1 + 2 + 1 + 1);
	m_pRaytracingPipeline->AddLibrarySubObject(compiledShader, std::size(compiledShader));
	m_pRaytracingPipeline->AddHitGroupSubObject(L"HitGroup", L"RadianceClosestHit", L"RadianceAnyHit");
	m_pRaytracingPipeline->AddHitGroupSubObject(L"ShadowHit", L"ShadowClosestHit");
	m_pRaytracingPipeline->AddShaderConfigSubObject(8, 20);
	m_pRaytracingPipeline->AddLocalRootAndAsoociationSubObject(m_pLocalRootSignature.Get());
	m_pRaytracingPipeline->AddGlobalRootSignatureSubObject(m_pGlobalRootSignature.Get());
	m_pRaytracingPipeline->AddPipelineConfigSubObject(4);
	m_pRaytracingPipeline->MakePipelineState();

	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	m_pResourceManager->SetUp(3);								// LightBufferReady
	// Read File Here ========================================	! All Files Are Read Once !
	m_pResourceManager->AddResourceFromFile(L"src\\model\\City.bin", "src\\texture\\City\\");
	//m_pResourceManager->AddResourceFromFile(L"src\\model\\WinterLand2.bin", "src\\texture\\Map\\");
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Gorhorrid.bin", "src\\texture\\Gorhorrid\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Xenokarce.bin", "src\\texture\\Xenokarce\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Lion.bin", "src\\texture\\Lion\\");
	m_pResourceManager->LightTest();
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// Create new Objects, Copy SkinningObject here ========================================

	// Copy Example
	//skinned.emplace_back(std::make_unique<CRayTracingSkinningObject>());
	//skinned[1]->CopyFromOtherObject(skinned[0].get());
	//aManagers.emplace_back(std::make_unique<CAnimationManager>(*aManagers[0].get()));
	//aManagers[1]->SetFramesPointerFromSkinningObject(skinned[1]->getObjects());
	//aManagers[1]->MakeAnimationMatrixIndex(skinned[1].get());
	//aManagers[1]->UpdateAnimation(0.5f);		// Not Need

	// Create new Object Example
	/*m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\asdf.raw", 2049, 2049, XMFLOAT3(1.0f, 0.025f, 1.0f));

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
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();	// CBV RAII
	m_pResourceManager->PrepareObject();	// Ready OutputBuffer to  SkinningObject


	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// Normal Object Copy & Manipulation Matrix Here ================================
	skinned[0]->SetPosition(XMFLOAT3(0.0f, 0.0f, 50.0f));
	//skinned[1]->setPreTransform(1.0, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3());
	//skinned[1]->SetPosition(XMFLOAT3(20.0f, 0.0f, 0.0f));
	skinned[0]->setPreTransform(2.0, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3());
	// ==============================================================================

	m_pResourceManager->PrepareObject();
	m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(0);

	// Setting Camera ==============================================================
	//m_pCamera->SetTarget(skinned[0]->getObjects()[0].get());
	//m_pCamera->SetCameraLength(10.0f);
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
	}
}

void CRaytracingTestScene::MouseProcessing(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_LBUTTONDOWN:
	{
		if (!m_LockAnimation && !m_LockAnimation1) {
			auto& animationManagers = m_pResourceManager->getAnimationManagers();
			for (auto& animationManager : animationManagers) {
				animationManager->OnAttackInput();
			}
		}
		break;
	}
	case WM_MOUSEMOVE:
	{
		int xPos = LOWORD(lParam);
		int yPos = HIWORD(lParam);
		static POINT oldPos = { 0, 0 };

		if (oldPos.x != 0 || oldPos.y != 0) {
			float deltaX = static_cast<float>(xPos - oldPos.x);

			m_pCamera->Rotate(deltaX * 3.0f * 0.5f, 0.0f); // ī�޶� ȸ���� /3.0f ���ִϱ� �׳� ���ؼ� ĳ���� ȸ���̶� ���� �����.

			auto* animationManager = m_pResourceManager->getAnimationManagers()[0].get();
			if (animationManager && !animationManager->getFrame().empty()) {
				CGameObject* frame = animationManager->getFrame()[0];
				m_pResourceManager->getSkinningObjectList()[0]->Rotation(XMFLOAT3(0.0f, deltaX *0.5f , 0.0f),*frame);
			}
		}

		oldPos.x = xPos;
		oldPos.y = yPos;
		break;
	}
	}
}

void CRaytracingTestScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);


	if (!m_pCamera->getThirdPersonState()) {
		if (keyBuffer['W'] & 0x80)
			m_pCamera->Move(0, fElapsedTime);
		if (keyBuffer['S'] & 0x80)
			m_pCamera->Move(5, fElapsedTime);
		if (keyBuffer['D'] & 0x80)
			m_pCamera->Move(3, fElapsedTime);
		if (keyBuffer['A'] & 0x80)
			m_pCamera->Move(4, fElapsedTime);
		if (keyBuffer[VK_SPACE] & 0x80)
			m_pCamera->Move(1, fElapsedTime);
		if (keyBuffer[VK_CONTROL] & 0x80)
			m_pCamera->Move(2, fElapsedTime);
	}

	if (m_LockAnimation) {
		if (m_pResourceManager->getAnimationManagers()[0]->IsInCombo()) {
			m_LockAnimation = false;
		}
		else {
			return;
		}
	}

	if (m_LockAnimation1) {
		if (m_pResourceManager->getAnimationManagers()[0]->IsAnimationFinished()) {
			m_LockAnimation1 = false;
		}
		else {
			return;
		}
	}
	// W + A (���� �밢�� ��)
	if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
		if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_LEFT_UP, true); // �ٱ�: ���� �밢�� ��
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_LEFT_UP, true); // �ٱ� ����
		}
	}
	// W + A + Shift �� ���� (���� �밢�� �� �ȱ�� ��ȯ)
	else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT_UP, true); // �ȱ�: ���� �밢�� ��
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// W + A �ܵ� (���� �밢�� �� �ȱ�)
	else if ((keyBuffer['W'] & 0x80) && (keyBuffer['A'] & 0x80)) {
		if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT_UP, true); // �ȱ�: ���� �밢�� ��
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT_UP, true); // �ȱ� ����
		}
	}
	// W + A + Shift���� A �� ���� (���� �ٱ�� ��ȯ)
	else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_FORWARD, true); // �ٱ�: ����
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// W + A���� A �� ���� (���� �ȱ�� ��ȯ)
	else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_FORWARD, true); // �ȱ�: ����
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// W + A���� W �� ���� (���� �ȱ�� ��ȯ)
	else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT, true); // �ȱ�: ���� (����: 8)
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// W + D + Shift (������ �밢�� �� �ٱ�)
	else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
		if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_RIGHT_UP, true); // �ٱ�: ������ �밢�� ��
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_RIGHT_UP, true); // �ٱ� ����
		}
	}
	// W + D + Shift �� ���� (������ �밢�� �� �ȱ�� ��ȯ)
	else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT_UP, true); // �ȱ�: ������ �밢�� ��
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// W + D �ܵ� (������ �밢�� �� �ȱ�)
	else if ((keyBuffer['W'] & 0x80) && (keyBuffer['D'] & 0x80)) {
		if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT_UP, true); // �ȱ�: ������ �밢�� ��
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT_UP, true); // �ȱ� ����
		}
	}
	// W + D + Shift���� D �� ���� (���� �ٱ�� ��ȯ)
	else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_FORWARD, true); // �ٱ�: ����
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// W + D���� D �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_FORWARD, true); // �ȱ�: ����
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// W + D���� W �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT, true); // �ȱ�: ���� (����: 9)
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S + A + Shift (���� �밢�� �� �ٱ�)
	else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
		if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_LEFT_DOWN, true); // �ٱ�: �� �� �밢�� ��
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_LEFT_DOWN, true); // �ٱ� ����
		}
	}
	// S + A + Shift �� ���� (���� �밢�� �� �ȱ��?��ȯ)
	else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT_DOWN, true); // �ȱ�: ���� �밢�� ��
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S + A �ܵ� (���� �밢�� �� �ȱ�)
	else if ((keyBuffer['S'] & 0x80) && (keyBuffer['A'] & 0x80)) {
		if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['A'] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT_DOWN, true); // �ȱ�: ���� �밢�� ��
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT_DOWN, true); // �ȱ� ����
		}
	}
	// S + A + Shift���� A �� ���� (���� �ٱ��?��ȯ)
	else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_BACKWARD, true); // �ٱ�: ���� (����: 20)
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S + A���� A �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['A'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_BACKWARD, true); // �ȱ�: ����
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S + A���� S �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT, true); // �ȱ�: ���� (����: 8)
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S + D + Shift (������ �밢�� �� �ٱ�)
	else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
		if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_RIGHT_DOWN, true); // �ٱ�: ������ �밢�� ��
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_RIGHT_DOWN, true); // �ٱ� ����
		}
	}
	// S + D + Shift �� ���� (������ �밢�� �� �ȱ��?��ȯ)
	else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT_DOWN, true); // �ȱ�: ������ �밢�� ��
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S + D �ܵ� (������ �밢�� �� �ȱ�)
	else if ((keyBuffer['S'] & 0x80) && (keyBuffer['D'] & 0x80)) {
		if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer['D'] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT_DOWN, true); // �ȱ�: ������ �밢�� ��
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT_DOWN, true); // �ȱ� ����
		}
	}
	// S + D + Shift���� D �� ���� (���� �ٱ��?��ȯ)
	else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_BACKWARD, true); // �ٱ�: ���� (����: 20)
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S + D���� D �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['D'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_BACKWARD, true); // �ȱ�: ����
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S + D���� S �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['S'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT, true); // �ȱ�: ���� (����: 9)
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// W + Shift (���� �ٱ�)
	else if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
		if (!(m_PrevKeyBuffer['W'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_FORWARD, true); // �ٱ�: ����
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_FORWARD, true); // �ٱ� ����
		}
	}
	// W + Shift �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['W'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_FORWARD, true); // �ȱ�: ����
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// W �ܵ� (���� �ȱ�)
	else if (keyBuffer['W'] & 0x80) {
		if (!(m_PrevKeyBuffer['W'] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_FORWARD, true); // �ȱ�: ����
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_FORWARD, true); // �ȱ� ����
		}
	}
	// S + Shift (���� �ٱ�)
	else if ((keyBuffer['S'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
		if (!(m_PrevKeyBuffer['S'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_BACKWARD, true); // �ٱ�: ���� (����: 20)
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_BACKWARD, true); // �ٱ� ����
		}
	}
	// S + Shift �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['S'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_BACKWARD, true); // �ȱ�: ����
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S �ܵ� (���� �ȱ�)
	else if (keyBuffer['S'] & 0x80) {
		if (!(m_PrevKeyBuffer['S'] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_BACKWARD, true); // �ȱ�: ����
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_BACKWARD, true); // �ȱ� ����
		}
	}
	// A + Shift (���� �ٱ�)
	else if ((keyBuffer['A'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
		if (!(m_PrevKeyBuffer['A'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_LEFT, true); // �ٱ�: ���� (����: 18)
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_LEFT, true); // �ٱ� ����
		}
	}
	// A + Shift �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['A'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT, true); // �ȱ�: ���� (����: 8)
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// A �ܵ� (���� �ȱ�)
	else if (keyBuffer['A'] & 0x80) {
		if (!(m_PrevKeyBuffer['A'] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT, true); // �ȱ�: ���� (����: 8)
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_LEFT, true); // �ȱ� ����
		}
	}
	// D + Shift (���� �ٱ�)
	else if ((keyBuffer['D'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
		if (!(m_PrevKeyBuffer['D'] & 0x80) || !(m_PrevKeyBuffer[VK_LSHIFT] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_RIGHT, true); // �ٱ�: ���� (����: 19)
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(RUN_RIGHT, true); // �ٱ� ����
		}
	}
	// D + Shift �� ���� (���� �ȱ��?��ȯ)
	else if ((keyBuffer['D'] & 0x80) && (m_PrevKeyBuffer[VK_LSHIFT] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT, true); // �ȱ�: ���� (����: 9)
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// D �ܵ� (���� �ȱ�)
	else if (keyBuffer['D'] & 0x80) {
		if (!(m_PrevKeyBuffer['D'] & 0x80)) {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT, true); // �ȱ�: ���� (����: 9)
			m_pResourceManager->UpdatePosition(fElapsedTime);
		}
		else {
			m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(WALK_RIGHT, true); // �ȱ� ����
		}
	}
	// W �� ���� (IDLE�� ��ȯ)
	else if ((m_PrevKeyBuffer['W'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(IDLE, false); // IDLE
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// S �� ���� (IDLE�� ��ȯ)
	else if ((m_PrevKeyBuffer['S'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(IDLE, false); // IDLE
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// A �� ���� (IDLE�� ��ȯ)
	else if ((m_PrevKeyBuffer['A'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(IDLE, false); // IDLE
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}
	// D �� ���� (IDLE�� ��ȯ)
	else if ((m_PrevKeyBuffer['D'] & 0x80) && !(keyBuffer['W'] & 0x80) && !(keyBuffer['A'] & 0x80) && !(keyBuffer['S'] & 0x80) && !(keyBuffer['D'] & 0x80) && !(keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(IDLE, false); // IDLE
		m_pResourceManager->UpdatePosition(fElapsedTime);
	}

	if (keyBuffer['G'] & 0x80) {
		auto* animationManager = m_pResourceManager->getAnimationManagers()[0].get();
		if (animationManager && !animationManager->getFrame().empty()) {
			CGameObject* frame = animationManager->getFrame()[0];
			m_pResourceManager->getSkinningObjectList()[0]->Rotation(XMFLOAT3(0.0f, -180.0f * fElapsedTime, 0.0f), *frame);
		}
	}

	if (keyBuffer['H'] & 0x80) {
		m_pResourceManager->getSkinningObjectList()[0]->Rotate(XMFLOAT3(0.0f, 180.0f * fElapsedTime, 0.0f)); //��ȸ��
	}

	if (keyBuffer['T'] & 0x80) {
		auto* animationManager = m_pResourceManager->getAnimationManagers()[0].get();
		if (animationManager && !animationManager->getFrame().empty()) {
			CGameObject* frame = animationManager->getFrame()[0];
			m_pResourceManager->getSkinningObjectList()[0]->Rotation(XMFLOAT3(0.0f, 180.0f * fElapsedTime, 0.0f),*frame);
		}
	}

	if ((keyBuffer['J'] & 0x80) && !(m_PrevKeyBuffer['J'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(1, true); //���ϰ� �±�
		m_LockAnimation1 = true;
	}

	if ((keyBuffer['K'] & 0x80) && !(m_PrevKeyBuffer['K'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(2, true); //���ϰ� �°� �ױ�
		m_LockAnimation1 = true;
	}

	if ((keyBuffer[VK_SPACE] & 0x80) && !(m_PrevKeyBuffer[VK_SPACE] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(21, true); //Dodge
		m_LockAnimation1 = true;
	}

	if ((keyBuffer['L'] & 0x80) && !(m_PrevKeyBuffer['L'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(3, true); //���ϰ� �±�
		m_LockAnimation1 = true;
	}

	if ((keyBuffer['U'] & 0x80) && !(m_PrevKeyBuffer['U'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(4, true); //���ϰ� �°� �ױ�
		m_LockAnimation1 = true;
	}

	if ((keyBuffer['2'] & 0x80) && !(m_PrevKeyBuffer['2'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(31, true); //��ų2
		m_LockAnimation1 = true;
	}

	if ((keyBuffer['1'] & 0x80) && !(m_PrevKeyBuffer['1'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(32, true); //��ų1
		m_LockAnimation1 = true;
	}

	if ((keyBuffer['3'] & 0x80) && !(m_PrevKeyBuffer['3'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->StartSkill3(); //��ų3
		m_LockAnimation = true;
	}
	// ���� Ű ���¸� ���� ���·� ����
	memcpy(m_PrevKeyBuffer, keyBuffer, sizeof(keyBuffer));
}

// ==============================================================================

void CRaytracingMaterialTestScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer)
{
	// Create Global & Local Root Signature
	CreateRootSignature();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	m_pRaytracingPipeline = std::make_unique<CRayTracingPipeline>();
	m_pRaytracingPipeline->Setup(1 + 2 + 1 + 2 + 1 + 1);
	m_pRaytracingPipeline->AddLibrarySubObject(compiledShader, std::size(compiledShader));
	m_pRaytracingPipeline->AddHitGroupSubObject(L"HitGroup", L"RadianceClosestHit", L"RadianceAnyHit");
	m_pRaytracingPipeline->AddHitGroupSubObject(L"ShadowHit", L"ShadowClosestHit", L"ShadowAnyHit");
	m_pRaytracingPipeline->AddShaderConfigSubObject(8, 20);
	m_pRaytracingPipeline->AddLocalRootAndAsoociationSubObject(m_pLocalRootSignature.Get());
	m_pRaytracingPipeline->AddGlobalRootSignatureSubObject(m_pGlobalRootSignature.Get());
	m_pRaytracingPipeline->AddPipelineConfigSubObject(6);
	m_pRaytracingPipeline->MakePipelineState();

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
	m_pResourceManager->LightTest();
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

void CRaytracingMaterialTestScene::PrepareTerrainTexture()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = 14;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	g_DxResource.device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pTerrainDescriptor.GetAddressOf()));

	struct alignas(16) terrainINFO  {
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

void CRaytracingWinterLandScene::SetUp(ComPtr<ID3D12Resource>& outputBuffer)
{
	// Create Global & Local Root Signature
	CreateRootSignature();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	m_pRaytracingPipeline = std::make_unique<CRayTracingPipeline>();
	m_pRaytracingPipeline->Setup(1 + 2 + 1 + 2 + 1 + 1);
	m_pRaytracingPipeline->AddLibrarySubObject(compiledShader, std::size(compiledShader));
	m_pRaytracingPipeline->AddHitGroupSubObject(L"HitGroup", L"RadianceClosestHit", L"RadianceAnyHit");
	m_pRaytracingPipeline->AddHitGroupSubObject(L"ShadowHit", L"ShadowClosestHit", L"ShadowAnyHit");
	m_pRaytracingPipeline->AddShaderConfigSubObject(8, 20);
	m_pRaytracingPipeline->AddLocalRootAndAsoociationSubObject(m_pLocalRootSignature.Get());
	m_pRaytracingPipeline->AddGlobalRootSignatureSubObject(m_pGlobalRootSignature.Get());
	m_pRaytracingPipeline->AddPipelineConfigSubObject(6);
	m_pRaytracingPipeline->MakePipelineState();

	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	m_pResourceManager->SetUp(3);
	// ���⿡ ���� �ֱ� ========================================	! ��� ������ �ѹ����� �б� !
	m_pResourceManager->AddResourceFromFile(L"src\\model\\WinterLand1.bin", "src\\texture\\Map\\");
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\");
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Gorhorrid.bin", "src\\texture\\Gorhorrid\\");
	// ���� �߰�
	m_pResourceManager->AddLightsFromFile(L"src\\Light\\LightingV2.bin");
	m_pResourceManager->ReadyLightBufferContent();
	m_pResourceManager->LightTest();
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// ������ ���ο� ��ü & skinning Object ����� ���⼭ ========================================

	for (auto& o : skinned[1]->getObjects()) {
		for (auto& ma : o->getMaterials())
			ma.m_bHasEmissiveColor = false;
	}

	// terrian
	m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\terrain.raw", 2049, 2049, XMFLOAT3(1.0f, 0.0312f, 1.0f));
	meshes.emplace_back(std::make_unique<Mesh>(m_pHeightMap.get(), "terrain"));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[normalObjects.size() - 1]->SetMeshIndex(meshes.size() - 1);

	normalObjects[normalObjects.size() - 1]->SetInstanceID(10);
	normalObjects[normalObjects.size() - 1]->getMaterials().emplace_back();
	normalObjects[normalObjects.size() - 1]->SetPosition(XMFLOAT3(-1024.0, 0.0, -1024.0));


	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02_NORM.dds"));
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\Map\\FrozenWater02_MNS.dds"));
	auto p = std::find_if(normalObjects.begin(), normalObjects.end(), [](std::unique_ptr<CGameObject>& p) {
		return p->getFrameName() == "Water";
		});
	if (p != normalObjects.end()) {
		(*p)->SetInstanceID(1);
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
	textures.emplace_back(std::make_unique<CTexture>(L"src\\texture\\WinterLandSky.dds", true));
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();	// ��� ������Ʈ ������� ���� & �ʱ�ȭ
	m_pResourceManager->PrepareObject();	// Ready OutputBuffer to  SkinningObject


	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// ���⼭ �ʿ��� ��ü(normalObject) ���� & ��� ���� ===============================

	skinned[0]->setPreTransform(3.0f, XMFLOAT3(), XMFLOAT3());
	skinned[0]->SetPosition(XMFLOAT3(-72.5f, 0.0f, -998.0f));
	skinned[1]->setPreTransform(5.0f, XMFLOAT3(), XMFLOAT3());
	skinned[1]->SetPosition(XMFLOAT3(-28.0f, 0.0f, -245.0f));
	skinned[1]->Rotate(XMFLOAT3(0.0f, 180.0f, 0.0f));

	// ==============================================================================

	// ī�޶� ���� ==============================================================
	m_pCamera->SetTarget(skinned[0]->getObjects()[0].get());
	m_pCamera->SetCameraLength(15.0f);
	// ==========================================================================

	// AccelerationStructure
	m_pAccelerationStructureManager = std::make_unique<CAccelerationStructureManager>();
	m_pAccelerationStructureManager->Setup(m_pResourceManager.get(), 1);
	m_pAccelerationStructureManager->InitBLAS();
	m_pAccelerationStructureManager->InitTLAS();
}

void CRaytracingWinterLandScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_KEYDOWN:
		switch (wParam) {
		case '1':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(5);
			break;
		case '2':
			m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(0);
			break;
		case 'n':
		case 'N':
			m_pCamera->toggleNormalMapping();
			break;
		case 'm':
		case 'M':
			m_pCamera->toggleAlbedoColor();
			break;
		case '9':
			m_pCamera->SetThirdPersonMode(false);
			break;
		case '0':
			m_pCamera->SetThirdPersonMode(true);
			break;
		}
		break;
	case WM_KEYUP:
		break;
	}
}

void CRaytracingWinterLandScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
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

void CRaytracingWinterLandScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);

	bool shiftDown = false;
	if (keyBuffer[VK_SHIFT] & 0x80)
		shiftDown = true;

	if (false == m_pCamera->getThirdPersonState()) {
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
		if (keyBuffer['W'] & 0x80) {
			m_pResourceManager->getSkinningObjectList()[0]->move(fElapsedTime, 0);
		}
		if (keyBuffer['S'] & 0x80) {
			m_pResourceManager->getSkinningObjectList()[0]->move(fElapsedTime, 1);
		}
		if (keyBuffer['A'] & 0x80) {
			m_pResourceManager->getSkinningObjectList()[0]->Rotate(XMFLOAT3(0.0, -90.0f * fElapsedTime, 0.0f));
		}
		if (keyBuffer['D'] & 0x80) {
			m_pResourceManager->getSkinningObjectList()[0]->Rotate(XMFLOAT3(0.0, 90.0f * fElapsedTime, 0.0f));
		}
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
	Flush();
	// Skinning Object BLAS ReBuild
	m_pResourceManager->ReBuildBLAS();

	for (auto& p : m_pResourceManager->getSkinningObjectList()) {
		XMFLOAT4X4& playerWorld = p->getWorldMatrix();
		playerWorld._42 -= (30 * fElapsedTime);
		p->SetPosition(XMFLOAT3(playerWorld._41, playerWorld._42, playerWorld._43));

		float terrainHeight = m_pHeightMap->GetHeightinWorldSpace(playerWorld._41 + 1024.0f, playerWorld._43 + 1024.0f);
		if (terrainHeight > playerWorld._42) {
			playerWorld._42 = terrainHeight;
			p->SetPosition(XMFLOAT3(playerWorld._41, playerWorld._42, playerWorld._43));
		}
	}
	m_pResourceManager->UpdateWorldMatrix();

	m_pCamera->UpdateViewMatrix();
	m_pAccelerationStructureManager->UpdateScene(m_pCamera->getEye());
}

void CRaytracingWinterLandScene::Render()
{
	m_pCamera->SetShaderVariable();
	m_pAccelerationStructureManager->SetScene();
	m_pResourceManager->SetLights();
	std::vector<std::unique_ptr<CTexture>>& textures = m_pResourceManager->getTextureList();
	g_DxResource.cmdList->SetComputeRootDescriptorTable(4, textures[textures.size() - 1]->getView()->GetGPUDescriptorHandleForHeapStart());
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
}