#include "stdafx.h"
#include "Monster.h"
#include "Network.h"

constexpr float MONSTER_CHASE_DISTANCE = 100.0f;
constexpr float MONSTER_ATTACK_RANGE = 30.0f;

extern Network g_server;

// 가장 가까운 플레이어를 찾는 함수
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

Monster::Monster(int id, const XMFLOAT3& spawnPos)
    : id(id), hp(100), state(MonsterState::Idle), position(spawnPos), spawnPoint(spawnPos) {
}

void Monster::Update(float deltaTime, const Room& room, const PlayerManager& playerManager) {
    std::lock_guard<std::mutex> lock(mtx);
    if (hp <= 0) {
        TransitionTo(MonsterState::Dead);
    }

    switch (state) {
    case MonsterState::Idle:
        HandleIdle(room, playerManager);
        break;
    case MonsterState::Chase:
        HandleChase(room, playerManager);
        break;
    case MonsterState::Attack:
        HandleAttack(playerManager);
        break;
    case MonsterState::Return:
        HandleReturn();
        break;
    case MonsterState::Dead:
        HandleDead(room);
        break;
    }
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

void Monster::HandleChase(const Room& room,const PlayerManager& playerManager ) {
    if (!IsPlayerNear(playerManager)) {
        TransitionTo(MonsterState::Return);
        return;
    }

  
    if (IsPlayerInAttackRange(playerManager)) {
        TransitionTo(MonsterState::Attack);
        return;
    }

    //  이동 방향 계산
    auto target = playerManager.GetPlayer(target_id);
    if (!target) return;

    XMFLOAT3 targetPos = {
        target->GetPosition()._41,
        target->GetPosition()._42,
        target->GetPosition()._43
    };

    float speed = 10.0f; // 유닛/sec
    float dx = targetPos.x - position.x;
    float dy = targetPos.y - position.y;
    float dz = targetPos.z - position.z;

    float length = sqrtf(dx * dx + dy * dy + dz * dz);
    if (length > 0.001f) {
        dx /= length; dy /= length; dz /= length;

        position.x += dx * speed * 0.016f;
        position.y += dy * speed * 0.016f;
        position.z += dz * speed * 0.016f;
    }

	  SendMovePacket(room);
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


    // 공격 처리 (쿨타임 생략)
    // 예: playerManager.ApplyDamage(target_id, 10);
}

void Monster::HandleReturn() {
    float dx = position.x - spawnPoint.x;
    float dy = position.y - spawnPoint.y;
    float dz = position.z - spawnPoint.z;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    if (dist < 10.f) {
        TransitionTo(MonsterState::Idle);
        return;
    }

    // 복귀 이동 처리
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

        // 클라이언트에게 몬스터 리스폰 알림
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
void Monster::SendMovePacket(const Room& room)
{
    sc_packet_monster_move pkt;
    pkt.size = sizeof(pkt);
    pkt.type = S2C_P_MONSTER_MOVE;
    pkt.monster_id = id;
    XMStoreFloat4x4(&pkt.pos, XMMatrixTranslation(position.x, position.y, position.z));
    for (int pid : room.id) {
        g_server.users[pid]->do_send(&pkt);
	}
    std::cout << "[몬스터 " << id << "] " << "X: " << position.x << "Y: " << position.y << "Z: " << position.z << endl;
}
