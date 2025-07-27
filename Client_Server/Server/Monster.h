#pragma once
#include <cmath>
#include <mutex>
#include <chrono>
#include <DirectXMath.h>
#include "PlayerManager.h"
#include "Room.h"
#include "HeightMap.h"

// =====================================================
extern std::unique_ptr<CHeightMapImage> g_pStage1Height;
extern std::unique_ptr<CHeightMapImage> g_pStage1Collision;
extern std::unique_ptr<CHeightMapImage> g_pStage2Height;
extern std::unique_ptr<CHeightMapImage> g_pStage2Collision;
extern std::unique_ptr<CHeightMapImage> g_pStage3Height;
extern std::unique_ptr<CHeightMapImage> g_pStage3Collision;
// =====================================================

float getHeight(CHeightMapImage* heightmap, float x, float z, float offsetx, float offsety, float offsetz, short mapNum);
bool CollisionCheck(CHeightMapImage* heightmap, CHeightMapImage* collisionmap, float x, float z, float offsetx, float offsety, float offsetz, short mapNum);

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
    Monster(int id, const XMFLOAT3& spawnPos, MonsterType t);
    MonsterType GetType() const { return type; }

    void Update(float deltaTime, const Room& room, const PlayerManager& playerManager);
    bool TakeDamage(float dmg);
    int GetId() const { return id; }
    int GetHP() const { return hp; }

    XMFLOAT3 GetPosition() const { return position; }
    int GetGold() const { return gold; }


    int GetATK() const { return ATK; }

    void GetDamage();

    void TransitionTo(MonsterState nextState);
    void HandleIdle(const Room& room, const PlayerManager& playerManager);
    void HandleChase(const PlayerManager& playerManager, const Room& room);
    void HandleAttack(const PlayerManager& playerManager, const Room& room);
    void HandleReturn(const PlayerManager& playerManager, const Room& room);
    void HandleDead(const Room& room);
    float DistanceToPlayer(const PlayerManager& playerManager) const;
    float DistanceFromSpawnToPlayer(const PlayerManager& playerManager) const;
    bool IsPlayerNear(const PlayerManager& playerManager) const;
    bool IsPlayerInAttackRange(const PlayerManager& playerManager) const;
    int GetAttackTypeCount() const { return Attacktypecount; }

    void SendSyncPacket(const Room& room);
    bool isBossMonster();

    void ChangeBossAttack();


    bool isAlive = true;
    float respawnTimer = 0.0f;
private:

    int id;
    MonsterType type;

    float hp;
    float max_hp;
    int ATK;
    int target_id = -1;

    MonsterState state;
    XMFLOAT3 position;
    XMFLOAT3 spawnPoint;
    XMFLOAT3 lookDir = { 0.0f, 0.0f, 1.0f }; // 기본값은 정면 (z축)

    int gold = 0; // 몬스터가 죽었을 때 드랍하는 골드

    bool isRespawning = false;
    char Attacktypecount = 1; // 공격 종류 카운트

    int m_currentAttackType = 1; // 현재 공격 타입
    bool hasRoared = false; // 몬스터가 울부짖었는지 여부

    std::chrono::steady_clock::time_point respawnTime;
    std::chrono::steady_clock::time_point lastAttackTime;
    std::mutex mtx;
};
