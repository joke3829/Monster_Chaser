#pragma once
#include "ResourceManager.h"
#include "Camera.h"

class CPlayableCharacter {
public:
	CPlayableCharacter(CSkinningObject* object, CAnimationManager* aManager);

	// example
	virtual void Skill1() {}
	virtual void Skill2() {}
	virtual void Skill3() {}

	virtual void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}
	virtual void ProcessInput(UCHAR* keyBuffer) {}
	virtual void UpdateObject(float fElapsedTime) {}

	CSkinningObject* getObject() { return m_Object; }
	CPlayableCharacterAnimationManager* getAniManager() { return m_AManager; }

	void SetCamera(std::shared_ptr<CCamera>& camera) { m_pCamera = camera; }
protected:
	// stat
	float m_HP{};
	float m_MP{};

	CSkinningObject* m_Object{};
	CPlayableCharacterAnimationManager* m_AManager{};

	std::shared_ptr<CCamera> m_pCamera;
};

class CPlayerMage : public CPlayableCharacter {
public:
	CPlayerMage(CSkinningObject* object, CAnimationManager* aManager);

	void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void ProcessInput(UCHAR* keyBuffer);

	void UpdateObject(float fElapsedTime);
protected:
	bool m_bLockAnimation = false;
	bool m_bLockAnimation1 = false;
	bool m_bStopAnimaiton = false;
	bool m_bDoingCombo = false;
	bool m_bMoving = false;

	bool mouseIsInitialize = false;
	POINT oldCursor{};

	UCHAR m_PrevKeyBuffer[256]{};

	// personal Resource(bullet, particle etc.)
	CProjectile bullet{};
};

class CPlayerWarrior : public CPlayableCharacter {

};

class CPlayerPriest : public CPlayableCharacter {

};

// A real controlling player
class CPlayer {
public:
	CPlayer(CPlayableCharacter* playerObject, std::shared_ptr<CCamera>& camera);

	CPlayableCharacter* getObject() { return m_pPlayerObject; }
	CAnimationManager* getAniManager() { return m_pPlayerObject->getAniManager(); }

	void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void ProcessInput(UCHAR* keyBuffer);

	void HeightCheck(CHeightMapImage* heightmap, float fElapsedTime);
protected:
	CPlayableCharacter* m_pPlayerObject{};
	std::shared_ptr<CCamera> m_pCamera{};
};

