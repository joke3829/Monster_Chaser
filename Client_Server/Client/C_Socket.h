#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "GameObject.h"
//#include "ObjectManager.h"
#include "Scene.h"
#include <mutex>


class C_Socket
{
public:
	C_Socket();
	~C_Socket();



	bool Init(const char* ip, int port);	//소켓 초기화												
	void do_recv();		//recv 받는곳																	
	void process_packet(char* ptr);	//패킷 처리														

	void send_packet(void* ptk);	//send하는곳 			
	//--------------------------------------------------------------//	
	void SendLogin(const char* UserID, const char* Userpassword);									//Send LoginPacket
	void SendCreateUser(const char* UserID, const char* Userpassword, const char* userNickName);	//Send CreateUserPacket
	void SendEnterRoom(const short RoomNum);															//Send EnterroomPacket
	void SendsetReady(const bool isReady, const int room_num);										//Send Readypakcet if isReday is true->player set ready
	void SendBroadCastRoom();																		
	void SendMovePacket(const float& Time, const UINT State);							//SendMovePacket	
	void SendPickCharacter(const short RoomNum, const short Job);

	//--------------------------------------------------------------//									

	bool getstart() { return InGameStart; }
	void Setstart(const bool& st) { InGameStart = st; }

	void set_id(int id) { my_id = id; }
	int get_id() const { return my_id; }
	//------------------------------------------------------------//									
	std::mutex myMutex;
	SOCKET Getsocket() const { return m_socket; }
private:
	SOCKET m_socket;

	bool running{ true };
	bool InGameStart = false;

	unsigned char remained{ 0 };


	int my_id = -1;


};

