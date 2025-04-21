// Room.cpp
#include "stdafx.h"
#include "Room.h"
#include "Network.h"


extern Network g_server;
Room::Room(int num) : room_number(num) {
	// �ʿ� �� �ʱ�ȭ �ڵ� �߰�
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








void Room::SendRoomInfo() {        //�� ��Ȳ �����ֱ�
	sc_packet_room_info pkt;
	pkt.size = sizeof(pkt);
	pkt.type = S2C_P_UPDATEROOM;
	pkt.room_number = static_cast<char>(room_number);
	pkt.player_count = id.size();

	for (auto& player : g_server.users) {
		player.second->do_send(&pkt);
	}
}








