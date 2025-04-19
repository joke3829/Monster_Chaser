// Network.h
#pragma once



#include "protocol.h"
#include "Room.h"
#include "Character.h"
#include "Monster.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 3500
#define BUF_SIZE 1024


enum IO_OP { IO_RECV, IO_SEND, IO_ACCEPT };

class EXP_OVER {
public:
    EXP_OVER(IO_OP op);
    WSAOVERLAPPED over;
    IO_OP io_op;
    WSABUF wsabuf[1];
     char buffer[BUF_SIZE];
    SOCKET accept_socket;
};

class SESSION;


class SESSION {
public:
    SOCKET socket;
    EXP_OVER* recv_over;            
    int id;
    int room_num = -1;
    bool is_ready = false;
    unsigned char remained = 0;
    std::string name;               //유저 닉네임
    Job character;
    SESSION(int i, SOCKET s);
    ~SESSION();

    void do_recv();
    void do_send(void* buff);
    void process_packet( char* p);
    void BroadCasting_position(const XMFLOAT4X4& pos);
};

class Network {
public:
    void Init();
     Room rooms[10];

    HANDLE iocp;
    std::unordered_map<int, SESSION*> users;
    std::unordered_map<int, Monster*> monsters;
    int client_id = 0;
    int monster_id = 0;
    SOCKET listen_socket;
};