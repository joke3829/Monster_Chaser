#include "stdafx.h"
#include "Player.h"
#include "Network.h"

extern Network g_server;

Player::Player(int id, const std::string& nickname, int room)
	: local_id(id), name(nickname), room_num(room)
{
	lastSentBuffState[BuffType::ATKUP] = false;
	lastSentBuffState[BuffType::DEFUP] = false;
	lastSentBuffState[BuffType::DEF_DOWN] = false;
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


void Player::RecoverSkillCost(int amount)
{
	skill_cost += amount;
	if (skill_cost > max_skill_cost) {
		skill_cost = max_skill_cost;
	}
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
		max_hp = hp; // 최대 HP 설정
		skill_cost = 100; // 스킬 사용 비용
		max_skill_cost = skill_cost; // 스킬 사용 비용
		attack = 800; // 공격력
		defense = 10; // 방어력
		break;
	}
	case Warrior:
	{
		hp = 1200;
		max_hp = hp; // 최대 HP 설정
		skill_cost = 100; // 스킬 사용 비용
		max_skill_cost = skill_cost; // 스킬 사용 비용
		attack = 600; // 공격력
		defense = 30; // 방어력
		break;
	}
	case Priest:
	{
		hp = 1000;
		max_hp = hp; // 최대 HP 설정
		skill_cost = 100; // 스킬 사용 비용
		max_skill_cost = skill_cost; // 스킬 사용 비용
		attack = 800; // 공격력
		defense = 10; // 방어력
		break;
	}
	default:
		break;
	}
}

void Player::AddATKBuff_Potion(float value, float duration_sec)
{
	atk_buff_potion = value;
	atk_buff_potion_end = std::chrono::steady_clock::now() + std::chrono::seconds((int)duration_sec);
}

void Player::AddATKBuff_Skill(float value, float duration_sec)
{
	atk_buff_skill = value;
	atk_buff_skill_end = std::chrono::steady_clock::now() + std::chrono::seconds((int)duration_sec);
}





void Player::AddATKBuff(float value, float duration_sec)
{
	atk_buff = value;
	atk_buff_skill_end = std::chrono::steady_clock::now() + std::chrono::seconds((int)duration_sec);
	std::cout << "[공격력 버프 적용] +" << value << " for " << duration_sec << " seconds\n";

}




void Player::AddDEFBuff(float value, float duration_sec)
{
	def_buff = value;
	def_buff_end = std::chrono::steady_clock::now() + std::chrono::seconds((int)duration_sec);
	std::cout << "[방어력 버프 적용] +" << value << " for " << duration_sec << " seconds\n";

}

float Player::GetDamage(int type)
{

	float attack = GetATK();	

	switch (type)
	{
	case 0: // 일반 공격
		return attack;
	case 1:
		return attack * 3.0f;
	case 2:
		return attack * 3.3f;
	case 3:
		return attack * 4.0f;
	default:
		break;
	}
	return 0.0f;
}

float Player::GetATK() {
	auto now = std::chrono::steady_clock::now();
	if (now >= atk_buff_potion_end)
		atk_buff_potion = 0.f;
	if (now >= atk_buff_skill_end) 
		atk_buff_skill = 0.f;
	return attack + atk_buff_potion + atk_buff_skill;
}


float Player::GetDEF() {
	auto now = std::chrono::steady_clock::now();
	if (now >= def_buff_end) def_buff = 0.f;
	if (now >= def_debuff_end) def_debuff = 0.f;

	return defense + def_buff - def_debuff;
}

void Player::UpdateBuffStatesIfChanged()
{
	auto now = std::chrono::steady_clock::now();

	// 현재 버프 상태 계산
	bool atkActive = (atk_buff_potion > 0.f && now < atk_buff_potion_end) ||
						(atk_buff_skill > 0.f && now < atk_buff_skill_end);

	bool defActive = (def_buff > 0.f && now < def_buff_end);

	bool defDownActive = (def_debuff > 0.f && now < def_debuff_end);

	// 각 상태 비교 및 전송
	SendBuffPacketIfChanged(BuffType::ATKUP, atkActive);
	SendBuffPacketIfChanged(BuffType::DEFUP, defActive);
	SendBuffPacketIfChanged(BuffType::DEF_DOWN, defDownActive);
}

void Player::SendBuffPacketIfChanged(BuffType type, bool currentState)
{
	// 이전에 전송한 상태와 다르면 새로 전송
	if (lastSentBuffState[type] != currentState) {
		lastSentBuffState[type] = currentState;

		sc_packet_buff_change pkt;
		pkt.size = sizeof(pkt);
		pkt.type = S2C_P_BUFFCHANGE;
		pkt.bufftype = static_cast<char>(type);
		pkt.state = currentState ? 1 : 0;

		for (int pid : g_server.rooms[room_num].id)
			g_server.users[pid]->do_send(&pkt);

		std::cout << "[버프 상태 변경] 플레이어 " << local_id << " | 타입: " << (int)pkt.bufftype
			<< " | 상태: " << (int)pkt.state << "\n";
	}
}

