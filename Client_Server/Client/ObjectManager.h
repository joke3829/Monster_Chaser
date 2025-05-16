#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "mutex"
#include "GameObject.h"
#include "AnimationManager.h"
#include "PlayableCharacter.h"

class ObjectManager {
public:
    ObjectManager(){}
    ObjectManager(int id) ;
    virtual ~ObjectManager() = default;

    int getID() const;

    void setMatrix(const XMFLOAT4X4& pos);
    XMFLOAT3 getPosition() const;

protected:
    int my_id;
    XMFLOAT4X4 m_Matrix; // ← XMFLOAT4로 변경됨
};

class Player : public ObjectManager {
public:
    Player() {}
    Player(int id) : ObjectManager(id) {}

    void setPlayerableCharacter(CPlayableCharacter* character)
    {
        Client_Character = character;
    }

    CSkinningObject* getRenderingObject() { return Client_Character->getObject(); }
    CAnimationManager* getAnimationManager() { return Client_Character->getAniManager(); }

    void setPosition(const XMFLOAT4X4& pos) {
        ObjectManager::setMatrix(pos);
        if (Client_Character->getObject()) {
            XMFLOAT3 pos3 = { pos._41, pos._42 ,pos._43 };
            Client_Character->getObject()->SetPosition(pos3);
        }
    }

    bool getReady() const { return readyToStart; }
    void setReady(const bool& ready) { readyToStart = ready; }
   
   
   
   
private:
   
    bool readyToStart = false;
    int hp = 100;
    
    CPlayableCharacter* Client_Character = nullptr;
    // 더 필요한 상태값들...
};

class Monster : public ObjectManager {
public:
    Monster(int id) : ObjectManager(id) {}

    void setTargetID(int tid) { m_targetID = tid; }
    int getTargetID() const { return m_targetID; }

private:
    int m_targetID = -1;            //공격할 클라의 id
    int hp = 300;
    // 몬스터 FSM, 상태값들...
};
