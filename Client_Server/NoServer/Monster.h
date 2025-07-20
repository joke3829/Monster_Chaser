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