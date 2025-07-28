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
    void MovePosition(float dx, float dz);
    void SendSyncPacket(const Room& room);
    bool isBossMonster();

    void ChangeBossAttack();
    void InitAttackRanges();  // ���� Ÿ�Ժ� ��Ÿ� ����
    MonsterState GetState() const { return state; }

    bool isAlive = true;
    float respawnTimer = 0.0f;
private:

    int id;
    MonsterType type;

    float hp;
    float max_hp;
    int ATK;
    int target_id = -1;
    float m_chaseRange = 50.0f; // �⺻�� (���� ����� �״��)
    MonsterState state;
    XMFLOAT3 position;
    XMFLOAT3 spawnPoint;
    XMFLOAT3 lookDir = { 0.0f, 0.0f, 1.0f }; // �⺻���� ���� (z��)
	XMFLOAT3 defaultLookDir = { 0.0f, 0.0f, 1.0f }; // �⺻ �ٶ󺸴� ����

	int stage = None; // ���Ͱ� ���� �������� (1, 2, 3 ��)
    int gold = 0; // ���Ͱ� �׾��� �� ����ϴ� ���


    
    bool isRespawning = false;

	array<float, 3>AttackRange = { 8.0f, 8.0f, 8.0f }; // �÷��̾ ���� ������ �ִ��� ����
    char Attacktypecount = 1; // ���� ���� ī��Ʈ

    int m_currentAttackType = 1; // ���� ���� Ÿ��

    bool hasRoared = false; // ���Ͱ� ���¢������ ����

    bool bAIPaused = false;
    std::chrono::steady_clock::time_point ai_pause_end_time{}; // �Ͻ� ���� ���� �ð�
    std::chrono::steady_clock::time_point respawnTime;
    std::chrono::steady_clock::time_point lastAttackTime;
    std::mutex mtx;
};
