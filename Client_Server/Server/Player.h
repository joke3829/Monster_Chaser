#pragma once


using namespace DirectX;

class Player {
public:
    int local_id;
    std::string name;
    int room_num;
    XMFLOAT4X4 position;
    int hp = 100;
    bool isReady = false;


    std::mutex playerMutex;

    Player(int id, const std::string& nickname, int room);

    void SetPosition(const XMFLOAT4X4& pos);
    const XMFLOAT4X4& GetPosition() const;

    void TakeDamage(int dmg);
    void Move(float dx, float dy, float dz); // 이동처리 예시
};
