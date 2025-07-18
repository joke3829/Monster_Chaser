#include "Monster.h"

Stage1_Monster::Stage1_Monster(CSkinningObject* obj, CAnimationManager* aManager, bool isBoss)
	: CPlayableCharacter(obj, aManager, isBoss)
{
	if (isBoss) m_HP = 15000;
	else m_HP = 5000;

	m_bBoss = isBoss;
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
	: CPlayableCharacter(obj, aManager, isBoss)
{
	if (isBoss) m_HP = 25000;
	else m_HP = 7000;

	m_bBoss = isBoss;
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
	: CPlayableCharacter(obj, aManager, isBoss)
{
	m_HP = 40000;
	m_bBoss = isBoss;
}

void Stage3_Monster::Skill1()
{
	// 0.5~0.8 && 1.3f~1.5f
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void Stage3_Monster::Skill2()
{
	//0.3 ~ 0.5f
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL2), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 2;
	}
}

void Stage3_Monster::Skill3()
{
	//projectile
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL3), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 3;
	}
}

void Stage3_Monster::ProcessInput(UCHAR* keyBuffer, float fElapsedTime)
{
	if (m_bSkillActive || m_bDoingCombo) {
		memset(m_PrevKeyBuffer, 0, sizeof(m_PrevKeyBuffer));
		return;
	}

	if (!m_bSkillActive && !m_bDoingCombo) {
		if ((keyBuffer['Z'] & 0x80) && !(m_PrevKeyBuffer['Z'] & 0x80)) {
			Skill1();
		}
		if ((keyBuffer['X'] & 0x80) && !(m_PrevKeyBuffer['X'] & 0x80)) {
			Skill2();
		}
		if ((keyBuffer['C'] & 0x80) && !(m_PrevKeyBuffer['C'] & 0x80)) {
			Skill3();
		}
	}
	memcpy(m_PrevKeyBuffer, keyBuffer, sizeof(m_PrevKeyBuffer));
}

void Stage3_Monster::UpdateObject(float fElapsedTime)
{
	bool test = false;
	if (m_AManager->IsAnimationFinished()) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_IDLE), false);
		test = true;
		m_bSkillActive = false;
		m_CurrentSkill = 0;
	}

	if (test) {
		m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
	}
}

// =============================================================================

CMonster::CMonster(CPlayableCharacter* monsterObject)
	: m_pMonsterObject(monsterObject)
{
}

void CMonster::ProcessInput(UCHAR* keyBuffer, float fElapsedTime)
{
	m_pMonsterObject->ProcessInput(keyBuffer, fElapsedTime);
}

void CMonster::HeightCheck(CHeightMapImage* heightmap, float fElapsedTime)
{
	CSkinningObject* p = m_pMonsterObject->getObject();
	XMFLOAT4X4& playerWorld = p->getWorldMatrix();
	XMFLOAT4X4& playerPreWorld = p->getPreWorldMatrix();
	XMFLOAT4X4& objectWorld = p->getObjects()[0]->getWorldMatrix();
	float fy = objectWorld._42 - (30 * fElapsedTime);

	float terrainHeight = heightmap->GetHeightinWorldSpace(objectWorld._41 + 1024.0f, objectWorld._43 + 1024.0f);
	if (objectWorld._43 >= -500.0f) {
		if (terrainHeight < 10.0f) {
			terrainHeight = 10.0f;
		}
	}
	if (fy < terrainHeight)
		playerWorld._42 = terrainHeight;
	else
		playerWorld._42 -= (30 * fElapsedTime);
	p->SetPosition(XMFLOAT3(playerWorld._41, playerWorld._42, playerWorld._43));
	playerPreWorld._42 = playerWorld._42;
}
