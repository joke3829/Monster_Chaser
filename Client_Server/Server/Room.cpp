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
	for (auto& [id, monster] : monsters) {
		sc_packet_monster_spawn sp;
		sp.size = sizeof(sp);
		sp.type = S2C_P_MONSTER_SPAWN;
		sp.monster_id = id;
		sp.monster_type = monster->GetType(); // 타입 나중에 추가 가능
		XMStoreFloat4x4(&sp.pos, XMMatrixTranslation(
			monster->GetPosition().x, 
			monster->GetPosition().y, 
			monster->GetPosition().z)
		);

		for (int pid : this->id)
			g_server.users[pid]->do_send(&sp);

		std::cout << "[몬스터 " << id << "] 초기 스폰 전송 완료\n";
	}
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
		//monsters[new_id++] = std::make_shared<Monster>(1000, XMFLOAT3(-28.0f, 0.0f, -235.0f), MonsterType::Feroptere);//5마리
		//monsters[new_id++] = std::make_shared<Monster>(1001, XMFLOAT3(-28.0f, 0.0f, -245.0f), MonsterType::Pistiripere);//5마리
		//monsters[new_id++] = std::make_shared<Monster>(1002, XMFLOAT3(-28.0f, 0.0f, -255.0f), MonsterType::RostrokarackLarvae);//5마리
		monsters[new_id] = std::make_shared<Monster>(new_id++, XMFLOAT3(-28.0f, 0.0f, -265.0f), MonsterType::XenokarceBoss); // 보스
		break;
	case 2:
		break; // 2스테이지 몬스터는 아직 정의되지 않음
	case 3:
		break; // 3스테이지 몬스터는 아직 정의되지 않음
	default:
		break;
	}
	

}








