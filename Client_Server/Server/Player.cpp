#include "stdafx.h"
#include "Player.h"
Player::Player(int id, const std::string& nickname, int room)
	: local_id(id), name(nickname), room_num(room)
{
	position = XMFLOAT4X4(); // 초기 위치 0
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
	return hp == 0; // 0이 되면 죽음
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
		hp = 800;
		skill_cost = 100; // 스킬 사용 비용
		attack = 800; // 공격력
		defense = 10; // 방어력
		break;
	}
	case Warrior:
	{
		hp = 1200;
		skill_cost = 100; // 스킬 사용 비용
		attack = 600; // 공격력
		defense = 30; // 방어력
		break;
	}
	case Priest:
	{
		hp = 1000;
		skill_cost = 100; // 스킬 사용 비용
		attack = 800; // 공격력
		defense = 10; // 방어력
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
	std::cout << "[공격력 버프 적용] +" << value << " for " << duration_sec << " seconds\n";

}

void Player::AddDEFBuff(float value, float duration_sec)
{
	def_buff = value;
	def_buff_end_time = std::chrono::steady_clock::now() + std::chrono::seconds((int)duration_sec);
	std::cout << "[방어력 버프 적용] +" << value << " for " << duration_sec << " seconds\n";

}

