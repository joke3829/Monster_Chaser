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

	m_IsBoss = isBoss;
}

void Stage1_Monster::Skill1()
{
	if (IsBoss())
	{
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
	}
	else
	{
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
	}
}

void Stage1_Monster::Skill2()
{
	if (IsBoss())
	{
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL2), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
	}
}

// ==============================================================

Stage2_Monster::Stage2_Monster(CSkinningObject* obj, CAnimationManager* aManager, bool isBoss)
	: Monster(obj, aManager, isBoss)
{
	if (isBoss) m_HP = 25000;
	else m_HP = 7000;

	m_IsBoss = isBoss;
}

void Stage2_Monster::Skill1()
{
	if (IsBoss())
	{
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
	}
	else
	{
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
	}
}

void Stage2_Monster::Skill2()
{
	if (IsBoss())
	{
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL2), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
	}
	else
	{
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL2), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
	}
}

void Stage2_Monster::Skill3()
{
	if (IsBoss())
	{
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL3), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
	}
}

// ==============================================================

Stage3_Monster::Stage3_Monster(CSkinningObject* obj, CAnimationManager* aManager, bool isBoss)
	: Monster(obj, aManager, isBoss)
{
	m_HP = 40000;
	m_IsBoss = isBoss;
}

void Stage3_Monster::Skill1()
{
	if (!m_IsSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_IsSkillActive = true;
	}
}

void Stage3_Monster::Skill2()
{
	m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL2), true);
	//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
	m_AManager->UpdateAniPosition(0.0f, m_Object);
}

void Stage3_Monster::Skill3()
{
	m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL3), true);
	//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
	m_AManager->UpdateAniPosition(0.0f, m_Object);
}

void Stage3_Monster::UpdateObject(float fElapsedTime)
{
	bool test = false;
	if (m_IsSkillActive) {
		if (m_AManager->IsAnimationInTimeRange(0.5f, 1.0f)) {
			//CheckCollision(); // 여기서 하는게 맞을까?
		}
		if (m_AManager->IsAnimationFinished()) {
			m_IsSkillActive = false;
		}
	}

	if (m_AManager->IsAnimationFinished()) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_IDLE), false);
		test = true;
	}

	if (test) {
		m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
	}
}
