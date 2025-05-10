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








