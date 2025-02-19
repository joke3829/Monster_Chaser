#pragma once
#include "stdafx.h"
#include "GameObject.h"

class CAnimationSet {
public:
	CAnimationSet(std::ifstream& inFile, UINT nBones);
private:
	std::string m_AnimationName{};
	float m_fLength{};
	int m_nFramePerSec{};
	int m_nKeyFrame{};

	// Ű�����Ӻ� ��� ���� Ű ������ ��ȣ�� �����ϸ� ��
	// �� �� ������ �ΰ��� ������ shared�� ����
	std::vector<std::vector<XMFLOAT4X4>> m_vTransforms{};		// ���� Ű ������, ���� Ű������ �� ���
	std::vector<float> m_vKeyTime{};	// Ű ������ �� �ð�(?)
	// �� �� ���� �̿��� ��� ����
};

// ���⼭ ������ �̸� �� �����͸� ���� ����
class CAnimationManager {
public:
	CAnimationManager(std::ifstream& inFile, CSkinningObject* object);
protected:

	UINT m_nAnimationSets{};
	std::vector<std::string> m_vFrameNames{};		// �ѹ� ���� ������?
	std::vector<std::shared_ptr<CAnimationSet>> m_vAnimationSets{};

	std::vector<CGameObject*> m_vFrames{};	// ���� �� �� �ش� ��ü�� �°� ������Ʈ �ʿ�
};

