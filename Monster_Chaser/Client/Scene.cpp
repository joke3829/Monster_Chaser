#include "Scene.h"

void CRaytracingScene::UpdateObject(float fElapsedTime)
{
	//m_pCamera->SetShaderVariable();

	// compute shader & rootSignature set
	g_DxResource.cmdList->SetPipelineState(m_pAnimationComputeShader.Get());
	g_DxResource.cmdList->SetComputeRootSignature(m_pComputeRootSignature.Get());

	auto& animationManagers = m_pResourceManager->getAnimationManagers();
	for (auto& animationManager : animationManagers) {
		animationManager->UpdateCombo(fElapsedTime);
		if (!animationManager->IsInCombo() && animationManager->IsAnimationFinished()) {
			animationManager->ChangeAnimation(24, false); // idle로 전환
		}
	}

	m_pResourceManager->UpdateSkinningMesh(fElapsedTime);
	Flush();
	// BLAS 재빌드
	m_pResourceManager->ReBuildBLAS();

	m_pResourceManager->UpdateWorldMatrix();

	m_pResourceManager->UpdatePosition(fElapsedTime); //위치 업데이트

	m_pCamera->UpdateViewMatrix();
	m_pAccelerationStructureManager->UpdateScene();
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
		D3D12_DESCRIPTOR_RANGE rootRange{};
		rootRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		rootRange.NumDescriptors = 1;
		rootRange.BaseShaderRegister = 0;
		rootRange.RegisterSpace = 0;

		// 0. uavBuffer, 1. AS, 2. camera
		D3D12_ROOT_PARAMETER params[3] = {};
		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;	// u0
		params[0].DescriptorTable.NumDescriptorRanges = 1;
		params[0].DescriptorTable.pDescriptorRanges = &rootRange;

		params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;				// t0
		params[1].Descriptor.RegisterSpace = 0;
		params[1].Descriptor.ShaderRegister = 0;

		params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;				// b0
		params[2].Descriptor.RegisterSpace = 0;
		params[2].Descriptor.ShaderRegister = 0;

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

	D3D12_DESCRIPTOR_RANGE uavRange{};
	uavRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	uavRange.NumDescriptors = 1;
	uavRange.BaseShaderRegister = 0;
	uavRange.RegisterSpace = 0;
	uavRange.OffsetInDescriptorsFromTableStart = 0;

	// 0 - ani info, 1 - InputVertex, 2 - OutputVertex
	D3D12_ROOT_PARAMETER params[3]{};
	params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[0].DescriptorTable.NumDescriptorRanges = 6;
	params[0].DescriptorTable.pDescriptorRanges = skinningRange;

	params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	params[1].Descriptor.RegisterSpace = 0;
	params[1].Descriptor.ShaderRegister = 5;

	params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	params[2].DescriptorTable.NumDescriptorRanges = 1;
	params[2].DescriptorTable.pDescriptorRanges = &uavRange;

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

void CRaytracingTestScene::SetUp()
{
	// Create Global & Local Root Signature
	CreateRootSignature();

	// animation Pipeline Ready
	CreateComputeRootSignature();
	CreateComputeShader();

	// Create And Set up PipelineState
	m_pRaytracingPipeline = std::make_unique<CRayTracingPipeline>();
	m_pRaytracingPipeline->Setup(1 + 1 + 1 + 2 + 1 + 1);
	m_pRaytracingPipeline->AddLibrarySubObject(compiledShader, std::size(compiledShader));
	m_pRaytracingPipeline->AddHitGroupSubObject(L"HitGroup", L"ClosestHit");
	m_pRaytracingPipeline->AddShaderConfigSubObject(8, 20);
	m_pRaytracingPipeline->AddLocalRootAndAsoociationSubObject(m_pLocalRootSignature.Get());
	m_pRaytracingPipeline->AddGlobalRootSignatureSubObject(m_pGlobalRootSignature.Get());
	m_pRaytracingPipeline->AddPipelineConfigSubObject(31);
	m_pRaytracingPipeline->MakePipelineState();

	// Resource Ready
	m_pResourceManager = std::make_unique<CResourceManager>();
	// 여기에 파일 넣기 ========================================	! 모든 파일은 한번씩만 읽기 !
	m_pResourceManager->AddResourceFromFile(L"src\\model\\City.bin", "src\\texture\\City\\");
	//m_pResourceManager->AddResourceFromFile(L"src\\model\\WinterLand2.bin", "src\\texture\\Map\\");
	m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Greycloak_33.bin", "src\\texture\\Greycloak\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Gorhorrid.bin", "src\\texture\\Gorhorrid\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Xenokarce.bin", "src\\texture\\Xenokarce\\");
	//m_pResourceManager->AddSkinningResourceFromFile(L"src\\model\\Lion.bin", "src\\texture\\Lion\\");
	// =========================================================

	std::vector<std::unique_ptr<CGameObject>>& normalObjects = m_pResourceManager->getGameObjectList();
	std::vector<std::unique_ptr<CSkinningObject>>& skinned = m_pResourceManager->getSkinningObjectList();
	std::vector<std::unique_ptr<Mesh>>& meshes = m_pResourceManager->getMeshList();
	std::vector<std::unique_ptr<CAnimationManager>>& aManagers = m_pResourceManager->getAnimationManagers();
	// 완전히 새로운 객체 & skinning Object 복사는 여기서 ========================================
	
	// 복사 예시
	//skinned.emplace_back(std::make_unique<CRayTracingSkinningObject>());
	//skinned[2]->CopyFromOtherObject(skinned[1].get());
	//aManagers.emplace_back(std::make_unique<CAnimationManager>(*aManagers[1].get()));
	//aManagers[2]->SetFramesPointerFromSkinningObject(skinned[2]->getObjects());
	//aManagers[2]->MakeAnimationMatrixIndex(skinned[2].get());
	//aManagers[2]->UpdateAnimation(0.5f);		// 필요 x

	// 객체 생성 예시
	/*m_pHeightMap = std::make_unique<CHeightMapImage>(L"src\\model\\asdf.raw", 2049, 2049, XMFLOAT3(1.0f, 0.025f, 1.0f));

	UINT finalindex = normalObjects.size();
	UINT finalmesh = meshes.size();
	Material tMaterial{};
	meshes.emplace_back(std::make_unique<Mesh>(m_pHeightMap.get(), "terrain"));
	normalObjects.emplace_back(std::make_unique<CGameObject>());
	normalObjects[finalindex]->SetMeshIndex(finalmesh);
	tMaterial.m_bHasAlbedoColor = true; tMaterial.m_xmf4AlbedoColor = XMFLOAT4(0.2, 0.4, 1.0, 1.0);
	normalObjects[finalindex]->getMaterials().emplace_back(tMaterial);
	normalObjects[finalindex]->SetPosition(XMFLOAT3(0.0, 0.0, 0.0));*/
	// ===========================================================================================
	m_pResourceManager->InitializeGameObjectCBuffer();	// 모든 오브젝트 상수버퍼 생성 & 초기화

	// ShaderBindingTable
	m_pShaderBindingTable = std::make_unique<CShaderBindingTableManager>();
	m_pShaderBindingTable->Setup(m_pRaytracingPipeline.get(), m_pResourceManager.get());
	m_pShaderBindingTable->CreateSBT();

	// 여기서 필요한 객체 복사 & 행렬 조작 ======================================================
	skinned[0]->SetPosition(XMFLOAT3(0.0f, 0.0f, 50.0f));
	//skinned[1]->setPreTransform(1.0, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3());
	//skinned[1]->SetPosition(XMFLOAT3(20.0f, 0.0f, 0.0f));
	skinned[0]->setPreTransform(2.0, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3());
	// ==============================================================================

	m_pResourceManager->PrepareObject();
	m_pResourceManager->getAnimationManagers()[0]->setCurrnetSet(24);

	// 카메라 설정 ==============================================================
	m_pCamera->SetTarget(skinned[0]->getObjects()[0].get());
	m_pCamera->SetCameraLength(10.0f);
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
		}
		break;
	}
}

void CRaytracingTestScene::MouseProcessing(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
	switch (nMessage) {
	case WM_RBUTTONDOWN:
	{
		if (!m_LockAnimation && !m_LockAnimation1) {
			auto& animationManagers = m_pResourceManager->getAnimationManagers();
			for (auto& animationManager : animationManagers) {
				animationManager->OnAttackInput();
			}
		}
		break;
	}
	}
}

void CRaytracingTestScene::ProcessInput(float fElapsedTime)
{
	UCHAR keyBuffer[256];
	GetKeyboardState(keyBuffer);

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

	if ((keyBuffer['W'] & 0x80) && (keyBuffer[VK_LSHIFT] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(8, true); // 뛰기
	}

	else if (keyBuffer['W'] & 0x80) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(5, true); // 걷기
	}
	else if (m_PrevKeyBuffer['W'] & 0x80) { // W 키를 뗐을 때
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(24, false);
	}

	if (keyBuffer['A'] & 0x80) {
		m_pResourceManager->getSkinningObjectList()[0]->Rotate(XMFLOAT3(0.0f, -180.0f * fElapsedTime, 0.0f)); //좌회전
	}
	else if (m_PrevKeyBuffer['A'] & 0x80) { // A 키를 뗐을 때
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(24, false);
	}

	if (keyBuffer['S'] & 0x80) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(5, false); //걷기
	}
	else if (m_PrevKeyBuffer['S'] & 0x80) { // S 키를 뗐을 때
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(24, false);
	}

	if (keyBuffer['D'] & 0x80) {
		m_pResourceManager->getSkinningObjectList()[0]->Rotate(XMFLOAT3(0.0f, 180.0f * fElapsedTime, 0.0f)); //우회전
	}
	else if (m_PrevKeyBuffer['D'] & 0x80) { // D 키를 뗐을 때
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(24, false);
	}
	if ((keyBuffer['J'] & 0x80) && !(m_PrevKeyBuffer['J'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(0, true); //약하게 맞기
	}
	if ((keyBuffer[VK_SPACE] & 0x80) && !(m_PrevKeyBuffer[VK_SPACE] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(10, true); //Dodge
		m_LockAnimation1 = true;
	}
	if ((keyBuffer['L'] & 0x80) && !(m_PrevKeyBuffer['L'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(2, true); //강하게 맞기
	}
	if ((keyBuffer['U'] & 0x80) && !(m_PrevKeyBuffer['U'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(3, true); //강하게 맞고 죽기
	}
	if ((keyBuffer['2'] & 0x80) && !(m_PrevKeyBuffer['2'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(22, true); //스킬2
		m_LockAnimation1 = true;
	}
	if ((keyBuffer['1'] & 0x80) && !(m_PrevKeyBuffer['1'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->ChangeAnimation(23, true); //스킬1
		m_LockAnimation1 = true;
	}
	if ((keyBuffer['3'] & 0x80) && !(m_PrevKeyBuffer['3'] & 0x80)) {
		m_pResourceManager->getAnimationManagers()[0]->StartSkill3(); //스킬3
		m_LockAnimation = true;
	}
	// 현재 키 상태를 이전 상태로 저장
	memcpy(m_PrevKeyBuffer, keyBuffer, sizeof(keyBuffer));
}