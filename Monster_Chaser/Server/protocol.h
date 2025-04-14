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

constexpr char C2S_P_LOGIN = 5;
constexpr char C2S_P_MOVE = 6;

constexpr char C2S_P_ENTER_ROOM = 10;
constexpr char C2S_P_READY = 11;
constexpr char S2C_P_ROOM_INFO = 12;
constexpr char S2C_P_READY_BROADCAST = 13;

#pragma pack(push, 1)
struct sc_packet_avatar_info {
	unsigned char size;
	char type;
	long long  id;
	short x, y;
	short hp;
	short level;
	int   exp;
};
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
};

#pragma pack(pop)
