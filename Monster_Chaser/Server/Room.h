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


	int getID(const int i) { return id[i]; }


	void setReadyUser(const int RU) { ready_user += RU; }

	void BroadCast_Room();

	bool IsStarted() const { return is_started; }
	void StartGame() { is_started = true; }
	void EndGame() { is_started = false; }
	std::vector<int>id;         //해당 방에 들어온 id 관리
private:
	int room_number;			// 방 번호
	int ready_user = 0;			// 레드 버튼을 누른 유저 수
	
	
	bool is_ready[3];


	bool is_started = false; // 게임이 시작되었는지 여부 다른 클라가 해당 방에 입장 못하도록 솔플 이나 듀오로 돌릴수도 있음
};