#pragma once
#include "ResourceManager.h"
#include "PlayableCharacter.h"

class Stage1_Monster : public CPlayableCharacter
{
public:
	enum class Minion {
		ANI_DEATH,
		ANI_HIT,
		ANI_IDLE,
		ANI_ROAR,
		ANI_SKILL1,
		ANI_FRONT,
		ANI_BACK
	};

	enum class Boss {
		ANI_DEATH,
		ANI_HIT,
		ANI_IDLE,
		ANI_ROAR,
		ANI_SKILL1,
		ANI_SKILL2,
		ANI_FRONT,
		ANI_BACK
	};
	Stage1_Monster(CSkinningObject* obj, CAnimationManager* aManager, bool isBoss);

	virtual void Skill1();
	virtual void Skill2();

	virtual void Attacked(float damage);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);

	virtual void UpdateObject(float fElapsedTime);
};

class Stage2_Monster : public CPlayableCharacter
{
public:
	enum class Minion {
		ANI_DEATH,
		ANI_HIT,
		ANI_IDLE,
		ANI_ROAR,
		ANI_SKILL1,
		ANI_SKILL2,
		ANI_FRONT,
		ANI_BACK
	};

	enum class Boss {
		ANI_DEATH,
		ANI_HIT,
		ANI_IDLE,
		ANI_ROAR,
		ANI_SKILL1,
		ANI_SKILL2,
		ANI_SKILL3,
		ANI_FRONT,
		ANI_BACK
	};
	Stage2_Monster(CSkinningObject* obj, CAnimationManager* aManager, bool isBoss);

	virtual void Skill1();
	virtual void Skill2();
	virtual void Skill3();

	virtual void Attacked(float damage);

	virtual void UpdateObject(float fElapsedTime);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);

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
};

class Stage3_Monster : public CPlayableCharacter
{
public:
	enum class Boss {
		ANI_DEATH,
		ANI_HIT,
		ANI_IDLE,
		ANI_ROAR,
		ANI_SKILL1,
		ANI_SKILL2,
		ANI_SKILL3,
		ANI_RUN,
		ANI_FRONT,
		ANI_BACK
	};
	Stage3_Monster(CSkinningObject* obj, CAnimationManager* aManager, bool isBoss);

	virtual void Skill1();
	virtual void Skill2();
	virtual void Skill3();

	virtual void Attacked(float damage);

	virtual void UpdateObject(float fElapsedTime);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);

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
};

// A real controlling monster
class CMonster {
public:
	CMonster(CPlayableCharacter* monsterObject);

	CPlayableCharacter* getObject() { return m_pMonsterObject; }
	CAnimationManager* getAniManager() { return m_pMonsterObject->getAniManager(); }

	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);
	void HeightCheck(CHeightMapImage* heightmap, float fElapsedTime);
protected:
	CPlayableCharacter* m_pMonsterObject{};

};