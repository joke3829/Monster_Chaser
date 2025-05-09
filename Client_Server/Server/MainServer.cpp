
// MainServer.cpp
#include "stdafx.h"
#include "Room.h"
#include "Network.h"
#include <MSWSock.h>



mutex myMutex;
Network g_server;
std::atomic<int> g_client_id = 0;
std::atomic<int>  g_monster_id = 50000;
void do_accept(Network& server) {
	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	EXP_OVER* accept_over = new EXP_OVER(IO_ACCEPT);
	accept_over->accept_socket = c_socket;

	AcceptEx(server.listen_socket, c_socket, accept_over->buffer, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		NULL, &accept_over->over);
}

int main() {

	g_server.Init();

	do_accept(g_server);
	//netework에 있는 소켓으로 처리 
	while (true) {
		DWORD bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over;

		BOOL success = GetQueuedCompletionStatus(g_server.iocp, &bytes, &key, &over, INFINITE);
		if (!success) continue;

		EXP_OVER* exp = reinterpret_cast<EXP_OVER*>(over);

		switch (exp->io_op) {
		case IO_ACCEPT:
		{
			int client_id = g_client_id++;
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(exp->accept_socket), g_server.iocp, client_id, 0);
			g_server.users[client_id] = std::make_unique<SESSION>(client_id, exp->accept_socket);



			sc_packet_enter ep;
			ep.size = sizeof(ep);
			ep.type = S2C_P_ENTER;
			for (auto& u : g_server.users) {			//모든 유저에게 정보 보냄 
				//if(u.first !=client_id)
				u.second->do_send(&ep);
			}


			for (auto& u : g_server.users) {			//신규 클라가 기존 클라의 존재를 인식 
				if (u.first != client_id)
				{
					sc_packet_enter ep;
					ep.size = sizeof(ep);
					ep.type = S2C_P_ENTER;
					g_server.users[client_id]->do_send(&ep);
				}
			}

			std::cout << "[클라이언트 " << client_id << " ]" << "이 접속했습니다." << std::endl;
			do_accept(g_server);
			
		}
		break;
		case IO_RECV:
		{
			SESSION* user = g_server.users[key].get();
			char* ptr = exp->buffer;
			int processed = 0;
			bytes += user->remained;
			while (processed < bytes) {
				int size = ptr[0];
				if (size > bytes - processed)
					break;
				user->process_packet(ptr);
				ptr += size;
				processed += size;

			}
			user->remained = bytes - processed;

			if (user->remained > 0) {
				memcpy(exp->buffer, ptr, user->remained);
			}

			user->do_recv();

		}
		break;
		case IO_SEND:

			delete exp;
			break;


		}
	}
	return 0;
}


