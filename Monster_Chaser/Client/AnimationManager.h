#pragma once
#include "stdafx.h"

class CAnimationSet {
public:
private:
	std::string m_AnimationName{};
	float m_fLength{};
	int m_nFramePerSec{};
	int m_nKeyFrame{};

	// Ű�����Ӻ� ��� ���� Ű ������ ��ȣ�� �����ϸ� ��
	std::vector<std::vector<XMFLOAT4X4>> m_vTransforms{};
	std::vector<float> m_vKeyTime{};	// Ű ������ �� �ð�(?)
	// �� �� ���� �̿��� ��� ����
};

// ���⼭ ������ �̸� �� �����͸� ���� ����
class CAnimationManager {
public:
protected:
	UINT m_nAnimationSets{};
	std::vector<std::string> m_vFrameNames{};

};

