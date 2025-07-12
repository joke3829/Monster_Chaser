#include "stdafx.h"
#include "Network.h"




extern Network g_server;
extern mutex myMutex;
EXP_OVER::EXP_OVER(IO_OP op) : io_op(op) {
	ZeroMemory(&over, sizeof(over));
	wsabuf[0].buf = buffer;
	wsabuf[0].len = BUF_SIZE;
	accept_socket = INVALID_SOCKET;
}

SESSION::SESSION(int Num, SOCKET s) :m_uniqueNo(Num), socket(s) {
	recv_over = std::make_unique<EXP_OVER>(IO_RECV);
	do_recv();
}

SESSION::~SESSION() {


	closesocket(socket);

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
		int room_Num = static_cast<int>(pkt->room_number);
		lock_guard<mutex> lock(g_server.rooms[room_Num].RoomMutex);

		if (!g_server.rooms[room_Num].IsAddPlayer()) {
			std::cout << "이미 " << room_Num << "번 방에는 사람이 꽉 찼습니다" << std::endl;
			break;
		}

		local_id = g_server.rooms[room_Num].GetPlayerCount();  // Local_id 할당
		g_server.rooms[room_Num].AddPlayer(m_uniqueNo);
		room_num = room_Num;

		// ?? 1. 현재 유저의 입장 정보를 모든 유저에게 전송
		for (auto& id : g_server.rooms[room_Num].id) {
			sc_packet_select_room sp;
			sp.size = sizeof(sp);
			sp.type = S2C_P_SELECT_ROOM;
			sp.Local_id = local_id;
			sp.room_number = static_cast<char>(room_Num);
			sp.is_self = (id == m_uniqueNo);  // 자기 자신에게만 true

			g_server.users[id]->do_send(&sp);
		}

		// ?? 2. 기존 유저들의 존재 정보를 나에게만 전송
		for (int existing_id : g_server.rooms[room_num].id) {
			if (existing_id == m_uniqueNo) continue;

			sc_packet_select_room sp_existing;
			sp_existing.size = sizeof(sp_existing);
			sp_existing.type = S2C_P_SELECT_ROOM;
			sp_existing.Local_id = g_server.users[existing_id]->local_id;
			sp_existing.room_number = static_cast<char>(room_num);
			sp_existing.is_self = false;  // 무조건 false

			g_server.users[m_uniqueNo]->do_send(&sp_existing);
		}

		// ?? 3. 기존 유저들의 캐릭터 선택 정보 전송
		for (const auto& [existing_id, char_type] : g_server.rooms[room_num].selected_characters) {
			sc_packet_pickcharacter cp;
			cp.size = sizeof(cp);
			cp.type = S2C_P_PICKCHARACTER;
			cp.C_type = char_type;
			cp.Local_id = g_server.users[existing_id]->local_id;

			g_server.users[m_uniqueNo]->do_send(&cp);
		}

		// ?? 4. 방 전체 인원 정보 업데이트
		sc_packet_room_info rp;
		rp.size = sizeof(rp);
		rp.type = S2C_P_UPDATEROOM;
		for (int i = 0; i < g_server.rooms.size(); ++i)
			rp.room_info[i] = g_server.rooms[i].GetPlayerCount();

		for (auto& player : g_server.users)
			player.second->do_send(&rp);

		std::cout << "[로컬아이디 " << local_id << "을 가진 클라이언트 " << m_uniqueNo
			<< "]이 " << (int)room_num << "번 방에 입장했습니다." << std::endl;
		break;
	}

	case C2S_P_PICKCHARACTER:
	{
		cs_packet_pickcharacter* pkt = reinterpret_cast<cs_packet_pickcharacter*>(p);

		short Character_type = pkt->C_type;
		int room_num = static_cast<int>(pkt->room_number);
		g_server.rooms[room_num].selected_characters[m_uniqueNo] = Character_type;


		sc_packet_pickcharacter cp;
		cp.size = sizeof(cp);
		cp.type = S2C_P_PICKCHARACTER;
		cp.Local_id = local_id;
		cp.C_type = Character_type;

		for (auto& id : g_server.rooms[room_num].id)
			g_server.users[id]->do_send(&cp);
		
		break;
	}
	case C2S_P_GetREADY: {
		
		cs_packet_getready* pkt = reinterpret_cast<cs_packet_getready*>(p);
		std::vector<int> room_players;
		int room_num = static_cast<int>(pkt->room_number);		//이미 방에 들어갈 떄 진행하니까 굳이 필요는 x
		bool ready = pkt->isReady;
		if (ready == true)
		{
			g_server.rooms[room_num].setReadyUser(1);

		}
		else
		{
			g_server.rooms[room_num].setReadyUser(-1);

		}
		sc_packet_set_ready rp;
		rp.size = sizeof(rp);
		rp.type = S2C_P_SETREADY;
		rp.Local_id = local_id;
		rp.room_number = static_cast<char>(room_num);
		rp.is_ready = ready;
		{
			lock_guard<mutex> lock(g_server.rooms[room_num].RoomMutex);
			for (int i = 0; i < g_server.rooms[room_num].id.size(); ++i)
				room_players.emplace_back(g_server.rooms[room_num].getID(i));		//room_players에 해당 방에 들어가 있는 ID값 넣기
		}
		for (auto& id : room_players)					//나를 제외한 나머지 방에있는 플레이어들에게 보내기
			if (id != m_uniqueNo)
				g_server.users[id]->do_send(&rp);


		// 3명 다 준비 완료일 때 인게임 넘어가는 패킷 넘어주기 
		if (g_server.rooms[room_num].GetReadyUser() == g_server.rooms[room_num].id.size()) {

			sc_packet_Ingame_start sp;
			sp.size = sizeof(sp);
			sp.type = S2C_P_ALLREADY;
			sp.room_number = static_cast<char>(room_num);
			//sp.Local_id = g_server.users[m_uniqueNo]->local_id;


			g_server.rooms[room_num].StartGame();
			g_server.users[m_uniqueNo]->room_num = room_num;
			for (auto& id : room_players)
				g_server.users[id]->do_send(&sp);

		}


		else
		{

			if (ready)
				std::cout << "[클라이언트 " << m_uniqueNo << "]이 " << (int)room_num << "번 방에서 준비완료했습니다." << std::endl;
			else
				std::cout << "[클라이언트 " << m_uniqueNo << "]이 준비를 취소했습니다." << std::endl;
		}
		break;
	}

	case C2S_P_ROOM_UPDATE: {
		// Every rooms state send

		sc_packet_room_info pkt;
		pkt.size = sizeof(pkt);
		pkt.type = S2C_P_UPDATEROOM;
		for (int i = 0; i < g_server.rooms.size(); ++i) {
			pkt.room_info[i] = g_server.rooms[i].GetPlayerCount();
		}
		for (auto& player : g_server.users)
			player.second->do_send(&pkt);

		break;
	}
	case C2S_P_MOVE:
	{
		auto start_time = high_resolution_clock::now();
		cs_packet_move* pkt = reinterpret_cast<cs_packet_move*>(p);

		m_pos = pkt->pos;
		float time = pkt->time;
		unsigned int state = pkt->state;


		//collision check
		/*{
			이동
		}*/



		sc_packet_move mp;
		mp.size = sizeof(mp);
		mp.type = S2C_P_MOVE;
		mp.Local_id =local_id;
		mp.pos = m_pos;
		mp.time = time;
		mp.state = state;
		auto end_time = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end_time - start_time).count();
		mp.pingTime = static_cast<UINT>(duration); // 서버 핑 시간
		//여기다가 서버에서 패킷 처리하면서 걸린 시간을 넣어서 클라에게 넘겨주기

		vector <int> room_players;
		for (int i = 0; i < g_server.rooms[room_num].id.size(); ++i)
			room_players.emplace_back(g_server.rooms[room_num].getID(i));

		for (auto& id : room_players)
			g_server.users[id]->do_send(&mp);



		//BroadCasting_position(g_server.rooms[room_num].id.size());
		break;
	}
	}
}

void SESSION::BroadCasting_position(const int& size)
{

	for (int i = 0; i < size; ++i) {
		int num = g_server.rooms[room_num].getID(i);
		cout << "=== [객체" << num << "postion]" "== ";

		cout << "x: " << g_server.users[num]->m_pos._41 << "y: " 
			<< g_server.users[num]->m_pos._42 << "z: " 
			<< g_server.users[num]->m_pos._43 << endl;
	}

}



Network::Network() : rooms(10)
{

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



	std::cout << "[서버 초기화 완료]" << std::endl;
}






