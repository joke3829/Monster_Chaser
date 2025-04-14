// Network.h
#pragma once



#include "protocol.h"
#include "Room.h"

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
    int id;
    int room_num = -1;
    bool is_ready = false;
    EXP_OVER* recv_over;
    unsigned char remained = 0;
    short x = 0, y = 0;
    std::string name;

    SESSION(int i, SOCKET s);
    ~SESSION();

    void do_recv();
    void do_send(void* buff);
    void process_packet(char* p);
};

class Network {
public:
    void Init();

    static std::vector<Room> rooms;

    HANDLE iocp;
    std::unordered_map<int, SESSION*> users;
    int next_client_id = 1;
    SOCKET listen_socket;
};