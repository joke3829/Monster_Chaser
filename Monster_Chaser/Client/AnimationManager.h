#pragma once
#include "stdafx.h"
#include "GameObject.h"

extern DXResources g_DxResource;

class CAnimationSet {
public:
	CAnimationSet(std::ifstream& inFile, UINT nBones);

	void UpdateAnimationMatrix(std::vector<CGameObject*>& vMatrixes, float fElapsedTime);

	float getLength() const { return m_fLength; }

private:
	std::string m_AnimationName{};
	float m_fLength{};
	int m_nFramePerSec{};
	int m_nKeyFrame{};

	// 키프레임별 행렬 행이 키 프레임 번호라 생각하면 됨
	// 될 수 있으면 두개의 정보는 shared로 변경
	std::vector<std::vector<XMFLOAT4X4>> m_vTransforms{};		// 행이 키 프레임, 열이 키프레임 별 행렬
	std::vector<float> m_vKeyTime{};	// 키 프레임 별 시간(?)
	// 이 두 놈을 이용해 행렬 보간
};

// 여기서 프레임 이름 별 포인터를 만들 예정
class CAnimationManager {
public:
	CAnimationManager(std::ifstream& inFile);
	CAnimationManager(const CAnimationManager& other);

	void SetFramesPointerFromSkinningObject(std::vector<std::unique_ptr<CGameObject>>& vObjects);	// 스키닝 준비 함수

	// SkinningInfo를 받고 그 SkinningInfo의 행렬 index 만들어 준다.
	void MakeAnimationMatrixIndex(CSkinningObject* pSkinningObject);

	void TimeIncrease(float fElapsedTime);			// 시간 증가
	void UpdateAnimation(float fElapsedTime);		// 시간 지정
	void UpdateAnimationMatrix();
	void UpdateAniPosition(float fElapsedTime, CSkinningObject* player);
	bool CheckSphereAABBCollision(std::vector<CGameObject*>& map);
	void ChangeAnimation(UINT nSet);
	void ChangeAnimation(UINT nSet, bool playOnce = false); // playOnce 옵션 추가
	void setCurrnetSet(UINT n) { m_nCurrentSet = n; }
	void setTimeZero() { m_fElapsedTime = 0.0f; }
	bool IsAnimationFinished() const { return m_bPlayOnce && m_fElapsedTime >= m_vAnimationSets[m_nCurrentSet]->getLength(); }
	bool IsAnimationNearEnd(float margin = 0.2f) const
	{
		float length = m_vAnimationSets[m_nCurrentSet]->getLength();
		float remainingTime = length - m_fElapsedTime;
		return remainingTime <= margin && remainingTime >= 0.0f; //끝나기 0.0 ~ 0.2초 전인지 확인
	}

	virtual void StartCombo() {};
	virtual void OnAttackInput() {};
	virtual void UpdateCombo(float fElapsedTime) {};
	virtual void ResetCombo() {};
	bool IsInCombo() const { return m_bInCombo; } // 콤보 진행 중 여부

	virtual void StartSkill3() {};
	virtual void OnKey3Input() {};

	bool IsComboInterrupted() const { return m_bComboEnd; }
	void ClearComboInterrupted() { m_bComboEnd = false; }

	const std::vector<CGameObject*>& getFrame()const { return m_vFrames; }
protected:
	UINT m_nAnimationSets{};
	UINT m_nCurrentSet{};
	float m_fElapsedTime{};
	std::vector<std::string> m_vFrameNames{};		// 한번 쓰고 버리나?
	std::vector<std::shared_ptr<CAnimationSet>> m_vAnimationSets{};

	std::vector<CGameObject*> m_vFrames{};	// 복사 할 때 해당 객체에 맞게 업데이트 필요
	
	// 상수버퍼를 여기서 만들까?
	ComPtr<ID3D12Resource> m_pMatrixBuffer{};		// 애니메이션 행렬을 넣을 상수 버퍼
	void* m_pMappedPointer{};
	std::vector<XMFLOAT4X4> m_vMatrixes{};			// 애니메이션 행렬을 저장할 배열

	bool m_bPlayOnce = false; // 한 번만 재생 여부
	bool m_isWalk = false; // 걷기 여부

	// 콤보
	bool m_bInCombo;                     // 콤보 진행 중 여부
	int m_CurrentComboStep;               // 현재 콤보 단계
	std::vector<UINT> m_vComboAnimationSets; // 콤보 애니메이션 세트
	float m_fComboTimer;                   // 콤보 입력 대기 시간
	const float m_fComboWaitTime = 0.5f;     // 다음 입력을 기다리는 시간
	bool m_bWaitingForNextInput;         // 다음 입력 대기 여부
	bool m_bNextAttack = false;			// 다음 공격 요청 여부
	bool m_bComboEnd = false;			// 콤보 공격 중단 여부

	std::vector<UINT> m_vSkillAnimationSets; //스킬 애니메이션 세트
};

class CMageManager : public CAnimationManager //마법사 전용
{
public:
	CMageManager(std::ifstream& inFile) : CAnimationManager(inFile) { m_vComboAnimationSets = { 22,23,24,25 }; m_vSkillAnimationSets = { 26, 27, 28, 29 , 30 };};
	virtual void StartCombo();
	virtual void OnAttackInput();
	virtual void UpdateCombo(float fElapsedTime);
	virtual void ResetCombo();

	virtual void StartSkill3();
	virtual void OnKey3Input();
};