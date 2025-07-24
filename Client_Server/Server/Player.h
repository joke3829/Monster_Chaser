#pragma once


using namespace DirectX;

class Player {
public:

	int gold = 0; // 플레이어가 획득한 골드
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
		int damage = dmg - GetDEF(); // 방어력 적용
		hp -= damage;
		if (hp < 0) hp = 0; // HP가 0 이하로 떨어지지 않도록 처리
	}

	void Move(float dx, float dy, float dz); // 이동처리 예시

	int GetHP() { return hp; }
	void SetHP(int newHP) { hp = newHP; } // Add a setter method for HP

	void PlusHP(int new_hp) {
		std::lock_guard<std::mutex> lock(playerMutex);
		hp += new_hp;
	}


	int GetMP() { return skill_cost; } // 스킬 사용 비용 반환
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
			atk_buff = 0; // 버프 만료 처리
		return attack + atk_buff;
	}

	float GetDEF()
	{
		if (std::chrono::steady_clock::now() >= def_buff_end_time)
			def_buff = 0; // 버프 만료 처리
		return defense + def_buff;
	}

private:
	Player() = default; // 기본 생성자는 private로 설정하여 사용하지 못하게 함



	std::string name;

	XMFLOAT4X4 position;
	int hp = 100;
	int skill_cost = 100; // 스킬 사용 비용
	Character type = Character::None; // 캐릭터 타입 (예: 전사, 마법사 등)

	int attack = 10; // 공격력
	float atk_buff = 0.f;   // 추가 공격력

	int defense = 5; // 방어력
	float def_buff = 0.f;   // 추가 방어력

	std::chrono::steady_clock::time_point atk_buff_end_time{};
	std::chrono::steady_clock::time_point def_buff_end_time;
};
