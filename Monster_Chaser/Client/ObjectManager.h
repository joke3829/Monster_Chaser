#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "mutex"
#include "GameObject.h"

class ObjectManager {
public:
    ObjectManager(int id);
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
    Player(int id) : ObjectManager(id) {}

    void setRenderingObject(CSkinningObject* obj) { Client_Object = obj; }

    CSkinningObject* getRenderingObject() { return Client_Object; }

    void setPosition(const XMFLOAT4X4& pos) {
        ObjectManager::setMatrix(pos);
        if (Client_Object) {
            XMFLOAT3 pos3 = { pos._41, pos._42 ,pos._43 };
            Client_Object->SetPosition(pos3);
        }
    }

    bool isReady() const { return readyToStart; }
    void setReady(const bool& ready) { readyToStart = ready; }
    void setRoomNumber(const int& num) { room_number = num; }
    void setPlayerID_In_Game(const int & val,const int&key);
    int GetRoomNumber() { return room_number; }
    
    int room_players[MAX_ROOM] = {0};
private:
    int room_number{ -1 };
    bool readyToStart = false;
    int hp = 100;
    int PlayerID_In_Game[3];
    CSkinningObject* Client_Object=nullptr;
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
