#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "mutex"
#include "GameObject.h"
#include "AnimationManager.h"

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
    XMFLOAT4X4 m_Matrix; // �� XMFLOAT4�� �����
};

class Player : public ObjectManager {
public:
    Player() {}
    Player(int id) : ObjectManager(id) {}

    void setRenderingObject(CSkinningObject* obj){Client_Object = obj;}

    void setAnimationManager(CAnimationManager* ani){Client_AniManager = ani;}

    CSkinningObject* getRenderingObject() { return Client_Object; }
    CAnimationManager* getAnimationManager() { return Client_AniManager; }

    void setPosition(const XMFLOAT4X4& pos) {
        ObjectManager::setMatrix(pos);
        if (Client_Object) {
            XMFLOAT3 pos3 = { pos._41, pos._42 ,pos._43 };
            Client_Object->SetPosition(pos3);
        }
    }

    bool getReady() const { return readyToStart; }
    void setReady(const bool& ready) { readyToStart = ready; }
   
    void setCharacterType(const short t) { type = t; }
    short getCharacterType() { return type; }
   
   
private:
   
    bool readyToStart = false;
    int hp = 100;
    
    CSkinningObject* Client_Object = nullptr;
    CAnimationManager* Client_AniManager = nullptr;
    // �� �ʿ��� ���°���...
    short type{}; //ĳ���� ���� Ÿ��
};

class Monster : public ObjectManager {
public:
    Monster(int id) : ObjectManager(id) {}

    void setTargetID(int tid) { m_targetID = tid; }
    int getTargetID() const { return m_targetID; }

private:
    int m_targetID = -1;            //������ Ŭ���� id
    int hp = 300;
    // ���� FSM, ���°���...
};
