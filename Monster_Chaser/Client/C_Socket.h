#pragma once
#include "stdafx.h"
#include "protocol.h"
#include "GameObject.h"
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
																																			
	bool getstart() { return InGameStart; }												
	void Setstart(const bool& st) { InGameStart = st; }			

	void set_id(int id) { my_id = id; }
	int get_id() const { return my_id; }
	//------------------------------------------------------------//									
																			
	SOCKET Getsocket() const { return m_socket; }														
private:																								
	SOCKET m_socket;																					
																						
	bool running{ true };																				
	bool InGameStart = false;																		
																					
	unsigned char remained{ 0 };																		
			

	int my_id = -1;
																										
																										
};

