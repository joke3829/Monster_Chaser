#pragma once


constexpr int PORT_NUM = 3500;
constexpr int BUF_SIZE = 256;
constexpr int NAME_SIZE = 20;
constexpr int W_WIDTH = 8;
constexpr int W_HEIGHT = 8;
constexpr int MAX_ROOM = 10;
constexpr int MAX_ROOM_MEMBER = 3;



constexpr char MAX_ID_LEN = 20;
//Packet 
constexpr char S2C_P_AVATAR_INFO = 1;
constexpr char S2C_P_MOVE = 2;
constexpr char S2C_P_ENTER = 3;
constexpr char S2C_P_LEAVE = 4;



constexpr char C2S_P_LOGIN = 51;
constexpr char C2S_P_MOVE = 52;

constexpr char C2S_P_ENTER_ROOM = 53;
constexpr char C2S_P_READY = 54;
constexpr char C2S_P_READY_Cancel = 55;
constexpr char S2C_P_ROOM_INFO = 56;
constexpr char S2C_P_READY_BROADCAST = 57;
constexpr char C2S_P_ROOM_REFRESH = 58;
#pragma pack(push, 1)
//struct sc_packet_avatar_info {
//	unsigned char size;
//	char type;
//	long long  id;
//	short x, y;
//	short hp;
//	short level;
//	int   exp;
//};
struct sc_packet_move {
	unsigned char size;
	char type;
	long long id;
	short x, y;
};
struct sc_packet_enter {
	unsigned char size;
	char type;
	long long  id;
	char name[MAX_ID_LEN];
	char o_type;
	short x, y;
};
struct sc_packet_leave {
	unsigned char size;
	char type; long long  id;
};
struct s2c_packet_room_info {
	unsigned char size;
	char type;
	char room_number;
	char player_count;
};

struct s2c_packet_ready_broadcast {
	unsigned char size;
	char type;
	int client_id;
	char room_number;
	bool is_ready;
};

struct cs_packet_login {
	unsigned char  size;
	char  type;
	char  name[MAX_ID_LEN];
};

struct cs_packet_move {
	unsigned char  size;
	char  type;
	char  direction;
};
struct cs_packet_enter_room {
	unsigned char size;
	char type;
	char room_number;
};

struct cs_packet_ready {
	unsigned char size;
	char type;
};
struct cs_packet_cancel_ready {
	unsigned char size;
	char type;
};
struct cs_packet_room_refresh {
	unsigned char size;
	char type;
};
#pragma pack(pop)
