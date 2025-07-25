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
constexpr char S2C_P_CREATEUSER = 2;
constexpr char S2C_P_SELECT_ROOM = 3;
constexpr char S2C_P_ALLREADY = 4;
constexpr char S2C_P_UPDATEROOM = 5;
constexpr char S2C_P_SETREADY = 6;
constexpr char S2C_P_MOVE = 7;
constexpr char S2C_P_PICKCHARACTER = 8;
constexpr char S2C_P_INGAME_START = 9;
constexpr char S2C_P_MONSTER_SPAWN = 10;
constexpr char S2C_P_MONSTER_ATTACK = 11;  // ���Ͱ� �÷��̾� ������ ���·� �ٲٱ�   
constexpr char S2C_P_MONSTER_HIT = 12;  // ���Ͱ� �÷��̾� ������ ���·� �ٲٱ�   
constexpr char S2C_P_MONSTER_DIE = 13;
constexpr char S2C_P_MONSTER_RESPAWN = 14;
constexpr char S2C_P_MONSTER_MOVE = 15;
constexpr char S2C_P_PLAYER_HIT = 16;  // �÷��̾ �����ؼ� ���Ͱ� �¾��� ��
constexpr char S2C_P_NEXTSTAGE = 17;  //  �������� óġ �� ���� ���������� �Ѿ ��
constexpr char S2C_P_APPLY_HPITEM = 18;  // HP ������ ��� ��
constexpr char S2C_P_APPLY_MPITEM = 19;  // MP ������ ��� ��
constexpr char S2C_P_BOSS_ROAR = 20; // ���� ��Ŷ ��ȣ
constexpr char S2C_P_BUFFCHANGE = 21; // ���� ��Ŷ ��ȣ

constexpr char S2C_P_LEAVE = 49; // �÷��̾ ���� ���� ��
struct sc_packet_enter {
	unsigned char size;
	char type;
};

struct sc_packet_createUser {
	unsigned char size;
	char type;
	int id;
	char Nickname[MAX_ID_LEN];
	bool loginSuccess = false;
};

struct sc_packet_select_room {
	unsigned char size;
	char type;
	int Local_id;
	char room_number;
	bool is_self;
};

struct sc_packet_Ingame_start {
	unsigned char size;
	char type;
	int Local_id;
	char room_number;
};

struct sc_packet_room_info {
	unsigned char size;
	char type;
	short room_info[10];
};

struct sc_packet_set_ready {
	unsigned char size;
	char type;
	int Local_id;
	char room_number;
	bool is_ready;
};

struct sc_packet_move {
	unsigned char size;
	char type;
	int Local_id;
	XMFLOAT4X4 pos;
	float time;
	UINT state;
	UINT pingTime;
};

struct sc_packet_pickcharacter {
	unsigned char size;
	char type;
	int Local_id;
	float Max_HP;
	float Max_MP;
	short C_type;
};

struct sc_packet_ingame_start {
	unsigned char size;
	char type;
};

struct sc_packet_monster_spawn {
	unsigned char size;
	char type;
	int monster_id;
	MonsterType monster_type;
	XMFLOAT4X4 pos;
};

struct sc_packet_monster_attack  //���Ͱ� ���� ������ ��
{
	unsigned char size;
	char type;
	int monster_id;
	char attack_type; // Attacktype
};
struct sc_packet_monster_hit  //���Ͱ� ���� ������ ��
{
	unsigned char size;
	char type;
	int monster_id;
	float hp;
};
struct sc_packet_monster_die  //���Ͱ� �׾�����
{
	unsigned char size;
	char type;
	int monster_id;
	int gold;
};

struct sc_packet_monster_respawn {
	unsigned char size;
	char type;
	int monster_id;
	XMFLOAT4X4 pos;
};

struct sc_packet_monster_move {
	unsigned char size;
	char type;
	int monster_id;
	XMFLOAT4X4 pos;
	int state;  // MonsterState (Idle, Chase, Attack, Return, Dead)

};

struct sc_packet_player_hit {
	unsigned char size;
	char type;
	int local_id;
	float hp;
};

struct sc_packet_NextStage {
	unsigned char size;
	char type;
};

struct sc_packet_leave {
	unsigned char size;
	char type;
	int Local_id;
};

struct sc_packet_apply_hpitem {
	unsigned char size;
	char type;
	float hp;
	char local_id;
};
struct sc_packet_apply_mpitem {
	unsigned char size;
	char type;
	float mp;
	char local_id;
};
struct sc_packet_apply_atkitem {
	unsigned char size;
	char type;
	float attack;
	char local_id;
};
struct sc_packet_apply_defitem {
	unsigned char size;
	char type;
	float defense;
	char local_id;
};
struct sc_packet_boss_roar {
	unsigned char size;
	char type;
	int monster_id;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client to Server packets
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
constexpr char C2S_P_LOGIN = 51;
constexpr char C2S_P_CREATEUSER = 52;
constexpr char C2S_P_ENTER_ROOM = 53;
constexpr char C2S_P_ROOM_UPDATE = 54;
constexpr char C2S_P_PICKCHARACTER = 55;
constexpr char C2S_P_GETREADY = 56;
constexpr char C2S_P_READYINGAME = 57;
constexpr char C2S_P_MOVE = 58;
constexpr char C2S_P_PLAYERATTACK = 59;
constexpr char C2S_P_MONSTER_HIT = 60;
constexpr char C2S_P_USE_ITEM = 61;
constexpr char C2S_P_USE_SKILL = 62;

struct cs_packet_login {
	unsigned char size;
	char type;
	char UserID[MAX_ID_LEN];
	char Userpassword[MAX_ID_LEN];
};

struct cs_packet_createuser {
	unsigned char size;
	char type;
	char UserID[MAX_ID_LEN];
	char Userpassword[MAX_ID_LEN];
	char UserNickName[MAX_ID_LEN];
};

struct cs_packet_enter_room {
	unsigned char size;
	char type;
	char room_number;
};
struct cs_packet_room_refresh
{
	unsigned char size;
	char type;
};

struct cs_packet_pickcharacter {
	unsigned char size;
	char type;
	char room_number;
	short C_type;
};
struct cs_packet_getready {
	unsigned char size;
	char type;
	char room_number;
	bool isReady;
};

struct cs_packet_readytoIngame {
	unsigned char size;
	char type;
	//int local_id; // ���� ID
	//int room_number; // �� ��ȣ
};

struct cs_packet_move {
	unsigned char size;
	char type;
	XMFLOAT4X4 pos;
	float time;
	UINT state;
};


struct cs_packet_player_attack_monster {
	unsigned char size;
	char type;
	int target_monster_id;
	float AtkCalculation; // ���ݷ� ���
	int attack_type; // Attacktype
};

struct cs_packet_monster_hit {
	unsigned char size;
	char type;
	int attacker_id;  // ���� ID
	int target_player_id;
	int attack_type; // Attacktype
	float attack_power; // ���ݷ�
};

struct cs_packet_item_use {
	unsigned char size;
	char type;
	char item_type;   // enum ItemType
};

struct cs_packet_skill_use {
	unsigned char size;
	char type;
	char skillNumber;	// 0 ~ 2
};
#pragma pack(pop)
