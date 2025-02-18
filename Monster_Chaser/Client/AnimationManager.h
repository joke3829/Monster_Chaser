#pragma once
#include "stdafx.h"

class CAnimationSet {
public:
private:
	std::string m_AnimationName{};
	float m_fLength{};
	int m_nFramePerSec{};
	int m_nKeyFrame{};

	// 키프레임별 행렬 행이 키 프레임 번호라 생각하면 됨
	std::vector<std::vector<XMFLOAT4X4>> m_vTransforms{};
	std::vector<float> m_vKeyTime{};	// 키 프레임 별 시간(?)
	// 이 두 놈을 이용해 행렬 보간
};

// 여기서 프레임 이름 별 포인터를 만들 예정
class CAnimationManager {
public:
protected:
	UINT m_nAnimationSets{};
	std::vector<std::string> m_vFrameNames{};

};

