// Network.h
#pragma once


#include "stdafx.h"
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




class SESSION {
public:
    SOCKET socket;
    std::unique_ptr<EXP_OVER> recv_over;
    int m_uniqueNo;
    XMFLOAT4X4 m_pos;
    unsigned char remained = 0;

    std::string name;               //user name
    int local_id;
    int room_num;

    SESSION(int Num,SOCKET s);
    ~SESSION();

    void do_recv();
    void do_send(void* buff);
    void process_packet( char* p);
    void BroadCasting_position(const int& size);
};

class Network {
public:
    Network();
    void Init();
    std::vector<Room> rooms;

    HANDLE iocp;
   // std::unordered_map<int, std::unique_ptr<SESSION>> users;
    //concurrent_unordered_map <int, std::unique_ptr<SESSION>> users;
    concurrent_unordered_map <int,shared_ptr <SESSION>> users;
    
    

    std::unordered_map<int, Monster*> monsters;


   SOCKET listen_socket;
};