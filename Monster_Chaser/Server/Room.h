// Room.h
#pragma once

#include "protocol.h"


class SESSION;
class Network;

class Room {
public:
    Room();

    bool IsAddPlayer();
    void SendRoomInfo();
   
    int GetPlayerCount() const;
    void setRoomNumber(const short& num);
    void Enterplayer() { playerInRoom++; }
    short ready_user = 0;
private:
    short room_number;
    short playerInRoom = 0;
};