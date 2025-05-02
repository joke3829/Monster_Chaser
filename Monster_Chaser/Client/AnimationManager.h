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

	// Ű�����Ӻ� ��� ���� Ű ������ ��ȣ�� �����ϸ� ��
	// �� �� ������ �ΰ��� ������ shared�� ����
	std::vector<std::vector<XMFLOAT4X4>> m_vTransforms{};		// ���� Ű ������, ���� Ű������ �� ���
	std::vector<float> m_vKeyTime{};	// Ű ������ �� �ð�(?)
	// �� �� ���� �̿��� ��� ����
};

// ���⼭ ������ �̸� �� �����͸� ���� ����
class CAnimationManager {
public:
	CAnimationManager(std::ifstream& inFile);
	CAnimationManager(const CAnimationManager& other);

	void SetFramesPointerFromSkinningObject(std::vector<std::unique_ptr<CGameObject>>& vObjects);	// ��Ű�� �غ� �Լ�

	// SkinningInfo�� �ް� �� SkinningInfo�� ��� index ����� �ش�.
	void MakeAnimationMatrixIndex(CSkinningObject* pSkinningObject);

	void TimeIncrease(float fElapsedTime);			// �ð� ����
	void UpdateAnimation(float fElapsedTime);		// �ð� ����
	void UpdateAnimationMatrix();
	void ChangeAnimation(UINT nSet);

	std::shared_ptr<CAnimationSet>& getAnimationSet(int index) { return m_vAnimationSets[index]; }

	void setCurrnetSet(UINT n) { m_nCurrnetSet = n; }
	void setTimeZero() { m_fElapsedTime = 0.0f; }
protected:
	UINT m_nAnimationSets{};
	UINT m_nCurrnetSet{};
	float m_fElapsedTime{};
	std::vector<std::string> m_vFrameNames{};		// �ѹ� ���� ������?
	std::vector<std::shared_ptr<CAnimationSet>> m_vAnimationSets{};

	std::vector<CGameObject*> m_vFrames{};	// ���� �� �� �ش� ��ü�� �°� ������Ʈ �ʿ�
	
	// ������۸� ���⼭ �����?
	ComPtr<ID3D12Resource> m_pMatrixBuffer{};		// �ִϸ��̼� ����� ���� ��� ����
	void* m_pMappedPointer{};
	std::vector<XMFLOAT4X4> m_vMatrixes{};			// �ִϸ��̼� ����� ������ �迭
};

