#pragma once
#include "stdafx.h"

constexpr int PORT_NUM = 3500;
constexpr int BUF_SIZE = 256;
constexpr int MAX_ROOM = 10;
constexpr int MAX_ROOM_MEMBER = 3;

constexpr char MAX_ID_LEN = 20;

#pragma pack(push, 1)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Server to Client packets
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr char S2C_P_ENTER = 1;
struct sc_packet_enter {
    unsigned char size;
    char type;
};

constexpr char S2C_P_CREATEUSER = 2;
struct sc_packet_createUser {
    unsigned char size;
    char type;
    int id;
    char Nickname[MAX_ID_LEN];
    bool loginSuccess = false;
};

constexpr char S2C_P_SELECT_ROOM = 3;
struct sc_packet_select_room {
    unsigned char size;
    char type;
    int Local_id;
    char room_number;
    bool is_self;
};

constexpr char S2C_P_ALLREADY = 4;
struct sc_packet_Ingame_start {
    unsigned char size;
    char type;
    int Local_id;
    char room_number;
};

constexpr char S2C_P_UPDATEROOM = 5;
struct sc_packet_room_info {
    unsigned char size;
    char type;
    short room_info[10];
};

constexpr char S2C_P_SETREADY = 6;
struct sc_packet_set_ready {
    unsigned char size;
    char type;
    int Local_id;
    char room_number;
    bool is_ready;
};

constexpr char S2C_P_MOVE = 7;
struct sc_packet_move {
    unsigned char size;
    char type;
    int Local_id;
    XMFLOAT4X4 pos;
    float time;
    UINT state;
    UINT pingTime;
};

constexpr char S2C_P_PICKCHARACTER = 8;
struct sc_packet_pickcharacter {
    unsigned char size;
    char type;
    int Local_id;
    short C_type;
};

constexpr char S2C_P_MONSTER_SPAWN = 10;
struct sc_packet_monster_spawn {
    unsigned char size;
    char type;
    int monster_id;
    int monster_type;
    XMFLOAT4X4 pos;
};

constexpr char S2C_P_MONSTER_HIT = 11;
struct sc_packet_monster_hit {
    unsigned char size;
    char type;
    int monster_id;
    int current_hp;
};

constexpr char S2C_P_MONSTER_DIE = 12;
struct sc_packet_monster_die {
    unsigned char size;
    char type;
    int monster_id;
};

constexpr char S2C_P_MONSTER_RESPAWN = 13;
struct sc_packet_monster_respawn {
    unsigned char size;
    char type;
    int monster_id;
    XMFLOAT4X4 pos;
};

constexpr char S2C_P_MONSTER_MOVE = 14;
struct sc_packet_monster_move {
    unsigned char size;
    char type;
    int monster_id;
    XMFLOAT4X4 pos;
};
constexpr char S2C_P_LEAVE = 49;
struct sc_packet_leave {
    unsigned char size;
    char type;
    int Local_id;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client to Server packets
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr char C2S_P_LOGIN = 51;
struct cs_packet_login {
    unsigned char size;
    char type;
    char UserID[MAX_ID_LEN];
    char Userpassword[MAX_ID_LEN];
};

constexpr char C2S_P_CREATEUSER = 52;
struct cs_packet_createuser {
    unsigned char size;
    char type;
    char UserID[MAX_ID_LEN];
    char Userpassword[MAX_ID_LEN];
    char UserNickName[MAX_ID_LEN];
};

constexpr char C2S_P_ENTER_ROOM = 53;
struct cs_packet_enter_room {
    unsigned char size;
    char type;
    char room_number;
};

constexpr char C2S_P_GETREADY = 54;
struct cs_packet_getready {
    unsigned char size;
    char type;
    char room_number;
    bool isReady;
};

constexpr char C2S_P_ROOM_UPDATE = 56;
struct cs_packet_room_refresh {
    unsigned char size;
    char type;
};

constexpr char C2S_P_MOVE = 57;
struct cs_packet_move {
    unsigned char size;
    char type;
    XMFLOAT4X4 pos;
    float time;
    UINT state;
};

constexpr char C2S_P_PICKCHARACTER = 58;
struct cs_packet_pickcharacter {
    unsigned char size;
    char type;
    char room_number;
    short C_type;
};

constexpr char C2S_P_PLAYERATTACK = 59;
struct cs_packet_player_attack {
    unsigned char size;
    char type;
    int target_monster_id;
};

#pragma pack(pop)
