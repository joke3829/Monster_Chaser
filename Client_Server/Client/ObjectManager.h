#pragma once
#include "stdafx.h"
#include <mutex>
#include <DirectXMath.h>
#include "GameObject.h"
#include "AnimationManager.h"

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

    float GetHP() const {
        return hp;
    }

    void Plusgold(int amount) {
        gold += amount;
	}
    void SetHP(float newHP) {
        hp = newHP;
        if (hp > maxHP) hp = maxHP; // HP가 최대치를 넘지 않도록
	}
    void SetMaxHP(float newMaxHP) {
        maxHP = newMaxHP;
        if (hp > maxHP) hp = maxHP; // 현재 HP가 최대치보다 크면 최대치로 설정
    }

    void SetMP(float newMP) {
        mp = newMP;
        if (mp > maxMP) mp = maxMP; // MP가 최대치를 넘지 않도록
	}
    void SetMaxMP(float newMaxMP) {
        maxMP = newMaxMP;
        if (mp > maxMP) mp = maxMP; // 현재 MP가 최대치보다 크면 최대치로 설정
    }

private:
    bool readyToStart = false;
    float hp = 0;
    float maxHP = 100; // 최대 HP

    float mp = 100; // 플레이어가 가진 MP
    float maxMP = 100; // 최대 MP

    float gold = 0; // 플레이어가 가진 골드    

    short type{ JOB_NOTHING };
};

class Monster : public ObjectManager {
public:


    enum class ANIMATION{
        ANI_DEATH,
        ANI_HIT,
        ANI_IDLE,
        ANI_ROAR,
        ANI_FRONT,
        ANI_BACK,
        ANI_SKILL1,
        ANI_SKILL2,
        ANI_SKILL3,
        ANI_RUN
    };
    Monster(int id, MonsterType t);

    void setTargetID(int tid) { m_targetID = tid; }
    int getTargetID() const { return m_targetID; }

	void setMonsterType(MonsterType t) { type = t; }
	MonsterType getMonsterType  () const { return type; }

	void setSpawnPoint(const XMFLOAT3& point) { spawnPoint = point; }
	void setHP(float newHP) 
    {
        hp = newHP; 
    } 

    void setCurrentAttackType(int attackType);
        
    UINT getCurrentAttackType() { return static_cast<UINT>(currentAttackType); }
  
    
private:
    int m_targetID = -1;
    float hp = -1;
	float max_hp = -1; // 몬스터 최대 HP
    MonsterType type;
	XMFLOAT3 spawnPoint; // 몬스터 스폰 위치

    ANIMATION currentAttackType;
};
