#pragma once
#include "ResourceManager.h"
#include "Camera.h"

class CPlayableCharacter {
public:
	CPlayableCharacter(CSkinningObject* object, CAnimationManager* aManager,bool isBoss);

	// example
	virtual void Skill1() {}
	virtual void Skill2() {}
	virtual void Skill3() {}																																							  
																																															
	virtual void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam) {}														  
	virtual void ProcessInput(UCHAR* keyBuffer,float fElapsedTime) {}																									
	virtual void UpdateObject(float fElapsedTime) {}

	bool IsBoss() const { return m_IsBoss; }																																	  
																																															
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
																																															
	bool m_IsBoss = false;																																							
	bool m_IsSkillActive = false;																																					  																																		
	bool m_bDoingCombo = false;
	bool m_bMoving = false;

	bool mouseIsInitialize = false;
	POINT oldCursor{};

	UCHAR m_PrevKeyBuffer[256]{};
};

class CPlayerMage : public CPlayableCharacter {
public:
	enum class MageAni {
		ANI_IDLE,
		ANI_HIT,
		ANI_HIT_DEATH,
		ANI_BIGHIT,
		ANI_BIGHIT_DEATH,
		ANI_WALK_FORWARD,
		ANI_WALK_LEFT_UP,
		ANI_WALK_RIGHT_UP,
		ANI_WALK_LEFT,
		ANI_WALK_RIGHT,
		ANI_WALK_BACKWARD,
		ANI_WALK_LEFT_DOWN,
		ANI_WALK_RIGHT_DOWN,
		ANI_RUN_FORWARD,
		ANI_RUN_LEFT_UP,
		ANI_RUN_RIGHT_UP,
		ANI_RUN_LEFT,
		ANI_RUN_RIGHT,
		ANI_RUN_BACKWARD,
		ANI_RUN_LEFT_DOWN,
		ANI_RUN_RIGHT_DOWN,
		ANI_DODGE,
		ANI_C_ATTACK1,
		ANI_C_ATTACK2,
		ANI_C_ATTACK3,
		ANI_C_ATTACK4,
		ANI_SKILL3_1,
		ANI_SKILL3_2,
		ANI_SKILL3_3,
		ANI_SKILL3_4,
		ANI_SKILL3_5,
		ANI_SKILL2,
		ANI_SKILL1
	};
	virtual void Skill1();
	virtual void Skill2();
	virtual void Skill3();

	CPlayerMage(CSkinningObject* object, CAnimationManager* aManager, bool isBoss);

	void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void ProcessInput(UCHAR* keyBuffer,float fElapsedTime);

	void UpdateObject(float fElapsedTime);
protected:
	// personal Resource(bullet, particle etc.)
	CProjectile bullet{};
};

class CPlayerWarrior : public CPlayableCharacter {
public:
	enum class WarriorAni {
		ANI_IDLE,
		ANI_HIT,
		ANI_BIGHIT,
		ANI_DEATH,
		ANI_WALK_FORWARD,
		ANI_WALK_LEFT_UP,
		ANI_WALK_RIGHT_UP,
		ANI_WALK_LEFT,
		ANI_WALK_RIGHT,
		ANI_WALK_BACKWARD,
		ANI_WALK_LEFT_DOWN,
		ANI_WALK_RIGHT_DOWN,
		ANI_RUN_FORWARD,
		ANI_RUN_LEFT_UP,
		ANI_RUN_RIGHT_UP,
		ANI_RUN_LEFT,
		ANI_RUN_RIGHT,
		ANI_RUN_BACKWARD,
		ANI_RUN_LEFT_DOWN,
		ANI_RUN_RIGHT_DOWN,
		ANI_DODGE,
		ANI_SKILL2,
		ANI_SKILL1,
		ANI_C_ATTACK1,
		ANI_C_ATTACK2,
		ANI_C_ATTACK3,
		ANI_SKILL3_1,
		ANI_SKILL3_2
	};
	virtual void Skill1();
	virtual void Skill2();
	virtual void Skill3();

	CPlayerWarrior(CSkinningObject* object, CAnimationManager* aManager, bool isBoss);

	void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);

	void UpdateObject(float fElapsedTime);
};

class CPlayerPriest : public CPlayableCharacter {
public:
	enum class PriestAni {
		ANI_IDLE,
		ANI_HIT,
		ANI_HIT_DEATH,
		ANI_BIGHIT,
		ANI_BIGHIT_DEATH,
		ANI_WALK_FORWARD,
		ANI_WALK_LEFT_UP,
		ANI_WALK_RIGHT_UP,
		ANI_WALK_LEFT,
		ANI_WALK_RIGHT,
		ANI_WALK_BACKWARD,
		ANI_WALK_LEFT_DOWN,
		ANI_WALK_RIGHT_DOWN,
		ANI_RUN_FORWARD,
		ANI_RUN_LEFT_UP,
		ANI_RUN_RIGHT_UP,
		ANI_RUN_LEFT,
		ANI_RUN_RIGHT,
		ANI_RUN_BACKWARD,
		ANI_RUN_LEFT_DOWN,
		ANI_RUN_RIGHT_DOWN,
		ANI_DODGE,
		ANI_C_ATTACK1,
		ANI_C_ATTACK2,
		ANI_C_ATTACK3,
		ANI_C_ATTACK4,
		ANI_SKILL1,
		ANI_SKILL2,
		ANI_SKILL3_1,
		ANI_SKILL3_2,
		ANI_SKILL3_3
	};
	virtual void Skill1();
	virtual void Skill2();
	virtual void Skill3();

	CPlayerPriest(CSkinningObject* object, CAnimationManager* aManager, bool isBoss);

	void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);

	void UpdateObject(float fElapsedTime);
protected:
	CProjectile bullet{};
};

// A real controlling player
class CPlayer {
public:
	CPlayer(CPlayableCharacter* playerObject, std::shared_ptr<CCamera>& camera);

	CPlayableCharacter* getObject() { return m_pPlayerObject; }
	CAnimationManager* getAniManager() { return m_pPlayerObject->getAniManager(); }

	void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void ProcessInput(UCHAR* keyBuffer,float fElapsedTime);

	void HeightCheck(CHeightMapImage* heightmap, float fElapsedTime);
protected:
	CPlayableCharacter* m_pPlayerObject{};
	std::shared_ptr<CCamera> m_pCamera{};
};

