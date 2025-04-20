
#include "C_Socket.h"
#include "GameObject.h"
extern C_Socket Client;
extern std::unordered_map<int, Player*> Players;
extern std::unordered_map<int, Monster*> g_monsters;
extern int RoomList[10];
C_Socket::C_Socket() : InGameStart(false), running(true), remained(0), m_socket(INVALID_SOCKET) {}


C_Socket::~C_Socket()
{
	closesocket(m_socket);
}

bool C_Socket::Init(const char* ip, int port)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &serverAddr.sin_addr);

	if (connect(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {

		closesocket(m_socket);
		WSACleanup();
		return false;
	}
	

	return true;
}


void C_Socket::send_packet(void* pkt)
{
	int ret = send(m_socket, reinterpret_cast<CHAR*>(pkt), reinterpret_cast<unsigned char*>(pkt)[0], 0);
}

void C_Socket::process_packet(char* ptr)
{
	char type = ptr[1];
	switch (type)
	{

	case S2C_P_ENTER:			//입장
	{
		sc_packet_enter* p = reinterpret_cast<sc_packet_enter*>(ptr);
		
		int id = p->id;
		if (!Players.contains(p->id)) {
			
			Players[p->id] = new Player(p->id);
		}
		if (Client.get_id() == -1) {
			Client.set_id(p->id);  // 내 ID 설정은 처음 받은 패킷에서만
			//std::cout << "[내 클라이언트 ID 설정됨] " << p->id << std::endl;
		}
		
		
		break;
	}
	case S2C_P_SELECT_ROOM: {

		sc_packet_select_room* p = reinterpret_cast<sc_packet_select_room*>(ptr);
		int id = p->id;
		int room_num = static_cast<int>(p->room_number);
		int playersInRoom = static_cast<int>(p->players_inRoom);
		
		RoomList[room_num] = playersInRoom;
		//Players[id]->setRoomNumber(room_num);
		//Players[id]->room_players[room_num] = playersInRoom;	//현재 그 방에 몇명있는지 알려주기
		
		
		break;
	}
	case S2C_P_SETREADY:
	{

		sc_packet_set_ready* p = reinterpret_cast<sc_packet_set_ready*>(ptr);
		int id = p->id;
		int room_num = static_cast<int>(p->room_number);//이미 방 선택할떄 room_num이 Players[id]안에 들어감
		bool ready = p->is_ready;
		
		Players[p->id]->setReady(ready);
		Players[p->id]->setRoomNumber(room_num);// 방 선택했을떄 해당 방 유저 수 나타내는 맴버 변수 
		//p->id = id;
		
		break;
	}
	case S2C_P_ALLREADY:
	{
		sc_packet_Ingame_start* p = reinterpret_cast<sc_packet_Ingame_start*>(ptr);
		Players[p->ready_id[0]]->setPlayerID_In_Game(p->ready_id[0], 0);					 //방에 있는 id 넣어주기 0번째 인덱스는 자기 id
		Players[p->ready_id[0]]->setPlayerID_In_Game(p->ready_id[1], 1);					 //방에 있는 id 넣어주기 0번째 인덱스는 자기 id
		Players[p->ready_id[0]]->setPlayerID_In_Game(p->ready_id[2], 2);					 //방에 있는 id 넣어주기 0번째 인덱스는 자기 id
		Setstart(true);		//맴버 변수 InGameStart true로 바꿔주기

		break;
	}

	case S2C_P_MOVE: {
		sc_packet_move* p = reinterpret_cast<sc_packet_move*>(ptr);
		int id = p->id;
		XMFLOAT4X4 position = p->pos;
		
		
		if (Players.contains(id))
		{
			Players[id]->setPosition(position);
		}

		break;
	}
	case S2C_P_UPDATEROOM: {

		sc_packet_room_info* p = reinterpret_cast<sc_packet_room_info*>(ptr);
		int room_in_Players = static_cast<int>(p->player_count);
		int roomNum = static_cast<int>(p->room_number);
		
		RoomList[roomNum] = room_in_Players;
		/*for (auto& p : Players) {
			p.second->room_players[roomNum] = room_in_Players;
		}*/

		
		break;
	}
	default:
		break;
	}


}

void C_Socket::do_recv()
{
	char buffer[BUF_SIZE] = {};
	while (running) {
		int processed = 0;

		int io_byte = recv(m_socket, buffer + remained, BUF_SIZE - remained, 0);      //io_byte


		if (io_byte == 0) {
			MessageBoxA(nullptr, "IO_BYTE 크기가 0입니다.", "수신 에러", MB_ICONERROR);

			break;
		}
		if (io_byte == SOCKET_ERROR) {
			int err = WSAGetLastError();
			
				MessageBoxA(NULL, "서버와의 연결이 끊어졌습니다.", "연결 종료", MB_OK | MB_ICONERROR);
				PostQuitMessage(0);  // 윈도우 루프 종료
				return;
			
		}
		char* ptr = buffer;
		io_byte += remained;
		while (processed < io_byte) {
			int size = ptr[0];
			if (size > io_byte - processed)
				break;


			process_packet(ptr);

			ptr += size;
			processed += size;
		}
		remained = io_byte - processed;

		if (remained > 0) {
			memcpy(buffer, ptr, remained);
		}
	}
}




