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
		readLabel();		// <Transforms>:
		inFile.read((char*)&tempInt, sizeof(UINT));
		inFile.read((char*)&tempFloat, sizeof(float));
		m_vKeyTime.push_back(tempFloat);
		inFile.read((char*)m_vTransforms[i].data(), sizeof(XMFLOAT4X4) * nBones);
	}
}

CAnimationManager::CAnimationManager(std::ifstream& inFile, CSkinningObject* object)
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
				m_vFrameNames.push_back(strLabel);
			}
			for (int i = 0; i < m_nAnimationSets; ++i)
				m_vAnimationSets.push_back(std::make_shared<CAnimationSet>(inFile, m_vFrameNames.size()));
		}
	}
}