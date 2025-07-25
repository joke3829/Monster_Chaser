﻿// Room.cpp
#include "stdafx.h"
#include "Room.h"
#include "Network.h"

extern Network g_server;

Room::Room() {}

Room::~Room() {
	StopGame();
}

void Room::setReady(int local_id, bool state) {
	player_ready[local_id] = state;
}

bool Room::isAllGameStartReady() {
	int count = GetPlayerCount();
	for (int i = 0; i < count; ++i)
		if (!player_ready[i]) return false;
	return true;
}

void Room::InitailizeReadyingame() {
	for (auto& r : player_ready) r = false;
}

void Room::setReadyUser(int delta) {
	ready_user += delta;
}

int Room::GetReadyUser() {
	return ready_user;
}

int Room::GetPlayerCount() {
	return static_cast<int>(id.size());
}

bool Room::IsAddPlayer() {
	return id.size() < MAX_ROOM_MEMBER;
}

void Room::AddPlayer(int pid) {
	id.push_back(pid);
}

void Room::RemovePlayer(int pid) {
	id.erase(std::remove(id.begin(), id.end(), pid), id.end());
}

void Room::StartGame() {
	if (!bMonsterThreadRunning) {
		bMonsterThreadRunning = true;
		monsterThread = std::thread(&Room::MonsterThreadFunction, this);
	}
}

void Room::StopGame() {
	bMonsterThreadRunning = false;
	if (monsterThread.joinable())
		monsterThread.join();
}

void Room::MonsterThreadFunction() {
	auto lastUpdateTime = std::chrono::steady_clock::now();

	while (bMonsterThreadRunning) {
		if (!bStageActive) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		auto now = std::chrono::steady_clock::now();
		float deltaTime = std::chrono::duration<float>(now - lastUpdateTime).count();
		lastUpdateTime = now;

		// 몬스터 업데이트
		for (auto& [id, monster] : monsters)
			monster->Update(deltaTime, *this, g_server.playerManager);

		// 플레이어 MP 회복 및 버프 상태 갱신
		for (int i = 0; i < GetPlayerCount(); ++i) {
			auto now = std::chrono::steady_clock::now();
			float elapsed = std::chrono::duration<float>(now - lastHitTime[i]).count();

			auto player = g_server.playerManager.GetPlayer(id[i]);
			if (!player) continue;

			if (elapsed >= 10.0f) {
				player->RecoverSkillCost(1); // 초당 1 회복
			}

			player->UpdateBuffStatesIfChanged();  // ✅ 버프 상태 변경 감지 및 패킷 전송
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(16)); // 약 60fps 주기
	}
}
void Room::setStage(short stage)
{
	currentStage = stage;
}
void Room::SpawnMonsters()
{
	// 예시로 1마리 생성
	int new_id = 0;
	/*XMFLOAT3 spawnPos = { -28.0f, 0.0f, -245.0f };
	monsters[new_id] = std::make_shared<Monster>(new_id, spawnPos);*/

	if (!monsters.empty()) return;

	// 예: 1스테이지 기준
	switch (currentStage)
	{
	case 1:
	{
		// Feroptere - 3마리
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-280.0f, 0.0f, 215.4f), MonsterType::Feroptere);
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-246.3, 0, 15.1), MonsterType::Feroptere);

		// Pistiripere - 3마리
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-240, 0, 149.8), MonsterType::Pistiripere);
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-351.1, 0, 26.7), MonsterType::Pistiripere);

		// RostrokarackLarvae - 4마리

		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-150.5, 0, 85.7), MonsterType::RostrokarackLarvae);
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-164.7, 0, 66), MonsterType::RostrokarackLarvae);

		// Boss - Xenokarce
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-306.7, 0, -150.8), MonsterType::Xenokarce);

		break;
	}
	case 2:
	{
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-18.0f, -0.14f, -15.5f), MonsterType::Limadon);
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-30.0f, 0.0f, 21.0f), MonsterType::Occisodonte);
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-160.0f, 1.3f, 78.6f), MonsterType::Fulgurodonte);
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-152.1f, 2.0f, 246.3f), MonsterType::Limadon);
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-128.6f, 2.2f, 272.8f), MonsterType::Fulgurodonte);
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-97.0f, 3.1f, 315.3f), MonsterType::Occisodonte);
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-92.0f, 4.2f, 376.5f), MonsterType::Fulgurodonte);

		// Boss: Crassorrid (추가 MonsterType 필요 시 정의해둬야 함)
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(0.5f, 5.2f, 362.8f), MonsterType::Crassorrid);

		break;
	}
	case 3:
	{
		// Final Boss: Gorhorrid
		monsters[new_id++] = std::make_shared<Monster>(new_id, XMFLOAT3(-86.3f, 0.0f, -301.1f), MonsterType::Gorhorrid);
		break;
	}
	default:
		break;
	}


}








