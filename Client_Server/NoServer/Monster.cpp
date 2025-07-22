#include "Monster.h"

Feroptere::Feroptere(CSkinningObject* obj, CAnimationManager* aManager)
	: CPlayableCharacter(obj, aManager)
{
	m_HP = 5000;
}

void Feroptere::Skill1()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void Feroptere::Attacked(float damage)
{
	m_HP -= damage;
	m_bAttacked = true;
	if (m_HP > 0.0f)
	{
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_HIT), true);
	}
	else {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_DEATH), true);
	}
}

void Feroptere::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_bCheckAC = false;
	if (m_bLive) {
		if (m_AManager->IsAnimationFinished()) {
			m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_IDLE), true);
			test = true;
			m_bSkillActive = false;
			m_CurrentSkill = 0;
			m_bAttacked = false;
		}

		if (test) {
			m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
		}

		switch (getCurrentSkill())
		{
		case 1:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
			break;
		}
	}
}

// ==================================================

Pistriptere::Pistriptere(CSkinningObject* obj, CAnimationManager* aManager)
	: CPlayableCharacter(obj, aManager)
{
	m_HP = 5000;
}

void Pistriptere::Skill1()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void Pistriptere::Attacked(float damage)
{
	m_HP -= damage;
	m_bAttacked = true;
	if (m_HP > 0.0f)
	{
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_HIT), true);
	}
	else {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_DEATH), true);
	}
}

void Pistriptere::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_bCheckAC = false;
	if (m_bLive) {
		if (m_AManager->IsAnimationFinished()) {
			m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_IDLE), true);
			test = true;
			m_bSkillActive = false;
			m_CurrentSkill = 0;
			m_bAttacked = false;
		}

		if (test) {
			m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
		}

		switch (getCurrentSkill())
		{
		case 1:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
			break;
		}
	}
}

// ==================================================

RostrokarckLarvae::RostrokarckLarvae(CSkinningObject* obj, CAnimationManager* aManager)
	: CPlayableCharacter(obj, aManager)
{
	m_HP = 5000;
}

void RostrokarckLarvae::Skill1()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void RostrokarckLarvae::Attacked(float damage)
{
	m_HP -= damage;
	m_bAttacked = true;
	if (m_HP > 0.0f)
	{
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_HIT), true);
	}
	else {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_DEATH), true);
	}
}

void RostrokarckLarvae::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_bCheckAC = false;
	if (m_bLive) {
		if (m_AManager->IsAnimationFinished()) {
			m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_IDLE), true);
			test = true;
			m_bSkillActive = false;
			m_CurrentSkill = 0;
			m_bAttacked = false;
		}

		if (test) {
			m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
		}

		switch (getCurrentSkill())
		{
		case 1:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
			break;
		}
	}
}

// ==================================================

Xenokarce::Xenokarce(CSkinningObject* obj, CAnimationManager* aManager)
	: CPlayableCharacter(obj, aManager)
{
	m_HP = 15000;
}

void Xenokarce::Skill1()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void Xenokarce::Skill2()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL2), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void Xenokarce::Attacked(float damage)
{
	m_HP -= damage;
	m_bAttacked = true;
	if (m_HP > 0.0f)
	{
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_HIT), true);
	}
	else {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_DEATH), true);
	}
}

void Xenokarce::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_bCheckAC = false;
	if (m_bLive) {
		if (m_AManager->IsAnimationFinished()) {
			m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_IDLE), true);
			test = true;
			m_bSkillActive = false;
			m_CurrentSkill = 0;
			m_bAttacked = false;
		}

		if (test) {
			m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
		}

		switch (getCurrentSkill())
		{
		case 1:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
		case 2:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
			break;
		}
	}
}

// ==================================================

Fulgurodonte::Fulgurodonte(CSkinningObject* obj, CAnimationManager* aManager)
	: CPlayableCharacter(obj, aManager)
{
	m_HP = 7000;
}

void Fulgurodonte::Skill1()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void Fulgurodonte::Skill2()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL2), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 2;
	}
}

void Fulgurodonte::Attacked(float damage)
{
	m_HP -= damage;
	m_bAttacked = true;
	if (m_HP > 0.0f)
	{
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_HIT), true);
	}
	else {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_DEATH), true);
	}
}

void Fulgurodonte::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_bCheckAC = false;
	if (m_bLive) {
		if (m_AManager->IsAnimationFinished()) {
			m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_IDLE), true);
			test = true;
			m_bSkillActive = false;
			m_CurrentSkill = 0;
			m_bAttacked = false;
		}

		if (test) {
			m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
		}

		switch (getCurrentSkill())
		{
		case 1:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
		case 2:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
			break;
		}
	}
}

// ==================================================

Limadon::Limadon(CSkinningObject* obj, CAnimationManager* aManager)
	: CPlayableCharacter(obj, aManager)
{
	m_HP = 7000;
}

void Limadon::Skill1()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void Limadon::Skill2()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL2), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 2;
	}
}

void Limadon::Attacked(float damage)
{
	m_HP -= damage;
	m_bAttacked = true;
	if (m_HP > 0.0f)
	{
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_HIT), true);
	}
	else {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_DEATH), true);
	}
}

void Limadon::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_bCheckAC = false;
	if (m_bLive) {
		if (m_AManager->IsAnimationFinished()) {
			m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_IDLE), true);
			test = true;
			m_bSkillActive = false;
			m_CurrentSkill = 0;
			m_bAttacked = false;
		}

		if (test) {
			m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
		}

		switch (getCurrentSkill())
		{
		case 1:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
		case 2:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
			break;
		}
	}
}

// ==================================================

Occisodonte::Occisodonte(CSkinningObject* obj, CAnimationManager* aManager)
	: CPlayableCharacter(obj, aManager)
{
	m_HP = 7000;
}

void Occisodonte::Skill1()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void Occisodonte::Skill2()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_SKILL2), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 2;
	}
}

void Occisodonte::Attacked(float damage)
{
	m_HP -= damage;
	m_bAttacked = true;
	if (m_HP > 0.0f)
	{
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_HIT), true);
	}
	else {
		m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_DEATH), true);
	}
}

void Occisodonte::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_bCheckAC = false;
	if (m_bLive) {
		if (m_AManager->IsAnimationFinished()) {
			m_AManager->ChangeAnimation(static_cast<int>(Minion::ANI_IDLE), true);
			test = true;
			m_bSkillActive = false;
			m_CurrentSkill = 0;
			m_bAttacked = false;
		}

		if (test) {
			m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
		}

		switch (getCurrentSkill())
		{
		case 1:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
		case 2:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
			break;
		}
	}
}

// ==================================================

Crassorrid::Crassorrid(CSkinningObject* obj, CAnimationManager* aManager)
	: CPlayableCharacter(obj, aManager)
{
	m_HP = 25000;
}

void Crassorrid::Skill1()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL1), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 1;
	}
}

void Crassorrid::Skill2()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL2), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 2;
	}
}

void Crassorrid::Skill3()
{
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL3), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 3;
	}
}

void Crassorrid::Attacked(float damage)
{
	m_HP -= damage;
	m_bAttacked = true;
	if (m_HP > 0.0f)
	{
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_HIT), true);
	}
	else {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_DEATH), true);
	}
}

void Crassorrid::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_bCheckAC = false;
	if (m_bLive) {
		if (m_AManager->IsAnimationFinished()) {
			m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_IDLE), true);
			test = true;
			m_bSkillActive = false;
			m_CurrentSkill = 0;
			m_bAttacked = false;
		}

		if (test) {
			m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
		}

		switch (getCurrentSkill())
		{
		case 1:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
		case 2:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
		case 3:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
			break;
		}
	}
}

// ==============================================================

Gorhorrid::Gorhorrid(CSkinningObject* obj, CAnimationManager* aManager)
	: CPlayableCharacter(obj, aManager)
{
	m_HP = 40000;
}

void Gorhorrid::Skill1()
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

void Gorhorrid::Skill2()
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

void Gorhorrid::Skill3()
{
	//projectile
	if (!m_bSkillActive) {
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_SKILL3), true);
		//m_Object->SetLookDirection(characterDir, XMFLOAT3(0.0f, 1.0f, 0.0f)); //어디 보는지
		m_AManager->UpdateAniPosition(0.0f, m_Object);
		m_bSkillActive = true;
		m_CurrentSkill = 3;
		if (!bullet.empty()) {
			const int numProjectiles = 5;
			const float spreadAngle = 30.0f;
			float startAngle = -spreadAngle / 2.0f;
			float angleStep = spreadAngle / (numProjectiles - 1);
			XMFLOAT3 pos = XMFLOAT3(m_Head->getWorldMatrix()._41, m_Head->getWorldMatrix()._42, m_Head->getWorldMatrix()._43);

			for (int i = 0; i < numProjectiles && !bullet.empty(); ++i) {
				CProjectile* projectile = bullet[currentBullet].get();
				projectile->getObjects().SetScale(XMFLOAT3(2.5f, 2.5f, 2.5f));
				if (projectile && !projectile->getActive()) {
					projectile->setPosition(pos, 2);

					XMFLOAT3 lookDirection = m_Object->getLook();
					float angle = startAngle + (i * angleStep);
					float rad = angle * (3.14159265359f / 180.0f);
					XMFLOAT3 spreadDirection(lookDirection.x * cos(rad) - lookDirection.z * sin(rad), -0.15f , lookDirection.x * sin(rad) + lookDirection.z * cos(rad));

					projectile->setMoveDirection(spreadDirection);
					projectile->setActive(true);
					currentBullet = (currentBullet + 1) % bullet.size();
				}
			}
		}
	}
}

void Gorhorrid::Attacked(float damage)
{
	m_HP -= damage;
	m_bAttacked = true;
	if (m_HP > 0.0f)
	{
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_HIT), true);
	}
	else
	{
		m_bLive = false;
		m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_DEATH), true);
	}
}

void Gorhorrid::ProcessInput(UCHAR* keyBuffer, float fElapsedTime)
{
	if (m_bSkillActive) {
		memset(m_PrevKeyBuffer, 0, sizeof(m_PrevKeyBuffer));
		return;
	}

	if (!m_bSkillActive) {
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

void Gorhorrid::UpdateObject(float fElapsedTime)
{
	bool test = false;
	m_bCheckAC = false;
	if (m_bLive) {
		if (m_AManager->IsAnimationFinished()) {
			m_AManager->ChangeAnimation(static_cast<int>(Boss::ANI_IDLE), false);
			test = true;
			m_bSkillActive = false;
			m_CurrentSkill = 0;
			m_bAttacked = false;
		}

		if (test) {
			m_AManager->UpdateAniPosition(fElapsedTime, m_Object);
		}

		for (auto& bulletPtr : bullet) {
			if (bulletPtr->getActive()) {
				bulletPtr->IsMoving(fElapsedTime);
			}
		}

		switch (getCurrentSkill())
		{
		case 1:
			if (getAniManager()->IsAnimationInTimeRange(0.5f, 0.8f) || getAniManager()->IsAnimationInTimeRange(1.3f, 1.6f))
			{
				m_bCheckAC = true;
			}
			break;
		case 2:
			if (getAniManager()->IsAnimationInTimeRange(0.3f, 0.6f))
			{
				m_bCheckAC = true;
			}
			break;
		}
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

// ==================================================
