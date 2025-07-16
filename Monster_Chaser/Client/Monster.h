#pragma once
#include "ResourceManager.h"

class Monster{
public:
	Monster(CSkinningObject* object, CAnimationManager* aManager,bool isBoss);

	// example
	virtual void Skill1() {}
	virtual void Skill2() {}
	virtual void Skill3() {}

	virtual void UpdateObject(float fElapsedTime) {}
	bool IsBoss() const { return m_IsBoss; }

	CSkinningObject* getObject() { return m_Object; }
	CAnimationManager* getAniManager() { return m_AManager; }

protected:
	// stat
	float m_HP{};
	bool m_IsBoss{ false };

	CSkinningObject* m_Object{};
	CAnimationManager* m_AManager{};

	bool m_IsSkillActive;
	float m_SkillStartTime;
	float m_SkillCollisionFrameStart;
	float m_SkillCollisionFrameEnd;
};

class Stage1_Monster : public Monster
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

class Stage2_Monster : public Monster
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

class Stage3_Monster : public Monster
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

	virtual void UpdateObject(float fElapsedTime);
};