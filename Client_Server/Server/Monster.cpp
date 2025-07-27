#include "stdafx.h"
#include "Monster.h"
#include "Network.h"
#include <random>

#define MONSTER_CHASE_DISTANCE 70.0f
#define MONSTER_ATTACK_RANGE 1.0f
constexpr float MONSTER_ATTACK_COOLDOWN = 2.0f;   // 공격 쿨타임 2초
constexpr float MONSTER_RETURN_SPEED = 80.0f;     // 귀환 속도
std::random_device rd;
std::mt19937 gen(rd());

extern Network g_server;

float getHeight(CHeightMapImage* heightmap, float x, float z, float offsetx, float offsety, float offsetz, short mapNum)
{
    // mapNum은 SCENE_ 따라감
    if (mapNum == SCENE_WINTERLAND) {
        if (z >= -500.0f) {
            if (heightmap->GetHeightinWorldSpace(x - offsetx, z - offsetz) + offsety < 10.0f)
                return 10.0f;
        }
        return heightmap->GetHeightinWorldSpace(x - offsetx, z - offsetz) + offsety;
    }
    return heightmap->GetHeightinWorldSpace(x - offsetx, z - offsetz) + offsety;
}

// true = 충돌(못가는 곳)
bool CollisionCheck(CHeightMapImage* heightmap, CHeightMapImage* collisionmap, float x, float z, float offsetx, float offsety, float offsetz, short mapNum)
{
    float colHeight = collisionmap->GetHeightinWorldSpace(x - offsetx, z - offsetz);
    float terHeight = heightmap->GetHeightinWorldSpace(x - offsetx, z - offsetz);

    if (colHeight - terHeight >= 0.1f)
        return true;
    return false;
}

int FindClosestPlayerInRoom(const Room& room, const DirectX::XMFLOAT3& monsterPos, const PlayerManager& playerManager) {
    int closestId = -1;
    float closestDistSq = std::numeric_limits<float>::max();

    for (int playerId : room.id) {
        auto player = playerManager.GetPlayer(playerId);
        if (!player) continue;

        const auto& pos = player->GetPosition();
        float dx = monsterPos.x - pos._41;
        float dy = monsterPos.y - pos._42;
        float dz = monsterPos.z - pos._43;

        float distSq = dx * dx + dy * dy + dz * dz;
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            closestId = playerId;
        }
    }

    return closestId;
}

float Monster::DistanceFromSpawnToPlayer(const PlayerManager& playerManager) const {
    auto player = playerManager.GetPlayer(target_id);
    if (!player) return std::numeric_limits<float>::max();

    const auto& p = player->GetPosition();
    float dx = spawnPoint.x - p._41;
    float dz = spawnPoint.z - p._43;
    return sqrtf(dx * dx + dz * dz);
}

Monster::Monster(int id, const XMFLOAT3& spawnPos, MonsterType t)
    : id(id), hp(100), state(MonsterState::Idle), position(spawnPos), spawnPoint(spawnPos), type(t), isRespawning(false) {
    std::uniform_int_distribution<> dis(30, 50);
    gold = dis(gen);

    switch (type) {
    case MonsterType::None:
        Attacktypecount = 0;
        break;
    case MonsterType::Feroptere:
    case MonsterType::Pistiripere:
    case MonsterType::RostrokarackLarvae:
        hp = 15000;
        max_hp = hp;
        ATK = 100;
        Attacktypecount = 1;
        break;
    case MonsterType::Xenokarce:
        hp = 40000;
        max_hp = hp;
        ATK = 200;
        Attacktypecount = 2;
        break;
    case MonsterType::Occisodonte:
    case MonsterType::Limadon:
    case MonsterType::Fulgurodonte:
        hp = 30000;
        max_hp = hp;
        ATK = 100;
        Attacktypecount = 2;
        break;
    case MonsterType::Crassorrid:
        hp = 80000;
        max_hp = hp;
        ATK = 200;
        Attacktypecount = 3;
        break;
    case MonsterType::Gorhorrid:
        hp = 200000;
        max_hp = hp;
        ATK = 300;
        Attacktypecount = 3;
        break;
    default:
        break;
    }
}

void Monster::Update(float deltaTime, const Room& room, const PlayerManager& playerManager) {
    std::lock_guard<std::mutex> lock(mtx);

    if (!room.bStageActive) return;

    if (state == MonsterState::Dead) {
        HandleDead(room);
        return;
    }

    target_id = FindClosestPlayerInRoom(room, position, playerManager);

    switch (state) {
    case MonsterState::Idle:    HandleIdle(room, playerManager); break;
    case MonsterState::Chase:   HandleChase(playerManager, room); break;
    case MonsterState::Attack:  HandleAttack(playerManager, room); break;
    case MonsterState::Return:  HandleReturn(playerManager, room); break;
    default: break;
    }

    if (state == MonsterState::Chase || state == MonsterState::Return)
        SendSyncPacket(room);
}

bool Monster::TakeDamage(float dmg) {
    std::lock_guard<std::mutex> lock(mtx);
    if (!isRespawning) {
        hp -= dmg;
        if (hp <= 0) {
            hp = 0;
            TransitionTo(MonsterState::Dead);
            return true;
        }
    }
    return false;
}

void Monster::GetDamage() {
    ChangeBossAttack();
}

void Monster::TransitionTo(MonsterState nextState) {
    if (state != nextState) state = nextState;
}

void Monster::HandleIdle(const Room& room, const PlayerManager& playerManager) {
    if (target_id != -1 && IsPlayerNear(playerManager)) {
        TransitionTo(MonsterState::Chase);
    }
}

void Monster::HandleChase(const PlayerManager& playerManager, const Room& room) {
    if (!IsPlayerNear(playerManager)) {
        TransitionTo(MonsterState::Return);
        return;
    }
    if (IsPlayerInAttackRange(playerManager)) {
        TransitionTo(MonsterState::Attack);
        return;
    }
    if (!hasRoared && isBossMonster()) {
        hasRoared = true;
        sc_packet_boss_roar pkt;
        pkt.size = sizeof(pkt);
        pkt.type = S2C_P_BOSS_ROAR;
        pkt.monster_id = id;
        for (int pid : room.id) g_server.users[pid]->do_send(&pkt);
    }
    auto player = playerManager.GetPlayer(target_id);
    if (!player) return;
    XMFLOAT3 targetPos = {
        player->GetPosition()._41,
        player->GetPosition()._42,
        player->GetPosition()._43
    };
    float speed = 10.0f;
    float dx = targetPos.x - position.x;
    float dz = targetPos.z - position.z;
    float len = sqrtf(dx * dx + dz * dz); // y축 무시

    if (len > 0.001f) {
        dx /= len;
        dz /= len;

        position.x += dx * speed * 0.016f;
        position.z += dz * speed * 0.016f;

        switch(스테이지) {
        case SCENE_PLAIN:
            if (CollisionCheck(g_pStage1Height.get(), g_pStage1Collision.get(), position.x, position.z,
                -512.0f, 0.0f, -512.0f, SCENE_PLAIN)) {
                position.x -= dx * speed * 0.016f;
                position.z -= dz * speed * 0.016f;
            }
            position.y = getHeight(g_pStage1Height.get(), position.x, position.z,
                -512.0f, 0.0f, -512.0f, SCENE_PLAIN);
            break;
        case SCENE_CAVE:
            if (CollisionCheck(g_pStage2Height.get(), g_pStage2Collision.get(), position.x, position.z,
                -200.0f, -10.0f, -66.5f, SCENE_CAVE)) {
                position.x -= dx * speed * 0.016f;
                position.z -= dz * speed * 0.016f;
            }
            position.y = getHeight(g_pStage2Height.get(), position.x, position.z,
                -200.0f, -10.0f, -66.5f, SCENE_CAVE);
            break;
        case SCENE_WINTERLAND:
            if (CollisionCheck(g_pStage3Height.get(), g_pStage3Collision.get(), position.x, position.z,
                -1024.0f, 0.0f, -1024.0f, SCENE_WINTERLAND)) {
                position.x -= dx * speed * 0.016f;
                position.z -= dz * speed * 0.016f;
            }
            position.y = getHeight(g_pStage3Height.get(), position.x, position.z,
                -1024.0f, 0.0f, -1024.0f, SCENE_WINTERLAND);
            break;
        }

        // y는 그대로 유지
        lookDir = { dx, 0.0f, dz };
    }
}

void Monster::HandleAttack(const PlayerManager& playerManager, const Room& room) {
    if (!IsPlayerNear(playerManager)) {
        TransitionTo(MonsterState::Return);
        return;
    }
    if (!IsPlayerInAttackRange(playerManager)) {
        TransitionTo(MonsterState::Chase);
        return;
    }
    if (Attacktypecount > 1) {
        std::uniform_int_distribution<> dis(1, GetAttackTypeCount());
        m_currentAttackType = dis(gen);
        ChangeBossAttack();
    }
    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - lastAttackTime).count();
    if (elapsed >= MONSTER_ATTACK_COOLDOWN) {
        lastAttackTime = now;
        sc_packet_monster_attack pkt;
        pkt.size = sizeof(pkt);
        pkt.type = S2C_P_MONSTER_ATTACK;
        pkt.monster_id = id;
        pkt.attack_type = m_currentAttackType;
        for (int pid : room.id) g_server.users[pid]->do_send(&pkt);
    }
}

void Monster::HandleReturn(const PlayerManager& playerManager, const Room& room) {
    if (DistanceFromSpawnToPlayer(playerManager) <= MONSTER_CHASE_DISTANCE) {
        TransitionTo(MonsterState::Chase);
        return;
    }
    float dx = spawnPoint.x - position.x;
    float dz = spawnPoint.z - position.z;
    float dist = sqrtf(dx * dx + dz * dz); // y축 무시
  
    if (dist < 10.0f) {
        TransitionTo(MonsterState::Idle);
        return;
    }

    if (dist > 0.001f) {
        dx /= dist;
        dz /= dist;

        position.x += dx * MONSTER_RETURN_SPEED * 0.016f;
        position.z += dz * MONSTER_RETURN_SPEED * 0.016f;

        // y는 그대로 유지
        lookDir = { dx, 0.0f, dz };
    }
}

void Monster::HandleDead(const Room& room) {
    if (!isRespawning) {
        isRespawning = true;
        respawnTime = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        return;
    }
    if (std::chrono::steady_clock::now() >= respawnTime) {
        hp = max_hp;
        isRespawning = false;
        position = spawnPoint;
        TransitionTo(MonsterState::Idle);
        sc_packet_monster_respawn pkt;
        pkt.size = sizeof(pkt);
        pkt.type = S2C_P_MONSTER_RESPAWN;
        pkt.monster_id = id;
        XMStoreFloat4x4(&pkt.pos, XMMatrixTranslation(position.x, position.y, position.z));
        for (int pid : room.id) g_server.users[pid]->do_send(&pkt);
    }
}

float Monster::DistanceToPlayer(const PlayerManager& playerManager) const {
    auto player = playerManager.GetPlayer(target_id);
    if (!player) return std::numeric_limits<float>::max();
    const auto& p = player->GetPosition();
    float dx = position.x - p._41;
    float dy = position.y - p._42;
    float dz = position.z - p._43;
    return sqrtf(dx * dx + dz * dz);
}

bool Monster::IsPlayerNear(const PlayerManager& playerManager) const {
    return DistanceFromSpawnToPlayer(playerManager) <= MONSTER_CHASE_DISTANCE;
}

bool Monster::IsPlayerInAttackRange(const PlayerManager& playerManager) const {
    return DistanceToPlayer(playerManager) <= MONSTER_ATTACK_RANGE;
}

void Monster::SendSyncPacket(const Room& room) {
    sc_packet_monster_move pkt;
    pkt.size = sizeof(pkt);
    pkt.type = S2C_P_MONSTER_MOVE;
    pkt.monster_id = id;
    pkt.state = static_cast<int>(state);

    // 회전 방향 적용된 4x4 행렬 생성
    XMVECTOR forward = XMVector3Normalize(XMLoadFloat3(&lookDir));      //추적할때는 가장 가까운 플레이어를 바라보게 귀환할떄는 스폰 위치 바라보게
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, forward));
    up = XMVector3Cross(forward, right);

    XMMATRIX rotation = XMMATRIX(
        right,        // X축
        up,           // Y축
        forward,      // Z축
        XMVectorSet(position.x, position.y, position.z, 1.0f) // 위치
    );

    XMStoreFloat4x4(&pkt.pos, rotation);

    // XMStoreFloat4x4(&pkt.pos, XMMatrixTranslation(position.x, position.y, position.z));


    for (int pid : room.id)
        g_server.users[pid]->do_send(&pkt);
}

bool Monster::isBossMonster() {
    return type == MonsterType::Xenokarce ||
        type == MonsterType::Crassorrid ||
        type == MonsterType::Gorhorrid;
}

void Monster::ChangeBossAttack() {
    switch (type) {
    case MonsterType::Xenokarce:
        if (m_currentAttackType == 1) ATK = 200;
        else if (m_currentAttackType == 2) ATK = 230;
        break;
    case MonsterType::Crassorrid:
        if (m_currentAttackType == 1 || m_currentAttackType == 2) ATK = 200;
        else if (m_currentAttackType == 3) ATK = 250;
        break;
    case MonsterType::Gorhorrid:
        if (m_currentAttackType == 1) ATK = 300;
        else if (m_currentAttackType == 2) ATK = 350;
        else if (m_currentAttackType == 3) ATK = 300;
        break;
    default:
        break;
    }
}
