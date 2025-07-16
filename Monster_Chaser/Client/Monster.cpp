#include "Monster.h"

Monster::Monster(CSkinningObject* object, CAnimationManager* aManager, bool isBoss)
	: m_Object(object)
{
		m_AManager = dynamic_cast<CAnimationManager*>(aManager);
		m_IsBoss = isBoss;
}

// ==============================================================

Stage1_Monster::Stage1_Monster(CSkinningObject* obj, CAnimationManager* aManager, bool isBoss)
	: Monster(obj, aManager, isBoss)
{
	if (isBoss) m_HP = 15000;
	else m_HP = 5000;
}

void Stage1_Monster::Skill1()
{
}

void Stage1_Monster::Skill2()
{
}

// ==============================================================

Stage2_Monster::Stage2_Monster(CSkinningObject* obj, CAnimationManager* aManager, bool isBoss)
	: Monster(obj, aManager, isBoss)
{
	if (isBoss) m_HP = 25000;
	else m_HP = 7000;
}

void Stage2_Monster::Skill1()
{

}

void Stage2_Monster::Skill2()
{
}

void Stage2_Monster::Skill3()
{
}

// ==============================================================

Stage3_Monster::Stage3_Monster(CSkinningObject* obj, CAnimationManager* aManager, bool isBoss)
	: Monster(obj, aManager, isBoss)
{
	if (isBoss) m_HP = 40000;
}

void Stage3_Monster::Skill1()
{
}

void Stage3_Monster::Skill2()
{
}

void Stage3_Monster::Skill3()
{
}
