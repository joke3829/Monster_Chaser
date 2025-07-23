#pragma once
#include "ResourceManager.h"
#include "PlayableCharacter.h"

class Feroptere : public CPlayableCharacter
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

	Feroptere(CSkinningObject* obj, CAnimationManager* aManager);
	virtual void Skill1();

	virtual void Attacked(float damage);

	virtual void UpdateObject(float fElapsedTime);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);
};

class Pistriptere : public CPlayableCharacter
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

	Pistriptere(CSkinningObject* obj, CAnimationManager* aManager);
	virtual void Skill1();

	virtual void Attacked(float damage);

	virtual void UpdateObject(float fElapsedTime);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);
};

class RostrokarckLarvae : public CPlayableCharacter
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

	RostrokarckLarvae(CSkinningObject* obj, CAnimationManager* aManager);
	virtual void Skill1();

	virtual void Attacked(float damage);

	virtual void UpdateObject(float fElapsedTime);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);
};

class Xenokarce : public CPlayableCharacter
{
public:
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

	Xenokarce(CSkinningObject* obj, CAnimationManager* aManager);
	virtual void Skill1();
	virtual void Skill2();

	virtual void Attacked(float damage);

	virtual void UpdateObject(float fElapsedTime);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);
};

// ==========================================================================

class Fulgurodonte : public CPlayableCharacter
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

	Fulgurodonte(CSkinningObject* obj, CAnimationManager* aManager);
	virtual void Skill1();
	virtual void Skill2();

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

class Limadon : public CPlayableCharacter
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

	Limadon(CSkinningObject* obj, CAnimationManager* aManager);
	virtual void Skill1();
	virtual void Skill2();

	virtual void Attacked(float damage);

	virtual void UpdateObject(float fElapsedTime);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);
};

class Occisodonte : public CPlayableCharacter
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

	Occisodonte(CSkinningObject* obj, CAnimationManager* aManager);
	virtual void Skill1();
	virtual void Skill2();

	virtual void Attacked(float damage);

	virtual void UpdateObject(float fElapsedTime);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);
};

class Crassorrid : public CPlayableCharacter
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
		ANI_FRONT,
		ANI_BACK
	};

	Crassorrid(CSkinningObject* obj, CAnimationManager* aManager);
	virtual void Skill1();
	virtual void Skill2();
	virtual void Skill3();

	virtual void Attacked(float damage);

	virtual void UpdateObject(float fElapsedTime);
	void ProcessInput(UCHAR* keyBuffer, float fElapsedTime);
};

class Gorhorrid : public CPlayableCharacter
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
	Gorhorrid(CSkinningObject* obj, CAnimationManager* aManager);

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

	void HeightCheck(CHeightMapImage* heightmap, float fElapsedTime, float offsetx, float offsety, float offsetz, short mapNum);
	void CollisionCheck(CHeightMapImage* heightmap, float fElapsedTime, float offsetx, float offsety, float offsetz, short mapNum);
protected:
	CPlayableCharacter* m_pMonsterObject{};
	XMFLOAT2 m_xmf2PrevPos{};
};