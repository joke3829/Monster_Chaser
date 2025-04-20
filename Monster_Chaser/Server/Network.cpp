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

SESSION::SESSION(SOCKET s) : socket(s) {
	recv_over = new EXP_OVER(IO_RECV);
	do_recv();
}

SESSION::~SESSION() {
	

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


		int id = pkt->id;
		int room_num = static_cast<int>(pkt->room_number);
		bool is_ready = false;

		
		if (g_server.rooms[room_num].IsAddPlayer())			//id ���� �ɹ� ������ �� 
		{
			g_server.rooms[room_num].AddPlayer(id);
			
			//g_server.rooms[room_num].SendRoomInfo(); //send
		}
		else
		{
			cout << "�̹� " << room_num << "�� �濡�� ����� �� á���ϴ�" << endl;
			break;
		}
		
		sc_packet_select_room sp;
		sp.size = sizeof(sp);
		sp.type = S2C_P_SELECT_ROOM;
		sp.id = id;
		sp.room_number = room_num;
		sp.players_inRoom = g_server.rooms[room_num].GetPlayerCount();
		for (auto& player : g_server.users) {
			player.second->do_send(&sp);
		}


		for (auto& u : g_server.users) {			//�ű� Ŭ�� ���� Ŭ���� ���縦 �ν� 
			if (u.first != id)
			{
				sc_packet_select_room sp;
				sp.size = sizeof(sp);
				sp.type = S2C_P_SELECT_ROOM;
				sp.id = u.first;
				g_server.users[id]->do_send(&sp);
			}
		}

		std::cout << "[Ŭ���̾�Ʈ " << id << "]�� " << (int)room_num << "�� �濡 �����߽��ϴ�." << std::endl;
		break;
	}
	case C2S_P_READY: {
		cs_packet_ready* pkt = reinterpret_cast<cs_packet_ready*>(p);
		int id = pkt->id;
		int room_num = static_cast<int>(pkt->room_number);		//�̹� �濡 �� �� �����ϴϱ� ���� �ʿ�� x
		bool is_ready = true;
		g_server.rooms[room_num].setReadyUser(1);

		if (g_server.rooms[room_num].GetReadyUser() >= 3)		//�� �濡 3���� �غ� �� �Ϸ� �ƴٴ� ��		//�ΰ��� �غ�Ϸ� 
		{
			
			sc_packet_Ingame_start sp;
			sp.size = sizeof(sp);
			sp.type = S2C_P_ALLREADY;
			sp.room_number = room_num;
			
			for (int i = 0; i < g_server.rooms[room_num].GetPlayerCount(); ++i) {
				sp.ready_id[i] = g_server.rooms[room_num].getID(i);  // ID ��Ͽ�
			}
		
			for (int i = 0; i < g_server.rooms[room_num].GetPlayerCount(); ++i) {
				int target_id = g_server.rooms[room_num].getID(i);
				if (g_server.users.contains(target_id)) {
					g_server.users[target_id]->do_send(&sp);
				}
			}
			
			
		}
		else 
		{
			sc_packet_set_ready rp;
			rp.size = sizeof(rp);
			rp.type = S2C_P_SETREADY;
			rp.id = id;
			rp.room_number = static_cast<char>(room_num);
			rp.is_ready = is_ready;
			for (auto& player : g_server.users) {
				player.second->do_send(&rp);
			}

			std::cout << "[Ŭ���̾�Ʈ " << id << "]�� " << (int)room_num << "�� �濡�� �غ�Ϸ��߽��ϴ�." << std::endl;
		}
		break;
	}
	case C2S_P_READY_Cancel: {
		cs_packet_cancel_ready* pkt = reinterpret_cast<cs_packet_cancel_ready*>(p);
		int id = pkt->id;
		int room_num = static_cast<int>(pkt->room_number);
		bool is_ready = false;
		g_server.rooms[room_num].setReadyUser(-1);

		sc_packet_set_ready cp;
		cp.size = sizeof(cp);
		cp.type = S2C_P_SETREADY;
		cp.id = id;
		cp.room_number = room_num;
		cp.is_ready = is_ready;
		for (auto& player : g_server.users) {
			player.second->do_send(&cp);
		}

		std::cout << "[Ŭ���̾�Ʈ " << id << "]�� �غ� ����߽��ϴ�." << std::endl;
		break;
	}


	case C2S_P_ROOM_UPDATE: {
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
		int id = pkt->id;
		auto pos = pkt->pos;
		//collision check
		/*{
			�̵�
		}*/

		sc_packet_move mp;
		mp.size = sizeof(mp);
		mp.type = S2C_P_MOVE;
		mp.id = id;
		mp.pos = pos;

		for (auto& u : g_server.users) {
			u.second->do_send(&mp);
		}
		BroadCasting_position(mp.pos,id);
		break;
	}
	}
}
void MoveCursorTo(int x, int y) {
	COORD pos = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
void SESSION::BroadCasting_position(const XMFLOAT4X4& pos,const int& id)
{
	
	MoveCursorTo(0, 0);
	system("cls");
	cout << "=== [��ü"<<id <<"postion]" "== ";

	cout << "x: " << pos._41 << "y: " << pos._42 << "z: " << pos._43 << endl;
	
}

Network::Network()
{
	for (int i = 0; i < 10; ++i) {
		rooms.emplace_back(i);  // �� �� Room�� �ε����� �ѱ�
	}
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

	

	std::cout << "[���� �ʱ�ȭ �Ϸ�]" << std::endl;
}






