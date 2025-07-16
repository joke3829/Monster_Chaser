
#include "C_Socket.h"

extern C_Socket Client;
extern std::unordered_map<int, Player> Players;
extern std::unordered_map<int, std::unique_ptr<Monster>> Monsters;
extern std::array<short, 10>	 userPerRoom;
extern std::vector<std::unique_ptr<CSkinningObject>>& skinned;
extern bool allready;
extern TitleState g_state;
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
	if (ret == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK) {
			std::cerr << "send 블로킹 발생!" << std::endl;
		}
	}
}

void C_Socket::SendLogin(const char* UserID, const char* Userpassword)
{
}

void C_Socket::SendCreateUser(const char* UserID, const char* Userpassword, const char* userNickName)
{
}

void C_Socket::SendEnterRoom(const short RoomNum)
{
	cs_packet_enter_room p;
	p.size = sizeof(p);
	p.type = C2S_P_ENTER_ROOM;
	p.room_number = static_cast<char>(RoomNum);
	Client.send_packet(&p);
}

void C_Socket::SendPickCharacter(const short RoomNum, const short Job)
{
	cs_packet_pickcharacter p;
	p.size = sizeof(p);
	p.type = C2S_P_PICKCHARACTER;
	p.room_number = static_cast<char>(RoomNum);
	p.C_type = Job;
	Client.send_packet(&p);
}

void C_Socket::SendsetReady(const bool isReady, const int room_num)
{
	cs_packet_getready rp;
	rp.size = sizeof(rp);
	rp.type = C2S_P_GETREADY;
	rp.room_number = room_num;
	rp.isReady = isReady;
	Client.send_packet(&rp);
}

void C_Socket::SendBroadCastRoom()
{
	cs_packet_room_refresh rp;
	rp.size = sizeof(rp);
	rp.type = C2S_P_ROOM_UPDATE;
	Client.send_packet(&rp);
}

void C_Socket::SendMovePacket(const float& Time, const UINT State)
{
	cs_packet_move mp;
	mp.size = sizeof(mp);
	mp.type = C2S_P_MOVE;
	mp.pos = Players[Client.get_id()].getRenderingObject()->getWorldMatrix();
	mp.time = Time;
	mp.state = State;
	Client.send_packet(&mp);

}





void C_Socket::process_packet(char* ptr)
{
	char type = ptr[1];
	switch (type)
	{

	case S2C_P_ENTER:			//입장
	{
		sc_packet_enter* p = reinterpret_cast<sc_packet_enter*>(ptr);


		break;
	}
	case S2C_P_UPDATEROOM: {

		sc_packet_room_info* p = reinterpret_cast<sc_packet_room_info*>(ptr);
		short room_in_Players[10];
		memcpy(room_in_Players, p->room_info, sizeof(room_in_Players));
		//memcpy(userperroom, p->room_info, sizeof(userperroom));		이것만 해주면 끝

		memcpy(userPerRoom.data(), room_in_Players, sizeof(room_in_Players));



		break;
	}
	case S2C_P_SELECT_ROOM: {

		sc_packet_select_room* p = reinterpret_cast<sc_packet_select_room*>(ptr);

		int room_num = static_cast<int>(p->room_number);
		int local_id = p->Local_id;
		if (!Players.contains(local_id)) {
			Player newPlayer(local_id); // 명시적 생성자 사용
			Players.emplace(local_id, std::move(newPlayer));

			//  is_self가 true일 때만 내 로컬 ID 설정
			if (Client.get_id() == -1) {
				if (p->is_self) {
					Client.set_id(local_id);
					g_state = InRoom;
				}

			}

		}

		break;
	}
	case S2C_P_PICKCHARACTER:
	{
		sc_packet_pickcharacter* p = reinterpret_cast<sc_packet_pickcharacter*>(ptr);
		short CT = p->C_type;
		int loacl_id = p->Local_id;
		Players[loacl_id].setCharacterType(CT);
		break;
	}
	case S2C_P_SETREADY:
	{

		sc_packet_set_ready* p = reinterpret_cast<sc_packet_set_ready*>(ptr);

		int id = p->Local_id;
		int room_num = static_cast<int>(p->room_number);//이미 방 선택할떄 room_num이 Players[id]안에 들어감
		bool ready = p->is_ready;

		Players[id].setReady(ready);
		//Players[p->id]->setReady(ready);
		//Players[p->id]->setRoomNumber(room_num);// 방 선택했을떄 해당 방 유저 수 나타내는 맴버 변수 
		//p->id = id;

		break;
	}
	case S2C_P_ALLREADY:
	{

		sc_packet_Ingame_start* p = reinterpret_cast<sc_packet_Ingame_start*>(ptr);
		Setstart(true);		//맴버 변수 InGameStart true로 바꿔주기
		g_state = GoLoading;

		break;
		//4 7 9
	}

	case S2C_P_MOVE: {
		sc_packet_move* p = reinterpret_cast<sc_packet_move*>(ptr);
		//로컬ID
		int local_id = p->Local_id;
		float time = p->time;
		unsigned int state = p->state;
		if (local_id == Client.get_id()) {
			return;
		}
		XMFLOAT4X4 position = p->pos;

		// write down to position bogan process~

		Players[local_id].getRenderingObject()->SetWolrdMatrix(position);
		Players[local_id].getAnimationManager()->ChangeAnimation(state, true);
		Players[local_id].getAnimationManager()->UpdateAnimation(time);

		//------------------


		break;
	}
	case S2C_P_MONSTER_SPAWN: {
		sc_packet_monster_spawn* pkt = reinterpret_cast<sc_packet_monster_spawn*>(ptr);

		int id = pkt->monster_id;

		// 이미 있으면 덮어쓰기 방지
		if (Monsters.find(id) == Monsters.end()) {
			auto newMonster = std::make_unique<Monster>(id);
			newMonster->setPosition(pkt->pos);
			newMonster->setVisible(true);
			newMonster->playIdleAnim();

			Monsters[id] = std::move(newMonster);

			
		}
		else {
			
		}

		break;
	}
	case S2C_P_MONSTER_RESPAWN: {
		sc_packet_monster_respawn* pkt = reinterpret_cast<sc_packet_monster_respawn*>(ptr);

		int id = pkt->monster_id;


		if (Monsters.find(id) != Monsters.end()) {
			auto& m = Monsters[id]; // Use auto& to correctly reference the unique_ptr  
			m->setPosition(pkt->pos);
			m->setVisible(true);
			m->playIdleAnim();
		}
		else {
			// 존재하지 않는 경우 새로 생성  
			auto newMonster = std::make_unique<Monster>(id);
			newMonster->setPosition(pkt->pos);
			newMonster->setVisible(true);
			newMonster->playIdleAnim();
			Monsters[id] = std::move(newMonster);
		}
		break;
	}

	case S2C_P_LEAVE:
	{
		sc_packet_leave* pkt = reinterpret_cast<sc_packet_leave*>(ptr);

		int local_id = pkt->Local_id;

		if (Players.find(local_id) != Players.end()) {
			Players.erase(local_id);
			if (local_id == Client.get_id()) {
				Client.set_id(-1); // 클라이언트 ID 초기화
				g_state = Title; // 타이틀 상태로 변경
			}
		}
		break;
	}
	default:
		break;
	}


}

void C_Socket::do_recv()
{
	//std::lock_guard<std::mutex>lock(myMutex);

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
			MessageBoxW(NULL, L"서버와의 연결이 끊어졌습니다.", L"연결 종료", MB_OK | MB_ICONERROR);
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




