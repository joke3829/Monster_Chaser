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

void Player::TakeDamage(int dmg) {
    std::lock_guard<std::mutex> lock(playerMutex);
    hp -= dmg;
    if (hp < 0) hp = 0;
}

void Player::Move(float dx, float dy, float dz) {
    position._41 += dx;
    position._42 += dy;
    position._43 += dz;
}
