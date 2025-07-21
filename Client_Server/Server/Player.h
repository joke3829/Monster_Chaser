#pragma once


using namespace DirectX;

class Player {
public:
    int local_id;
    std::string name;
    int room_num;
    XMFLOAT4X4 position;
    int hp = 100;
	int gold = 0; // 플레이어가 획득한 골드
    bool isReady = false;


    std::mutex playerMutex;

    Player(int id, const std::string& nickname, int room);

    void SetPosition(const XMFLOAT4X4& pos);
    const XMFLOAT4X4& GetPosition() const;

    bool  TakeDamage(int dmg);
    void Move(float dx, float dy, float dz); // 이동처리 예시

    int GetHP() { return hp; }
    int setHP(int new_hp) { 
        std::lock_guard<std::mutex> lock(playerMutex);
        hp = new_hp; 
        return hp; 
	}

};
