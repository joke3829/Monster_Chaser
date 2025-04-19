// Room.cpp
#include "stdafx.h"
#include "Room.h"
#include "Network.h"


extern Network g_server;
Room::Room()
{
}

bool Room::IsAddPlayer() {
    if (playerInRoom >= MAX_ROOM_MEMBER) 
        return false;
   
    return true;
}

void Room::SendRoomInfo() {        //방 현황 보내주기
    sc_packet_room_info pkt;
    pkt.size = sizeof(pkt);
    pkt.type = S2C_P_UPDATEROOM;
    pkt.room_number = static_cast<char>(room_number);
    pkt.player_count = playerInRoom;

    for (auto& player : g_server.users) {
        player.second->do_send(&pkt);
    }
}



int Room::GetPlayerCount() const
{
        return static_cast<int>(playerInRoom);
}

void Room::setRoomNumber(const short& num)
{
    room_number = num;
}

