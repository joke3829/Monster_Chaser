#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "mutex"

class ObjectManager {
public:
    ObjectManager(int id);
    virtual ~ObjectManager() = default;

    int getID() const;

    void setPosition(const XMFLOAT4& pos);
    XMFLOAT4 getPosition() const;

protected:
    int my_id;
    XMFLOAT4 m_pos; // �� XMFLOAT4�� �����
};

class Player : public ObjectManager {
public:
    Player(int id) : ObjectManager(id) {}

    bool isReady() const { return readyToStart; }
    void setReady(const bool& ready) { readyToStart = ready; }
    void setRoomNumber(const int& num) { room_number = num; }

    int GetRoomNumber() { return room_number; }
    
    int room_players[MAX_ROOM] = {0};
private:
    int room_number{ -1 };
    bool readyToStart = false;
    int hp = 100;
    
    // �� �ʿ��� ���°���...
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
