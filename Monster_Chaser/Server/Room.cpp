// Room.cpp
#include "stdafx.h"
#include "Network.h"
#include "Room.h"

Room::Room(int number) : room_number(number) {}

bool Room::AddPlayer(SESSION* player) {
    if (players.size() >= MAX_ROOM_MEMBER) return false;
    players.emplace_back(player);
    return true;
}

void Room::BroadcastRoomInfo() {
    s2c_packet_room_info pkt;
    pkt.size = sizeof(pkt);
    pkt.type = S2C_P_ROOM_INFO;
    pkt.room_number = static_cast<char>(room_number);
    pkt.player_count = static_cast<char>(players.size());

    for (auto* player : players) {
        player->do_send(&pkt);
    }
}

void Room::BroadcastReady(int player_id) {
    s2c_packet_ready_broadcast pkt;
    pkt.size = sizeof(pkt);
    pkt.type = S2C_P_READY_BROADCAST;
    pkt.client_id = player_id;
    pkt.room_number = static_cast<char>(room_number);

    for (auto* player : players) {
        if (player->id == player_id) {
            pkt.is_ready = player->is_ready ? 1 : 0;
            break;
        }
    }
    for (auto* player : players) {
        player->do_send(&pkt);
    }
}

int Room::GetPlayerCount() const
{
        return static_cast<int>(players.size());
}

