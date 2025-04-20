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


	bool IsStarted() const { return is_started; }
	void StartGame() { is_started = true; }
	void EndGame() { is_started = false; }
private:
	int room_number;			// 방 번호
	int ready_user = 0;			// 레드 버튼을 누른 유저 수 -> 
	std::vector<int>id;         //해당 방에 들어온 id 관리 
	bool is_started = false; // 게임이 시작되었는지 여부
};