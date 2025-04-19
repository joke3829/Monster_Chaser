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
																										
																										
																										
	bool Init(const char* ip, int port);	//���� �ʱ�ȭ												
	void do_recv();		//recv �޴°�																	
	void process_packet( char* ptr);	//��Ŷ ó��														
																										
	void send_packet(void* ptk);	//send�ϴ°� 														
	//--------------------------------------------------------------//									
	void DrawRoomList();			//�� ��Ȳ ���																																			
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

