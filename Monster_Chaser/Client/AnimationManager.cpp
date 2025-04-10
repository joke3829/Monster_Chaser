#include "AnimationManager.h"

CAnimationSet::CAnimationSet(std::ifstream& inFile, UINT nBones)
{
	UINT tempInt{};
	float tempFloat{};
	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};

	readLabel();	// <AnimationSet>:
	inFile.read((char*)&tempInt, sizeof(UINT));

	readLabel();
	m_AnimationName = strLabel;

	inFile.read((char*)&m_fLength, sizeof(float));
	inFile.read((char*)&m_nFramePerSec, sizeof(int));
	inFile.read((char*)&m_nKeyFrame, sizeof(int));

	m_vTransforms.assign(m_nKeyFrame, std::vector<XMFLOAT4X4>(nBones));
	for (int i = 0; i < m_nKeyFrame; ++i) {
		std::vector<XMFLOAT4X4>& test = m_vTransforms[i];
		readLabel();		// <Transforms>:
		inFile.read((char*)&tempInt, sizeof(UINT));
		inFile.read((char*)&tempFloat, sizeof(float));
		m_vKeyTime.emplace_back(tempFloat);
		inFile.read((char*)test.data(), sizeof(XMFLOAT4X4) * nBones);
	}
}

void CAnimationSet::UpdateAnimationMatrix(std::vector<CGameObject*>& vMatrixes, float fElapsedTime)
{
	auto p = std::find_if(m_vKeyTime.begin(), m_vKeyTime.end(), [&](float& time) {return time >= fElapsedTime; });
	if (p == m_vKeyTime.end())
		p -= 1;
	else if (p == m_vKeyTime.begin())
		p += 1;
	UINT index = std::distance(m_vKeyTime.begin(), p) - 1;
	// t (0 ~ 1)
	float t = (fElapsedTime - m_vKeyTime[index]) / (m_vKeyTime[index + 1] - m_vKeyTime[index]);
	// 행렬 보간
	for (int i = 0; i < vMatrixes.size(); ++i) {
		XMFLOAT4X4 xmf{};
		XMVECTOR S0, R0, T0, S1, R1, T1;
		XMMatrixDecompose(&S0, &R0, &T0, XMLoadFloat4x4(&m_vTransforms[index][i]));
		XMMatrixDecompose(&S1, &R1, &T1, XMLoadFloat4x4(&m_vTransforms[index + 1][i]));
		XMVECTOR S = XMVectorLerp(S0, S1, t);
		XMVECTOR T = XMVectorLerp(T0, T1, t);
		XMVECTOR R = XMQuaternionSlerp(R0, R1, t);
		//XMStoreFloat4x4(&xmf, XMMatrixTranspose(XMMatrixAffineTransformation(S, XMVectorZero(), R, T)));
		XMStoreFloat4x4(&xmf, XMMatrixAffineTransformation(S, XMVectorZero(), R, T));
		vMatrixes[i]->SetLocalMatrix(xmf);
	}
}

// ====================================================================================

CAnimationManager::CAnimationManager(std::ifstream& inFile)
{
	UINT tempInt{};
	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};

	while (1) {
		readLabel();
		if ("</Animation>" == strLabel)
			break;
		else if ("<AnimationSets>:" == strLabel) {
			inFile.read((char*)&m_nAnimationSets, sizeof(UINT));
			// <FrameNames>:
			readLabel();
			inFile.read((char*)&tempInt, sizeof(UINT));
			for (int i = 0; i < tempInt; ++i) {
				readLabel();
				m_vFrameNames.emplace_back(strLabel);
			}
			for (int i = 0; i < m_nAnimationSets; ++i)
				m_vAnimationSets.emplace_back(std::make_shared<CAnimationSet>(inFile, m_vFrameNames.size()));
		}
	}
	m_vMatrixes.assign(m_vFrameNames.size(), XMFLOAT4X4());
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = sizeof(XMFLOAT4X4) * m_vFrameNames.size();
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pMatrixBuffer.GetAddressOf()));
	m_pMatrixBuffer->Map(0, nullptr, &m_pMappedPointer);
}

CAnimationManager::CAnimationManager(const CAnimationManager& other)
{
	m_nAnimationSets = other.m_nAnimationSets;
	for (int i = 0; i < other.m_vFrameNames.size(); ++i)
		m_vFrameNames.emplace_back(other.m_vFrameNames[i]);
	for (int i = 0; i < m_nAnimationSets; ++i)
		m_vAnimationSets.emplace_back(other.m_vAnimationSets[i]);

	m_vMatrixes.assign(m_vFrameNames.size(), XMFLOAT4X4());
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = sizeof(XMFLOAT4X4) * m_vFrameNames.size();
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pMatrixBuffer.GetAddressOf()));
	m_pMatrixBuffer->Map(0, nullptr, &m_pMappedPointer);
}

void CAnimationManager::SetFramesPointerFromSkinningObject(std::vector<std::unique_ptr<CGameObject>>& vObjects)
{
	for (std::string& name : m_vFrameNames) {
		auto p = std::find_if(vObjects.begin(), vObjects.end(), [&](std::unique_ptr<CGameObject>& frame) {
			return frame->getFrameName() == name;
			});
		if (p != vObjects.end()) {
			m_vFrames.emplace_back((*p).get());
		}
	}
}

void CAnimationManager::MakeAnimationMatrixIndex(CSkinningObject* pSkinningObject)
{
	std::vector<std::unique_ptr<CSkinningInfo>>& skinningInfo = pSkinningObject->getSkinningInfo();
	for (std::unique_ptr<CSkinningInfo>& p : skinningInfo) {
		p->MakeAnimationMatrixIndex(m_vFrameNames);
		p->MakeBufferAndDescriptorHeap(m_pMatrixBuffer, m_vMatrixes.size());
	}
}

void CAnimationManager::TimeIncrease(float fElapsedTime)
{
	m_fElapsedTime += fElapsedTime;
	float length = m_vAnimationSets[m_nCurrnetSet]->getLength();
	while (m_fElapsedTime > length)
		m_fElapsedTime -= length;
	m_vAnimationSets[m_nCurrnetSet]->UpdateAnimationMatrix(m_vFrames, m_fElapsedTime);
}

void CAnimationManager::UpdateAnimation(float fElapsedTime)
{
	m_fElapsedTime = fElapsedTime;
	float length = m_vAnimationSets[m_nCurrnetSet]->getLength();
	while (m_fElapsedTime > length)
		m_fElapsedTime -= length;
	m_vAnimationSets[m_nCurrnetSet]->UpdateAnimationMatrix(m_vFrames, m_fElapsedTime);
}

void CAnimationManager::UpdateAnimationMatrix()
{
	for (int i = 0; i < m_vMatrixes.size(); ++i) 
		XMStoreFloat4x4(&m_vMatrixes[i], XMMatrixTranspose(XMLoadFloat4x4(&m_vFrames[i]->getAnimationMatrix())));
	memcpy(m_pMappedPointer, m_vMatrixes.data(), sizeof(XMFLOAT4X4) * m_vMatrixes.size());
}

void CAnimationManager::ChangeAnimation(UINT nSet)
{
	if (nSet != m_nCurrnetSet) {
		m_nCurrnetSet = nSet;
		m_fElapsedTime = 0.0f;
	}
}