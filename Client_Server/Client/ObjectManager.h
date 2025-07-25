#pragma once
#include "stdafx.h"
#include <mutex>
#include <DirectXMath.h>
#include "GameObject.h"
#include "AnimationManager.h"

// 07.25 ===========================================
extern std::array<bool, 3>	g_PlayerBuffState{};
extern std::array<float, 3> g_maxHPs;
extern std::array<float, 3> g_curHPs;
extern std::array<float, 3> g_maxMPs;
extern std::array<float, 3> g_curMPs;
extern std::array<float, 3> g_SkillCoolTime{};
extern std::array<float, 3> g_SkillCurCTime{};
extern std::array<float, 3> g_SkillCost{};
// =================================================

class ObjectManager {
public:
    ObjectManager() {}
    ObjectManager(int id);
    virtual ~ObjectManager() = default;

    int getID() const;

    void setMatrix(const XMFLOAT4X4& pos);
    XMFLOAT3 getPosition() const;

    void setRenderingObject(CSkinningObject* obj) { SkinningObject = obj; }
    void setAnimationManager(CAnimationManager* ani) { AnimationManager = ani; }

    CSkinningObject* getRenderingObject() { return SkinningObject; }
    CAnimationManager* getAnimationManager() { return AnimationManager; }

    void setPosition(const XMFLOAT4X4& pos) {
        setMatrix(pos);
        if (SkinningObject) {
            XMFLOAT3 pos3 = { pos._41, pos._42 ,pos._43 };
            SkinningObject->SetPosition(pos3);
        }
    }

protected:
    int my_id;
    XMFLOAT4X4 m_Matrix;
    CSkinningObject* SkinningObject = nullptr;
    CAnimationManager* AnimationManager = nullptr;
};

class Player : public ObjectManager {
public:
    Player() {}
    Player(int id) : ObjectManager(id) {}

    bool getReady() const { return readyToStart; }
    void setReady(const bool& ready) { readyToStart = ready; }

    void setCharacterType(const short t) { type = t; }
    short getCharacterType() { return type; }

    bool TakeDamage(int dmg) {
        hp -= dmg;
        if (hp < 0) hp = 0;
        return hp == 0;
    }

    int GetHP() const {
        return hp;
    }



    void Plusgold(int amount) {
        gold += amount;
	}
    void SetHP(int newHP) {
        hp = newHP;
        if (hp > maxHP) hp = maxHP; // HP�� �ִ�ġ�� ���� �ʵ���
	}
    void SetMaxHP(int newMaxHP) {
        maxHP = newMaxHP;
        if (hp > maxHP) hp = maxHP; // ���� HP�� �ִ�ġ���� ũ�� �ִ�ġ�� ����
    }

    void SetMP(int newMP) {
        mp = newMP;
        if (mp > maxMP) mp = maxMP; // MP�� �ִ�ġ�� ���� �ʵ���
	}
    void SetMaxMP(int newMaxMP) {
        maxMP = newMaxMP;
        if (mp > maxMP) mp = maxMP; // ���� MP�� �ִ�ġ���� ũ�� �ִ�ġ�� ����
    }

private:
    bool readyToStart = false;
    int hp = 0;
	int maxHP = 100; // �ִ� HP

    int mp = 100; // �÷��̾ ���� MP
    int maxMP = 100; // �ִ� MP

	int gold = 0; // �÷��̾ ���� ���    

    short type{ JOB_NOTHING };
};

class Monster : public ObjectManager {
public:
    Monster(int id, MonsterType t);

    void setTargetID(int tid) { m_targetID = tid; }
    int getTargetID() const { return m_targetID; }

	void setMonsterType(MonsterType t) { type = t; }
	MonsterType getMonsterType  () const { return type; }

	void setSpawnPoint(const XMFLOAT3& point) { spawnPoint = point; }
	void setHP(int newHP) { hp = newHP; }    
private:
    int m_targetID = -1;
    int hp = 300;
    MonsterType type;
	XMFLOAT3 spawnPoint; // ���� ���� ��ġ
};
