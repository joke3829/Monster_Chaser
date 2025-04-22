
#include "C_Socket.h"

extern C_Socket Client;
extern std::unordered_map<int, Player> Players;
extern std::unordered_map<int, Monster*> g_monsters;
extern int RoomList[10];

extern std::vector<std::unique_ptr<CSkinningObject>>& skinned;
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

	case S2C_P_ENTER:			//����
	{
		sc_packet_enter* p = reinterpret_cast<sc_packet_enter*>(ptr);


		break;
	}
	case S2C_P_UPDATEROOM: {

		sc_packet_room_info* p = reinterpret_cast<sc_packet_room_info*>(ptr);
		int room_in_Players[10];
		memcpy(room_in_Players, p->room_info, sizeof(room_in_Players));

		memcpy(RoomList, room_in_Players, sizeof(RoomList));



		break;
	}
	case S2C_P_SELECT_ROOM: {

		sc_packet_select_room* p = reinterpret_cast<sc_packet_select_room*>(ptr);

		int room_num = static_cast<int>(p->room_number);
		int local_id = p->Local_id;
		if (!Players.contains(local_id)) {

			Players.try_emplace(local_id, local_id);
		Client.set_id(local_id);
			//Players[local_id] = new Player(local_id);
		}

		RoomList[room_num]++;
		//Players[id]->setRoomNumber(room_num);
		//Players[id]->room_players[room_num] = playersInRoom;	//���� �� �濡 ����ִ��� �˷��ֱ�


		break;
	}
	case S2C_P_SETREADY:
	{

		sc_packet_set_ready* p = reinterpret_cast<sc_packet_set_ready*>(ptr);

		int id = p->Local_id;
		int room_num = static_cast<int>(p->room_number);//�̹� �� �����ҋ� room_num�� Players[id]�ȿ� ��
		bool ready = p->is_ready;

		//Players[p->id]->setReady(ready);
		//Players[p->id]->setRoomNumber(room_num);// �� ���������� �ش� �� ���� �� ��Ÿ���� �ɹ� ���� 
		//p->id = id;

		break;
	}
	case S2C_P_ALLREADY:
	{
		
		sc_packet_Ingame_start* p = reinterpret_cast<sc_packet_Ingame_start*>(ptr);
		Client.set_id(p->Local_id);
		Setstart(true);		//�ɹ� ���� InGameStart true�� �ٲ��ֱ�
		
		break;
		//4 7 9
	}

	case S2C_P_MOVE: {
		sc_packet_move* p = reinterpret_cast<sc_packet_move*>(ptr);
		//����ID
		int local_id = p->Local_id;
		XMFLOAT4X4 position = p->pos;

		// 3���� �Ѳ�����	

		Players[local_id].setPosition(position);


		break;
	}
	default:
		break;
	}


}

void C_Socket::do_recv()
{
	std::lock_guard<std::mutex>lock(myMutex);

	char buffer[BUF_SIZE] = {};
	while (running) {
		int processed = 0;

		int io_byte = recv(m_socket, buffer + remained, BUF_SIZE - remained, 0);      //io_byte


		if (io_byte == 0) {
			MessageBoxA(nullptr, "IO_BYTE ũ�Ⱑ 0�Դϴ�.", "���� ����", MB_ICONERROR);

			break;
		}
		if (io_byte == SOCKET_ERROR) {
			int err = WSAGetLastError();

			MessageBoxA(NULL, "�������� ������ ���������ϴ�.", "���� ����", MB_OK | MB_ICONERROR);
			PostQuitMessage(0);  // ������ ���� ����
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




