#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "GameObject.h"
#include "mutex"
#include "ObjectManager.h"

																										
																										
class C_Socket																							
{																										
public:																									
	C_Socket();																							
	~C_Socket();																						
																										
																										
																										
	bool Init(const char* ip, int port);	//소켓 초기화												
	void do_recv();		//recv 받는곳																	
	void process_packet( char* ptr);	//패킷 처리														
																										
	void send_packet(void* ptk);	//send하는곳 														
	//--------------------------------------------------------------//									
	void DrawRoomList();			//방 현황 출력																																			
	bool get_ready_to_start() { return ready_to_start; }												
	std::thread drawThread;																				
	//------------------------------------------------------------//									
																			
	SOCKET Getsocket() const { return m_socket; }														
private:																								
	SOCKET m_socket;																					
																						
	bool running{ true };																				
	bool ready_to_start = false;																		
	std::mutex mtx;																						
	unsigned char remained{ 0 };																		
	char room_players[MAX_ROOM] = {};																	
																										
																										
};

