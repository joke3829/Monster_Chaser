#pragma once
#include "protocol.h"


class Network;
using namespace DirectX;



enum class BuffType : char {
	ATKUP = 0,
	DEFUP = 1,
	DEF_DOWN = 2
};

class Player {
public:

	int gold = 0; // �÷��̾ ȹ���� ���
	int local_id;
	int room_num;
	bool isReady = false;


	std::mutex playerMutex;

	Player(int id, const std::string& nickname, int room);

	void SetPosition(const XMFLOAT4X4& pos);
	const XMFLOAT4X4& GetPosition() const;

	bool  TakeDamage(int dmg);
	//void GetDamage(int dmg) {
	//	std::lock_guard<std::mutex> lock(playerMutex);
	//	int damage = dmg - GetDEF(); // ���� ����
	//	hp -= damage;
	//	if (hp < 0) hp = 0; // HP�� 0 ���Ϸ� �������� �ʵ��� ó��
	//}

	void Move(float dx, float dy, float dz); // �̵�ó�� ����

	float GetHP() { return hp; }
	void SetHP(float newHP) { hp = newHP; } // Add a setter method for HP

	void PlusHP(int new_hp) {
		std::lock_guard<std::mutex> lock(playerMutex);
		hp += new_hp;
		if (hp > max_hp) hp = max_hp; // HP�� �ִ�ġ�� ���� �ʵ��� ó��
	}


	float GetMP() { return skill_cost; } // ��ų ��� ��� ��ȯ
	void SetMP(float newMP) { skill_cost = newMP; } // Add a setter method for MP
	void PlusMP(float new_mp) {
		std::lock_guard<std::mutex> lock(playerMutex);
		skill_cost += new_mp;
	}
	void setSkillCost(float new_cost) {
		std::lock_guard<std::mutex> lock(playerMutex);
		skill_cost = new_cost;
		if (skill_cost > max_skill_cost) skill_cost = max_skill_cost; // ��ų ��� ����� �ִ�ġ�� ���� �ʵ���
	}
	// MP �ڵ� ȸ�� ���� �Լ�
	void RecoverSkillCost(int amount);
	// MP �ڵ� ȸ�� ���� �Լ�


	void Updatestatus(Character t);

	void AddATKBuff_Potion(float value, float duration_sec);
	void AddATKBuff_Skill(float value, float duration_sec);



	void AddATKBuff(float value, float duration_sec);

	void AddDEFBuff(float value, float duration_sec);


	bool IsATKBuffActive() const {
		return atk_buff_potion > 0 || atk_buff_skill > 0;
	}

	bool IsDEFBuffActive() const {
		return def_buff > 0;
	}

	bool IsDEFDeBuffActive() const {
		return def_debuff > 0;
	}


	float GetDamage(int type);

	bool TakeDamage(int dmg, int type) {
		std::lock_guard<std::mutex> lock(playerMutex);
		int damage = dmg - GetDEF(); // ���� ����
		if (damage < 0) damage = 0; // �������� ���� ���ذ� 0 ���ϰ� ���� �ʵ��� ó��
		hp -= damage;

		if (hp < 0)
			hp = 0; // HP�� 0 ���Ϸ� �������� �ʵ��� ó��

		return hp == 0; // HP�� 0�� �Ǹ� true ��ȯ
	}

	float GetATK();
	float GetDEF();
	



	void SetLastHitTime() {
		lastHitTime = std::chrono::steady_clock::now();
	}

	std::chrono::steady_clock::time_point GetLastHitTime() const {
		return lastHitTime;
	}



	void UpdateBuffStatesIfChanged();
	void SendBuffPacketIfChanged(BuffType type, bool currentState);



	
private:
	Player() = default; // �⺻ �����ڴ� private�� �����Ͽ� ������� ���ϰ� ��



	std::string name;

	XMFLOAT4X4 position;
	float hp = 100;
	float max_hp = 100; // �ִ� HP
	float skill_cost = 100; // ��ų ��� ���
	float max_skill_cost = 100; // ��ų ��� ���
	Character type = Character::None; // ĳ���� Ÿ�� (��: ����, ������ ��)

	int attack = 10; // ���ݷ�
	float atk_buff = 0.f;   // �߰� ���ݷ�

	int defense = 5; // ����

	// ���ݷ� ����
	float atk_buff_potion = 0.f;
	float atk_buff_skill = 0.f;
	std::chrono::steady_clock::time_point atk_buff_potion_end{};
	std::chrono::steady_clock::time_point atk_buff_skill_end{};

	// �߰� ����
	float def_buff = 0.f;  
	std::chrono::steady_clock::time_point def_buff_end{};

	// ���� ����� (����)
	float def_debuff = 0.f;
	std::chrono::steady_clock::time_point def_debuff_end{};


	std::unordered_map<BuffType, bool> lastSentBuffState; // Ŭ�� ���������� ���۵� ���� ����


	std::chrono::steady_clock::time_point lastHitTime = std::chrono::steady_clock::now();
};
