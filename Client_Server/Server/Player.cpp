#include "stdafx.h"
#include "Player.h"
Player::Player(int id, const std::string& nickname, int room)
	: local_id(id), name(nickname), room_num(room)
{
	position = XMFLOAT4X4(); // �ʱ� ��ġ 0
}

void Player::SetPosition(const XMFLOAT4X4& pos) {
	position = pos;
}

const XMFLOAT4X4& Player::GetPosition() const {
	return position;
}

bool  Player::TakeDamage(int dmg) {
	std::lock_guard<std::mutex> lock(playerMutex);
	hp -= dmg;
	if (hp < 0) hp = 0;
	return hp == 0; // 0�� �Ǹ� ����
}

void Player::Move(float dx, float dy, float dz) {
	position._41 += dx;
	position._42 += dy;
	position._43 += dz;
}

void Player::Updatestatus(Character t)
{
	type = t;
	switch (type)
	{
	case None:
		break;
	case Wizard:
	{
		int hp = 800;
		int skill_cost = 100; // ��ų ��� ���
		int attack = 800; // ���ݷ�
		int defense = 10; // ����
		break;
	}
	case Warrior:
	{
		int hp = 1200;
		int skill_cost = 100; // ��ų ��� ���
		int attack = 600; // ���ݷ�
		int defense = 30; // ����
		break;
	}
	case Priest:
	{
		int hp = 1000;
		int skill_cost = 100; // ��ų ��� ���
		int attack = 800; // ���ݷ�
		int defense = 10; // ����
		break;
	}
	default:
		break;
	}
}

void Player::AddATKBuff(float value, float duration_sec)
{
	atk_buff = value;
	atk_buff_end_time = std::chrono::steady_clock::now() + std::chrono::seconds((int)duration_sec);
	std::cout << "[���ݷ� ���� ����] +" << value << " for " << duration_sec << " seconds\n";
}

void Player::AddDEFBuff(float value, float duration_sec)
{
	def_buff = value;
	def_buff_end_time = std::chrono::steady_clock::now() + std::chrono::seconds((int)duration_sec);
	std::cout << "[���� ���� ����] +" << value << " for " << duration_sec << " seconds\n";
}

