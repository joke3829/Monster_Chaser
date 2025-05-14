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
	virtual ~CAnimationManager() = default;

	void SetFramesPointerFromSkinningObject(std::vector<std::unique_ptr<CGameObject>>& vObjects);

	// Use Skinning Info to create indexes
	void MakeAnimationMatrixIndex(CSkinningObject* pSkinningObject);

	void TimeIncrease(float fElapsedTime);			
	void UpdateAnimation(float fElapsedTime);		
	void UpdateAnimationMatrix();
	void UpdateAniPosition(float fElapsedTime, CSkinningObject* player);
	void ChangeAnimation(UINT nSet);
	void ChangeAnimation(UINT nSet, bool playOnce = false); // playOnce 
	void setCurrnetSet(UINT n) { m_nCurrentSet = n; }

	std::shared_ptr<CAnimationSet>& getAnimationSet(int index) { return m_vAnimationSets[index]; }
	float getElapsedTime() const { return m_fElapsedTime; }
	UINT getCurrentSet() const { return m_nCurrentSet; }

	void setTimeZero() { m_fElapsedTime = 0.0f; }
	bool IsAnimationFinished() const { return m_bPlayOnce && m_fElapsedTime >= m_vAnimationSets[m_nCurrentSet]->getLength(); }
	bool IsAnimationNearEnd(float margin = 0.2f) const
	{
		float length = m_vAnimationSets[m_nCurrentSet]->getLength();
		float remainingTime = length - m_fElapsedTime;
		return remainingTime <= margin && remainingTime >= 0.0f;
	}
	const std::vector<CGameObject*>& getFrame() const { return m_vFrames; }

	bool CheckCollision() const { return m_bCollision; }
	void IsCollision() { m_bCollision = true; }
protected:
	UINT m_nAnimationSets{};
	UINT m_nCurrentSet{};
	float m_fElapsedTime{};
	std::vector<std::string> m_vFrameNames{};		// Bone Names
	std::vector<std::shared_ptr<CAnimationSet>> m_vAnimationSets{};

	std::vector<CGameObject*> m_vFrames{};	// Bones
	
	ComPtr<ID3D12Resource> m_pMatrixBuffer{};		// Matrix Buffer to be passed to shader
	void* m_pMappedPointer{};
	std::vector<XMFLOAT4X4> m_vMatrixes{};			// Matrix Buffer to be passed to shader

	bool m_bPlayOnce = false;
	bool m_bCollision = false;
};

class CPlayableCharacterAnimationManager : public CAnimationManager {
public:
	CPlayableCharacterAnimationManager(std::ifstream& inFile) : CAnimationManager(inFile) {}

	virtual void StartCombo() {}
	virtual void OnAttackInput() {}
	virtual void UpdateCombo(float fElapsedTime) {}
	virtual void ResetCombo() {}

	virtual void StartSkill3() {};
	virtual void OnKey3Input() {};

	bool IsInCombo() const { return m_bInCombo; }
	bool IsComboInterrupted() const { return m_bComboEnd; }
	void ClearComboInterrupted() { m_bComboEnd = false; }
protected:
	bool m_bInCombo;          
	int m_CurrentComboStep;
	std::vector<UINT> m_vComboAnimationSets;
	float m_fComboTimer;
	const float m_fComboWaitTime = 0.5f;
	bool m_bWaitingForNextInput;
	bool m_bNextAttack = false;
	bool m_bComboEnd = false;

	std::vector<UINT> m_vSkillAnimationSets;
};

class CMageManager : public CPlayableCharacterAnimationManager {
public:
	CMageManager(std::ifstream& inFile) : CPlayableCharacterAnimationManager(inFile) 
	{ m_vComboAnimationSets = { 22,23,24,25 }; m_vSkillAnimationSets = { 26, 27, 28, 29 , 30 };}

	virtual void StartCombo();
	virtual void OnAttackInput();
	virtual void UpdateCombo(float fElapsedTime);
	virtual void ResetCombo();

	virtual void StartSkill3();
	virtual void OnKey3Input();
};