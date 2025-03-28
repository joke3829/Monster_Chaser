constexpr int PORT_NUM = 3500;
constexpr int BUF_SIZE = 256; 
constexpr int NAME_SIZE = 20;

constexpr int W_WIDTH = 8;
constexpr int W_HEIGHT = 8;

// Packet ID


// 패킷 타입 정의
enum PACKET_TYPE {
	CS_LOGIN = 0,
	CS_MOVE = 1,   // 클라이언트 → 서버 : 이동 요청
	SC_UPDATE = 2  // 서버 → 클라이언트 : 전체 플레이어 위치 업데이트
};

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_MOVE_PLAYER = 4;





#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	char	name[NAME_SIZE];
};
#pragma pack(pop)

// 서버 → 클라이언트 : 전체 플레이어 위치 패킷
#pragma pack(push, 1)
struct SC_UPDATE_PACKET {
	short size;
	short type;  // SC_UPDATE
	short playerCount; // 현재 접속 중인 플레이어 수
	struct PlayerData {
		short id;
		int x;
		int y;
	} players[10];  // 최대 10명까지 동시 접속 가능 (필요시 확장)
};
#pragma pack(pop)
struct CS_MOVE_PACKET { 
	short size;
	short type;  // CS_MOVE
	short id;    // 클라이언트 ID
	char direction;  // 'W', 'A', 'S', 'D'
};
struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
	char	name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET {		
	unsigned char size;
	char	type;
	short	id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	short	id;
	short	x, y;
};

#pragma pack (pop)