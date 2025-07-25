#include "ObjectManager.h"
extern std::mutex mtx;
extern std::unordered_map<int, Player> Players;

ObjectManager::ObjectManager(int id)
	: my_id(id) {}

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

Monster::Monster(int id, MonsterType t) : ObjectManager(id), type(t) 
{

	switch (type)
	{
		
	case MonsterType::Feroptere:
	case MonsterType::Pistiripere:
	case MonsterType::RostrokarackLarvae:
		hp = 15000;
		break;
	case MonsterType::XenokarceBoss:
		hp = 40000;
		break;
	case MonsterType::Occisodonte:
	case MonsterType::Limadon:
	case MonsterType::Fulgurodonte:
		hp = 30000;
		break;
	case MonsterType::CrassorridBoss:
		hp = 80000;
		break;
	case MonsterType::GorhorridBoss:
		hp = 200000;
		break;

	default:
		hp = 100; // �⺻��
		break;
	}
}
