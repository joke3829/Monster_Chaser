#pragma once
#include "stdafx.h"
#include "GameObject.h"

extern DXResources g_DxResource;

class CAnimationSet {
public:
	CAnimationSet(std::ifstream& inFile, UINT nBones);

	void UpdateAnimationMatrix(std::vector<CGameObject*>& vMatrixes, float fElapsedTime);

	float getLength() const { return m_fLength; }
	int getNumKeyFrame() const { return m_nKeyFrame; }
	std::vector<std::vector<XMFLOAT4X4>>& getTransforms() { return m_vTransforms; }

private:
	std::string m_AnimationName{};
	float m_fLength{};
	int m_nFramePerSec{};
	int m_nKeyFrame{};


	std::vector<std::vector<XMFLOAT4X4>> m_vTransforms{};	// Row : KeyFrame, Col : AnimationMatrix by Frames
	std::vector<float> m_vKeyTime{};	// Time by KeyFrame
};


class CAnimationManager {
public:
	CAnimationManager(std::ifstream& inFile);
	CAnimationManager(const CAnimationManager& other);

	void SetFramesPointerFromSkinningObject(std::vector<std::unique_ptr<CGameObject>>& vObjects);	// ��Ű�� �غ� �Լ�

	// Use Skinning Info to create indexes
	void MakeAnimationMatrixIndex(CSkinningObject* pSkinningObject);

	void TimeIncrease(float fElapsedTime);			
	void UpdateAnimation(float fElapsedTime);		
	void UpdateAnimationMatrix();
	void UpdateAniPosition(float fElapsedTime, CSkinningObject* player);
	void ChangeAnimation(UINT nSet);
	void ChangeAnimation(UINT nSet, bool playOnce = false); // playOnce �ɼ� �߰�
	void setCurrnetSet(UINT n) { m_nCurrentSet = n; }

	std::shared_ptr<CAnimationSet>& getAnimationSet(int index) { return m_vAnimationSets[index]; }

	void setTimeZero() { m_fElapsedTime = 0.0f; }
	void setAnimationTime(float fElapsedTime) {m_fElapsedTime = fElapsedTime;}
	bool IsAnimationFinished() const { return m_bPlayOnce && m_fElapsedTime >= m_vAnimationSets[m_nCurrentSet]->getLength(); }
	bool IsAnimationNearEnd(float margin = 0.2f) const
	{
		float length = m_vAnimationSets[m_nCurrentSet]->getLength();
		float remainingTime = length - m_fElapsedTime;
		return remainingTime <= margin && remainingTime >= 0.0f; //������ 0.0 ~ 0.2�� ������ Ȯ��
	}

	virtual void StartCombo() {};
	virtual void OnAttackInput() {};
	virtual void UpdateCombo(float fElapsedTime) {};
	virtual void ResetCombo() {};
	bool IsInCombo() const { return m_bInCombo; } // �޺� ���� �� ����
	bool CheckCollision() const { return m_bCollision; }
	void IsCollision() { m_bCollision = true; }

	virtual void StartSkill3() {};
	virtual void OnKey3Input() {};

	bool IsComboInterrupted() const { return m_bComboEnd; }
	void ClearComboInterrupted() { m_bComboEnd = false; }

	const std::vector<CGameObject*>& getFrame()const { return m_vFrames; }
	UINT& getCurrentSet() {return m_nCurrentSet;}
	float getElapsedTime() const { return m_fElapsedTime; }
protected:
	UINT m_nAnimationSets{};
	UINT m_nCurrentSet{};
	float m_fElapsedTime{};
	std::vector<std::string> m_vFrameNames{};		// Bone Names
	std::vector<std::shared_ptr<CAnimationSet>> m_vAnimationSets{};

	std::vector<CGameObject*> m_vFrames{};	// ���� �� �� �ش� ��ü�� �°� ������Ʈ �ʿ�
	
	// ������۸� ���⼭ �����?
	ComPtr<ID3D12Resource> m_pMatrixBuffer{};		// �ִϸ��̼� ����� ���� ��� ����
	void* m_pMappedPointer{};
	std::vector<XMFLOAT4X4> m_vMatrixes{};			// �ִϸ��̼� ����� ������ �迭

	bool m_bPlayOnce = false; // �� ���� ��� ����
    bool m_bCollision = false;

	// �޺�
	bool m_bInCombo;                     // �޺� ���� �� ����
	int m_CurrentComboStep;               // ���� �޺� �ܰ�
	std::vector<UINT> m_vComboAnimationSets; // �޺� �ִϸ��̼� ��Ʈ
	float m_fComboTimer;                   // �޺� �Է� ��� �ð�
	const float m_fComboWaitTime = 0.5f;     // ���� �Է��� ��ٸ��� �ð�
	bool m_bWaitingForNextInput;         // ���� �Է� ��� ����
	bool m_bNextAttack = false;			// ���� ���� ��û ����
	bool m_bComboEnd = false;			// �޺� ���� �ߴ� ����

	std::vector<UINT> m_vSkillAnimationSets; //��ų �ִϸ��̼� ��Ʈ
};

class CMageManager : public CAnimationManager //������ ����
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