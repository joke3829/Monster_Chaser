#include "stdafx.h"
#include "Network.h"
#include <MSWSock.h>



extern Network g_server;
EXP_OVER::EXP_OVER(IO_OP op) : io_op(op) {
	ZeroMemory(&over, sizeof(over));
	wsabuf[0].buf = buffer;
	wsabuf[0].len = BUF_SIZE;
	accept_socket = INVALID_SOCKET;
}

SESSION::SESSION(int i, SOCKET s) : id(i), socket(s) {
	recv_over = new EXP_OVER(IO_RECV);
	do_recv();
}

SESSION::~SESSION() {
	sc_packet_leave lp;
	lp.size = sizeof(lp);
	lp.type = S2C_P_LEAVE;
	lp.id = id;
	for (auto& u : g_server.users) {
		if (id != u.first)
			u.second->do_send(&lp);
	}

	closesocket(socket);
	delete recv_over;
}

void SESSION::do_recv() {
	DWORD flags = 0;
	recv_over->wsabuf[0].buf = recv_over->buffer + remained;
	recv_over->wsabuf[0].len = BUF_SIZE - remained;
	WSARecv(socket, recv_over->wsabuf, 1, nullptr, &flags, &recv_over->over, nullptr);
}

void SESSION::do_send(void* packet) {
	EXP_OVER* over = new EXP_OVER(IO_SEND);
	int len = reinterpret_cast<unsigned char*>(packet)[0];
	memcpy(over->buffer, packet, len);
	over->wsabuf[0].len = len;
	WSASend(socket, over->wsabuf, 1, nullptr, 0, &over->over, nullptr);
}

void SESSION::process_packet(char* p) {
	char type = p[1];
	switch (type) {
	case C2S_P_LOGIN: {
	
		break;
	}
	case C2S_P_ENTER_ROOM: {
		cs_packet_enter_room* pkt = reinterpret_cast<cs_packet_enter_room*>(p);
		room_num = pkt->room_number;
		is_ready = false;
		g_server.rooms[room_num].setRoomNumber(room_num);
		if (g_server.rooms[room_num].IsAddPlayer())
		{
			g_server.rooms[room_num].Enterplayer();
			g_server.rooms[room_num].SendRoomInfo(); //send
		}
		else
		{
			cout << "이미 " << room_num << "번 방에는 사람이 꽉 찼습니다" << endl;
			break;
		}
		
		

		std::cout << "[클라이언트 " << id << "]이 " << (int)room_num << "번 방에 입장했습니다." << std::endl;
		break;
	}
	case C2S_P_READY: {
		cs_packet_ready* pkt = reinterpret_cast<cs_packet_ready*>(p);
		int id = pkt->id;
		room_num = pkt->room_number;		//이미 방에 들어갈 떄 진행하니까 굳이 필요는 x
		is_ready = true;
		g_server.rooms[room_num].ready_user++;


		sc_packet_set_ready rp;
		rp.size = sizeof(rp);
		rp.type = S2C_P_SETREADY;
		rp.id = id;
		rp.room_number = static_cast<char>(room_num);
		rp.is_ready = g_server.users[id]->is_ready;
		for (auto& player : g_server.users) {
			player.second->do_send(&rp);
		}
		
		std::cout << "[클라이언트 " << id << "]이 " << (int)room_num << "번 방에서 준비완료했습니다." << std::endl;
		break;
	}
	case C2S_P_READY_Cancel: {
		cs_packet_cancel_ready* pkt = reinterpret_cast<cs_packet_cancel_ready*>(p);
		int id = pkt->id;
		room_num = pkt->room_number;		//이미 방에 들어갈 떄 진행하니까 굳이 필요는 x
		is_ready = false;
		g_server.rooms[room_num].ready_user--;

		sc_packet_set_ready cp;
		cp.size = sizeof(cp);
		cp.type = S2C_P_SETREADY;
		cp.id = id;
		cp.room_number = static_cast<char>(room_num);
		cp.is_ready = g_server.users[id]->is_ready;
		for (auto& player : g_server.users) {
			player.second->do_send(&cp);
		}

		std::cout << "[클라이언트 " << id << "]이 준비를 취소했습니다." << std::endl;
		break;
	}


	case C2S_P_ROOM_REFRESH: {
		// Every rooms state send
		for (int i = 0; i < MAX_ROOM; ++i) {
			sc_packet_room_info pkt;
			pkt.size = sizeof(pkt);
			pkt.type = S2C_P_UPDATEROOM;
			pkt.room_number = static_cast<char>(i);
			pkt.player_count = static_cast<char>(g_server.rooms[i].GetPlayerCount());
			do_send(&pkt);
		}
		break;
	}
	case C2S_P_MOVE:
	{
		cs_packet_move* pkt = reinterpret_cast<cs_packet_move*>(p);
		//collision check
		/*{
			이동
		}*/

		sc_packet_move mp;
		mp.size = sizeof(mp);
		mp.type = S2C_P_MOVE;
		mp.id = id;
		mp.pos = pkt->pos;

		for (auto& u : g_server.users) {
			u.second->do_send(&mp);
		}
		BroadCasting_position(mp.pos);
		break;
	}
	}
}
void MoveCursorTo(int x, int y) {
	COORD pos = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
void SESSION::BroadCasting_position(const XMFLOAT4X4& pos)
{
	
	MoveCursorTo(0, 0);
	system("cls");
	cout << "=== [객체 postion] ===\n";

	cout << "x: " << pos._41 << "y: " << pos._42 << "z: " << pos._43 << endl;
	
}

void Network::Init() {
	WSAData wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	listen_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(listen_socket, (sockaddr*)&addr, sizeof(addr));
	listen(listen_socket, SOMAXCONN);

	iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listen_socket), iocp, 0, 0);

	for (int i = 0; i < MAX_ROOM; ++i)
		rooms[i].setRoomNumber(i);

	std::cout << "[서버 초기화 완료]" << std::endl;
}






