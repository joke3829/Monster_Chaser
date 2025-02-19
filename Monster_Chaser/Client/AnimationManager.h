#pragma once
#include "stdafx.h"
#include "GameObject.h"

class CAnimationSet {
public:
private:
	std::string m_AnimationName{};
	float m_fLength{};
	int m_nFramePerSec{};
	int m_nKeyFrame{};

	// 키프레임별 행렬 행이 키 프레임 번호라 생각하면 됨
	// 될 수 있으면 두개의 정보는 shared로 변경
	std::vector<std::vector<XMFLOAT4X4>> m_vTransforms{};		
	std::vector<float> m_vKeyTime{};	// 키 프레임 별 시간(?)
	// 이 두 놈을 이용해 행렬 보간
};

// 여기서 프레임 이름 별 포인터를 만들 예정
class CAnimationManager {
public:
	CAnimationManager(std::ifstream& inFile, CSkinningObject* object);
protected:

	UINT m_nAnimationSets{};
	std::vector<std::string> m_vFrameNames{};
	std::vector<CAnimationSet> m_vAnimationSets{};

	std::vector<CGameObject*> m_vFrames{};
};

