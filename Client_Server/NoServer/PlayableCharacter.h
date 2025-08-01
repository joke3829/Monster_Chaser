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
	virtual void ProcessInput(UCHAR* keyBuffer, float fElapsedTime) {}
	virtual void UpdateObject(float fElapsedTime) {}

	virtual void Attacked(float damage) {}

	bool IsAttacking()const { return m_bSkillActive; }
	bool IsCombo()const { return m_bDoingCombo; }
	bool IsOnceAttacked()const { return m_bAttacked; }
	bool CheckAC() const { return m_bCheckAC; }

	int getCurrentSkill() const { return m_CurrentSkill; }
	float getCurrentDamage()const { return m_Damage; }
	CSkinningObject* getObject() { return m_Object; }
	CPlayableCharacterAnimationManager* getAniManager() { return m_AManager; }

	void SetCamera(std::shared_ptr<CCamera>& camera) { m_pCamera = camera; }
	void SetHead(CGameObject* h) { m_Head = h; }

	virtual bool HasActiveBullet() const { return false; }

	virtual std::vector<std::unique_ptr<CProjectile>>& GetBullets() {
		static std::vector<std::unique_ptr<CProjectile>> empty;
		return empty;
	}
	virtual const std::vector<std::unique_ptr<CProjectile>>& GetBullets() const {
		static std::vector<std::unique_ptr<CProjectile>> empty;
		return empty;
	}
protected:
	// stat																																													
	float m_HP{};
	float m_MP{};
	int m_CurrentSkill = 0;
	float m_Damage{};

	CSkinningObject* m_Object{};
	CPlayableCharacterAnimationManager* m_AManager{};
	CGameObject* m_Head{};

	std::shared_ptr<CCamera> m_pCamera;

	bool m_bSkillActive = false;
	bool m_bDoingCombo = false;
	bool m_bMoving = false;
	bool m_bLive = true;
	bool m_bAttacked = false;

	bool m_bCheckAC = false;

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

	virtual void Attacked(float damage);

	CPlayerMage(CSkinningObject* object, CAnimationManager* aManager);

	void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);

	void UpdateObject(float fElapsedTime);

	void MakeBullet(float speed=50.0f, int skill=1);

	virtual bool HasActiveBullet() const
	{
		for (const auto& bullet : bullet)
		{
			if (bullet && bullet->getActive())
			{
				return true;
			}
		}
		return false;
	}

	std::vector<std::unique_ptr<CProjectile>>& GetBullets() { return bullet; }
	const std::vector<std::unique_ptr<CProjectile>>& GetBullets() const { return bullet; }
protected:
	// personal Resource(bullet, particle etc.)

	std::vector<std::unique_ptr<CProjectile>> bullet;
	int currentBullet = 0;

	bool m_bBulletFired[5] = { false, false, false, false, false };
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

	virtual void Attacked(float damage);

	CPlayerWarrior(CSkinningObject* object, CAnimationManager* aManager);

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

	virtual void Attacked(float damage);

	CPlayerPriest(CSkinningObject* object, CAnimationManager* aManager);

	void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);

	void UpdateObject(float fElapsedTime);

	void MakeBullet(float speed = 50.0f, int skill = 1);

	virtual bool HasActiveBullet() const
	{
		for (const auto& bullet : bullet)
		{
			if (bullet && bullet->getActive())
			{
				return true;
			}
		}
		return false;
	}

	std::vector<std::unique_ptr<CProjectile>>& GetBullets() { return bullet; }
	const std::vector<std::unique_ptr<CProjectile>>& GetBullets() const { return bullet; }
protected:
	std::vector<std::unique_ptr<CProjectile>> bullet;
	int currentBullet = 0;
};

// A real controlling player
class CPlayer {
public:
	CPlayer(CPlayableCharacter* playerObject, std::shared_ptr<CCamera>& camera);

	CPlayableCharacter* getObject() { return m_pPlayerObject; }
	CAnimationManager* getAniManager() { return m_pPlayerObject->getAniManager(); }

	void MouseProcess(HWND hWnd, UINT nMessage, WPARAM wParam, LPARAM lParam);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);

	void HeightCheck(CHeightMapImage* heightmap, float fElapsedTime, float offsetx, float offsety, float offsetz, short mapNum);
	void CollisionCheck(CHeightMapImage* heightmap, float fElapsedTime, float offsetx, float offsety, float offsetz, short mapNum);
	void CollisionCheck(CHeightMapImage* heightmap, CHeightMapImage* CollisionMap, float fElapsedTime, float offsetx, float offsety, float offsetz, short mapNum);
protected:
	CPlayableCharacter* m_pPlayerObject{};
	std::shared_ptr<CCamera> m_pCamera{};
	XMFLOAT2 m_xmf2PrevPos{};
};

