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
		hp = 100;
		break;
	case MonsterType::Pistiripere:
		hp = 200;
		break;
	case MonsterType::RostrokarackLarvae:
		hp = 300;
		break;
	case MonsterType::XenokarceBoss:
		break;
	case MonsterType::Occisodonte:
		break;
	case MonsterType::Limadon:
		break;
	case MonsterType::Fulgurodonte:
		break;
	case MonsterType::CrassorridBoss:
		break;
	case MonsterType::GorhorridBoss:
		break;

	default:
		hp = 100; // �⺻��
		break;
	}
}
