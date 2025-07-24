#pragma once


using namespace DirectX;

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
	void GetDamage(int dmg) {
		std::lock_guard<std::mutex> lock(playerMutex);
		int damage = dmg - GetDEF(); // ���� ����
		hp -= damage;
		if (hp < 0) hp = 0; // HP�� 0 ���Ϸ� �������� �ʵ��� ó��
	}

	void Move(float dx, float dy, float dz); // �̵�ó�� ����

	int GetHP() { return hp; }
	void SetHP(int newHP) { hp = newHP; } // Add a setter method for HP

	void PlusHP(int new_hp) {
		std::lock_guard<std::mutex> lock(playerMutex);
		hp += new_hp;
	}


	int GetMP() { return skill_cost; } // ��ų ��� ��� ��ȯ
	void SetMP(int newMP) { skill_cost = newMP; } // Add a setter method for MP
	void PlusMP(int new_mp) {
		std::lock_guard<std::mutex> lock(playerMutex);
		skill_cost += new_mp;
	}

	

	void Updatestatus(Character t);


	void AddATKBuff(float value, float duration_sec);

	void AddDEFBuff(float value, float duration_sec);

	float GetATK() {
		if (std::chrono::steady_clock::now() >= atk_buff_end_time)
			atk_buff = 0; // ���� ���� ó��
		return attack + atk_buff;
	}

	float GetDEF()
	{
		if (std::chrono::steady_clock::now() >= def_buff_end_time)
			def_buff = 0; // ���� ���� ó��
		return defense + def_buff;
	}

private:
	Player() = default; // �⺻ �����ڴ� private�� �����Ͽ� ������� ���ϰ� ��



	std::string name;

	XMFLOAT4X4 position;
	int hp = 100;
	int skill_cost = 100; // ��ų ��� ���
	Character type = Character::None; // ĳ���� Ÿ�� (��: ����, ������ ��)

	int attack = 10; // ���ݷ�
	float atk_buff = 0.f;   // �߰� ���ݷ�

	int defense = 5; // ����
	float def_buff = 0.f;   // �߰� ����

	std::chrono::steady_clock::time_point atk_buff_end_time{};
	std::chrono::steady_clock::time_point def_buff_end_time;
};
