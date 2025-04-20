// Room.h
#pragma once

#include "protocol.h"


class SESSION;
class Network;

class Room {
public:
	Room(int num);;

	bool IsAddPlayer();
	void AddPlayer(const int& enter_id);
	void SendRoomInfo();

	int GetPlayerCount() { return id.size(); }
	int GetReadyUser() { return ready_user; }
	int Getroom_number() { return room_number; }


	int getID(const int& i) { return id[i]; }

	void setReadyUser(const int& RU) { ready_user += RU; }

private:
	int room_number;			// �� ��ȣ
	int ready_user = 0;			// ���� ��ư�� ���� ���� �� -> 
	std::vector<int>id;         //�ش� �濡 ���� id ���� 

};