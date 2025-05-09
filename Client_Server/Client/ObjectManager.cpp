#include "ObjectManager.h"
extern std::mutex mtx;
extern std::unordered_map<int, Player*> Players;

ObjectManager::ObjectManager(int id)
    : my_id(id), m_Matrix() {}

int ObjectManager::getID() const {
    return my_id;
}

void ObjectManager::setMatrix(const XMFLOAT4X4& pos) {
    m_Matrix = pos;
}

XMFLOAT3 ObjectManager::getPosition() const {
    XMFLOAT3 pos = XMFLOAT3(m_Matrix._41, m_Matrix._42, m_Matrix._43);
    return pos;
}




