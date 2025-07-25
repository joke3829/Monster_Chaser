// Room.cpp
#include "stdafx.h"
#include "Room.h"
#include "Network.h"


extern Network g_server;
int Room::room_num = 0;
Room::Room() : room_number(room_num++) {
	// 필요 시 초기화 코드 추가
}




bool Room::IsAddPlayer() {
	if (id.size() >= MAX_ROOM_MEMBER)
		return false;


	return true;
}

void Room::AddPlayer(const int& enter_id)
{
	int new_id = enter_id;
	id.emplace_back(new_id);


}








void Room::SendRoomInfo() {        //방 현황 보내주기
	sc_packet_room_info pkt;
	pkt.size = sizeof(pkt);
	pkt.type = S2C_P_UPDATEROOM;
	pkt.room_info;


	for (auto& player : g_server.users) {
		player.second->do_send(&pkt);
	}
}

void Room::BroadCast_Room()
{
	/*sc_packet_room_info rp;
	rp.size = sizeof(rp);
	rp.type = S2C_P_UPDATEROOM;
	rp.room_info[Getroom_number()] = GetPlayerCount();*/


	//for (auto& player : g_server.users) {							//send other player to broadcast room update
	//	//if (player.second->m_uniqueNo == this->m_uniqueNo)  // 나 자신은 제외
	//	//	continue;	
	//	player.second->do_send(&rp);								//if come UI Not to send me
	//}
	//g_server.users
}

void Room::StartGame()
{
	is_started = true;

	// 기존 StartGame 로직이 있다면 유지하고...
	//for (auto& [id, monster] : monsters) {
	//	sc_packet_monster_spawn sp;
	//	sp.size = sizeof(sp);
	//	sp.type = S2C_P_MONSTER_SPAWN;
	//	sp.monster_id = id;
	//	sp.monster_type = monster->GetType(); // 타입 나중에 추가 가능
	//	XMStoreFloat4x4(&sp.pos, XMMatrixTranslation(
	//		monster->GetPosition().x, 
	//		monster->GetPosition().y, 
	//		monster->GetPosition().z)
	//	);

	//	for (int pid : this->id)
	//		g_server.users[pid]->do_send(&sp);

	//	std::cout << "[몬스터 " << id << "] 초기 스폰 전송 완료\n";
	//}
	if (monsters.size() > 0) {
		std::thread([this]() {
			while (is_started) {
				for (auto& [id, monster] : this->monsters) {
					monster->Update(0.016f, *this, g_server.playerManager);
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(16));
			}
			}).detach();
	}
	
}

void Room::SpawnMonsters()
{
	// 예시로 1마리 생성
	int new_id = 0;
	/*XMFLOAT3 spawnPos = { -28.0f, 0.0f, -245.0f };
	monsters[new_id] = std::make_shared<Monster>(new_id, spawnPos);*/

	if (!monsters.empty()) return;

	// 예: 1스테이지 기준
	switch (stage)
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

void Room::setReady(int local_id, bool ready)
{
	if (local_id >= 0 && local_id < 3)
		player_readytoPlaygame[local_id] = ready;
}

bool Room::isAllGameStartReady() const
{
	int activePlayerCount = id.size();
	int readyCount = 0;
	for (int i = 0; i < activePlayerCount; ++i) {
		if (player_readytoPlaygame[i]) ++readyCount;
	}
	return readyCount == activePlayerCount;
	
}








