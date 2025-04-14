// Room.h
#pragma once

#include "protocol.h"

class SESSION;

class Room {
public:
    Room(int number);

    bool AddPlayer(SESSION* player);
    void BroadcastRoomInfo();
    void BroadcastReady(int player_id);
    int GetPlayerCount() const {
        return static_cast<int>(players.size());
    }
private:
    int room_number;
    std::vector<SESSION*> players;
};