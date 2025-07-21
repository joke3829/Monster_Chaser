#include "stdafx.h"
#include "Monster.h"
#include "Network.h"
#include <random>

#define MONSTER_CHASE_DISTANCE 100.0f
#define MONSTER_ATTACK_RANGE 30.0f
constexpr float MONSTER_ATTACK_COOLDOWN = 2.0f;   // 공격 쿨타임 2초
constexpr float MONSTER_RETURN_SPEED = 80.0f;     // 귀환 속도
std::random_device rd;


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

Monster::Monster(int id, const XMFLOAT3& spawnPos, MonsterType t)
    : id(id), hp(100), state(MonsterState::Idle), position(spawnPos), spawnPoint(spawnPos), type(t){

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(30, 50);

    gold = dis(gen);  //  30~50 골드 설정
}

void Monster::Update(float deltaTime, const Room& room, const PlayerManager& playerManager) {
    std::lock_guard<std::mutex> lock(mtx);

    if (hp <= 0) {
        TransitionTo(MonsterState::Dead);
    }

    switch (state) {
    case MonsterState::Idle:    HandleIdle(room, playerManager); break;
    case MonsterState::Chase:   HandleChase(playerManager, room); break;
    case MonsterState::Attack:  HandleAttack(playerManager); break;
    case MonsterState::Return:  HandleReturn(); break;
    case MonsterState::Dead:    HandleDead(room); return; // 죽으면 패킷 보내지 않음
    }

    SendSyncPacket(room); //  매 업데이트마다 위치+상태 전송
}

bool Monster::TakeDamage(int dmg) {
    std::lock_guard<std::mutex> lock(mtx);
    hp -= dmg;
    if (hp <= 0) {
        TransitionTo(MonsterState::Dead);
        return true;
    }
    return false;
}

void Monster::TransitionTo(MonsterState nextState) {
    if (state != nextState) state = nextState;
}

void Monster::HandleIdle(const Room& room, const PlayerManager& playerManager) {
    target_id = FindClosestPlayerInRoom(room, position, playerManager);
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
    target_id = FindClosestPlayerInRoom(room, position, playerManager);

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

void Monster::HandleAttack(const PlayerManager& playerManager) {
    if (!IsPlayerNear(playerManager)) {
        TransitionTo(MonsterState::Return);
        return;
    }

    if (!IsPlayerInAttackRange(playerManager)) {
        TransitionTo(MonsterState::Chase);
        return;
    }

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - lastAttackTime).count();

    if (elapsed >= MONSTER_ATTACK_COOLDOWN) {
        lastAttackTime = now;

        //  실제 데미지 적용
        auto player = playerManager.GetPlayer(target_id);
        if (player) {
            player->TakeDamage(10);  // 예: 고정 데미지 10
            std::cout << "[몬스터 " << id << "] 플레이어 " << target_id << " 공격!\n";
        }
    }
}


void Monster::HandleReturn() {
    float dx = spawnPoint.x - position.x;
    float dy = spawnPoint.y - position.y;
    float dz = spawnPoint.z - position.z;

    float dist = sqrtf(dx * dx + dy * dy + dz * dz);
    if (dist < 10.0f) {
        TransitionTo(MonsterState::Idle);
        return;
    }

    //  이동 방향 계산
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
        hp = 100;
        isRespawning = false;
        position = spawnPoint;
        TransitionTo(MonsterState::Idle);

        sc_packet_monster_respawn pkt;
        pkt.size = sizeof(pkt);
        pkt.type = S2C_P_MONSTER_RESPAWN;
        pkt.monster_id = id;
        XMStoreFloat4x4(&pkt.pos, XMMatrixTranslation(position.x, position.y, position.z));

        for (int pid : room.id) {
            g_server.users[pid]->do_send(&pkt);
        }
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
    return DistanceToPlayer(playerManager) <= MONSTER_CHASE_DISTANCE;
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

    for (int pid : room.id)
        g_server.users[pid]->do_send(&pkt);
}
