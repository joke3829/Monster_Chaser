
#include "C_Socket.h"
#include "GameObject.h"
extern CSkinningObject* g_myPlayerObject;
extern std::unordered_map<int, Player*> Players;
extern std::unordered_map<int, Monster*> g_monsters;
extern int my_id;
C_Socket::C_Socket() : ready_to_start(false), running(true), remained(0), id(0), m_socket(INVALID_SOCKET) {}


C_Socket::~C_Socket()
{
	closesocket(m_socket);
}

bool C_Socket::Init(const char* ip, int port)
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    m_socket =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    if (connect(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        
        closesocket(m_socket);
        WSACleanup();
		return false;
    }
	id = my_id;
	my_id++;
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
		if (p->id == id) {
			id = p->id;
		}
		Players[p->id] = new Player(p->id); // 
		
		/*snprintf(p->name, sizeof(p->name), "클라 %d", p->id);*/
		break;
	}
	case S2C_P_SETREADY:
	{
		
		sc_packet_set_ready* p = reinterpret_cast<sc_packet_set_ready*>(ptr);
		p->
		//p->id = id;
			g_server.users[client_id] = new SESSION(client_id, exp->accept_socket);
			 
		break;
	}
	case S2C_P_MOVE: {
		sc_packet_move* p = reinterpret_cast<sc_packet_move*>(ptr);
	/*	XMFLOAT3 newPos = { static_cast<float>(p->x), static_cast<float>(p->y), static_cast<float>(p->z) };

		if (p->id == id && g_myPlayerObject) {
			g_myPlayerObject->SetPosition(newPos);
		}
		else if (g_otherPlayers.count(p->id)) {
			g_otherPlayers[p->id]->SetPosition(newPos);
		}
		else if (g_monsters.count(p->id)) {
			g_monsters[p->id]->SetPosition(newPos);
		}*/

		break;
	}
	case S2C_P_ROOM_INFO: {
		
		sc_packet_room_info* p = reinterpret_cast<sc_packet_room_info*>(ptr);
		
		room_players[p->room_number] = p->player_count;
		int ready_count = 0;
		for (int i = 0; i < p->player_count; ++i) {
			if (p->ready_states[i]) ++ready_count;
		}
		if (p->player_count == 3 && ready_count == 3)
			ready_to_start = true;
		DrawRoomList();
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

void C_Socket::DrawRoomList()
{

	std::lock_guard<std::mutex> lock(mtx);
	system("cls");
	std::cout << "=== [Room Status] ===\n";
	for (int i = 0; i < MAX_ROOM; ++i) {
		std::cout << i << "번 방: " << (int)room_players[i] << "/" << MAX_ROOM_MEMBER << std::endl;
	}
	std::cout << "=====================" << std::endl;
	std::cout << "'r' 키: Ready 전송 / 'q' 키: 종료" << std::endl;
}


