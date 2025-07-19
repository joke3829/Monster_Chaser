#pragma once
#include <cmath>
#include <mutex>
#include <chrono>
#include <DirectXMath.h>
#include "PlayerManager.h"
#include "Room.h"

enum class MonsterState {
    Idle,
    Chase,
    Attack,
    Return,
    Dead
};

class Room;

class Monster {
public:
    Monster(int id, const XMFLOAT3& spawnPos);

    void Update(float deltaTime, const Room& room, const PlayerManager& playerManager);
    bool TakeDamage(int dmg);
    int GetId() const { return id; }
    int GetHP() const { return hp; }

    XMFLOAT3 GetPosition() const { return position; }
    int GetGold() const { return gold; }

private:
    void TransitionTo(MonsterState nextState);
    void HandleIdle(const Room& room, const PlayerManager& playerManager);
    void HandleChase(const PlayerManager& playerManager, const Room& room);
    void HandleAttack(const PlayerManager& playerManager);
    void HandleReturn();
    void HandleDead(const Room& room);
    float DistanceToPlayer(const PlayerManager& playerManager) const;

    bool IsPlayerNear(const PlayerManager& playerManager) const;
    bool IsPlayerInAttackRange(const PlayerManager& playerManager) const;

    void SendSyncPacket(const Room& room);

    int id;
    int hp;
    int target_id = -1;
    MonsterState state;
    XMFLOAT3 position;
    XMFLOAT3 spawnPoint;

    int gold = 0; // 몬스터가 죽었을 때 드랍하는 골드

    bool isRespawning = false;
    std::chrono::steady_clock::time_point respawnTime;
    std::chrono::steady_clock::time_point lastAttackTime;

    std::mutex mtx;
};
