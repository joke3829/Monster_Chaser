// Room.h
#pragma once

#include "protocol.h"
#include "Monster.h"
#include "array"




class SESSION;
class Network;
class Monster;
class Room {
public:
	Room();
	
	bool IsAddPlayer();
	void AddPlayer(const int& enter_id);
	void SendRoomInfo();

	short GetPlayerCount() { return id.size(); }
	int GetReadyUser() { return ready_user; }
	int Getroom_number() { return room_number; }


	int getID(const int i) { return id[i]; }


	void setReadyUser(const int RU) { ready_user += RU; }

	void BroadCast_Room();

	bool IsStarted() const { return is_started; }

	void StartGame();
	void EndGame() { is_started = false; }

	void SpawnMonsters();

	void setReady(int local_id, bool ready);

	void setStage(short new_stage) { stage = new_stage; }
	short getStage() const { return stage; }
	bool isAllGameStartReady() const;
	concurrent_unordered_map<int, shared_ptr<Monster>> monsters;
	std::vector<int>id;        //�ش� �濡 ���� id ���� -> ���� �ʿ��� 
	concurrency::concurrent_unordered_map<int, short> selected_characters; // ĳ���� ���� ���� (�߰�)
	
	mutex RoomMutex;
	bool bStageActive = false;  // ���������� ����������


private:
	int room_number;			// �� ��ȣ
	int ready_user = 0;			// ���� ��ư�� ���� ���� ��
	
	static int room_num;
	

	array<bool, 3> player_readytoPlaygame = { false, false, false }; // �÷��̾� �غ� ����

	short stage = SCENE_TITLE; // ���� �������� (��: 0: �ΰ��� X 1:��������1, 2: �������� 2, 3: �������� 3)

	bool is_started = false; // ������ ���۵Ǿ����� ���� �ٸ� Ŭ�� �ش� �濡 ���� ���ϵ��� ���� �̳� ����� �������� ����

};