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
	void UpdateAnimationAndPosition(float fElapsedTime, CSkinningObject* player);
	void ChangeAnimation(UINT nSet);
	void ChangeAnimation(UINT nSet, bool playOnce = false); // playOnce �ɼ� �߰�
	void setCurrnetSet(UINT n) { m_nCurrentSet = n; }
	void setTimeZero() { m_fElapsedTime = 0.0f; }
	bool IsAnimationFinished() const { return m_bPlayOnce && m_fElapsedTime >= m_vAnimationSets[m_nCurrentSet]->getLength(); }


protected:
	UINT m_nAnimationSets{};
	UINT m_nCurrentSet{};
	float m_fElapsedTime{};
	std::vector<std::string> m_vFrameNames{};		// �ѹ� ���� ������?
	std::vector<std::shared_ptr<CAnimationSet>> m_vAnimationSets{};

	std::vector<CGameObject*> m_vFrames{};	// ���� �� �� �ش� ��ü�� �°� ������Ʈ �ʿ�
	
	// ������۸� ���⼭ �����?
	ComPtr<ID3D12Resource> m_pMatrixBuffer{};		// �ִϸ��̼� ����� ���� ��� ����
	void* m_pMappedPointer{};
	std::vector<XMFLOAT4X4> m_vMatrixes{};			// �ִϸ��̼� ����� ������ �迭
private:
	bool m_bPlayOnce = false; // �� ���� ��� ����
	CSkinningObject* m_pSkinningObject = nullptr; // ����� ������ �߰�
};

