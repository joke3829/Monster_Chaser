#include "ObjectManager.h"
extern std::mutex mtx;
extern std::unordered_map<int, Player*> Players;

ObjectManager::ObjectManager(int id)
    : my_id(id), m_pos({ 0.0f, 0.0f, 0.0f, 1.0f }) {}

int ObjectManager::getID() const {
    return my_id;
}

void ObjectManager::setPosition(const XMFLOAT4& pos) {
    m_pos = pos;
}

XMFLOAT4 ObjectManager::getPosition() const {
    return m_pos;
}



void Player::setPlayerID_In_Game(const int& val, const int& key)
{
    PlayerID_In_Game[key] = val;
}
