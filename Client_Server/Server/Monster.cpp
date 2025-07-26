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
    float dy = spawnPoint.y - p._42;
    float dz = spawnPoint.z - p._43;
    return sqrtf(dx * dx + dy * dy + dz * dz);
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
    float dy = targetPos.y - position.y;
    float dz = targetPos.z - position.z;
    float len = sqrtf(dx * dx + dy * dy + dz * dz);
    if (len > 0.001f) {
        dx /= len; dy /= len; dz /= len;
        position.x += dx * speed * 0.016f;
        position.y += dy * speed * 0.016f;
        position.z += dz * speed * 0.016f;
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
    float dy = spawnPoint.y - position.y;
    float dz = spawnPoint.z - position.z;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    if (dist < 10.0f) {
        TransitionTo(MonsterState::Idle);
        return;
    }
    dx /= dist; dy /= dist; dz /= dist;
    position.x += dx * MONSTER_RETURN_SPEED * 0.016f;
    position.y += dy * MONSTER_RETURN_SPEED * 0.016f;
    position.z += dz * MONSTER_RETURN_SPEED * 0.016f;
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
    return sqrtf(dx * dx + dy * dy + dz * dz);
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
    XMStoreFloat4x4(&pkt.pos, XMMatrixTranslation(position.x, position.y, position.z));
    for (int pid : room.id) g_server.users[pid]->do_send(&pkt);
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
