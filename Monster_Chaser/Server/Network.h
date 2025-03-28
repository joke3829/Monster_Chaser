#pragma once

#include "stdafx.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 9000
#define BUF_SIZE 1024

// 패킷 구조체
struct CS_MOVE_PACKET {
    short size;
    short type;
    short id;
    char direction;
};

// 클라이언트 세션 구조체
struct ClientSession {
    SOCKET socket;
    OVERLAPPED overlapped;
    WSABUF wsabuf;
    char buffer[BUF_SIZE];
};

// 네트워크 클래스 (Overlapped I/O 기반)
class Network {
private:
    SOCKET listenSocket;

public:
    Network();
    ~Network();
    bool Run();

private:
    bool InitializeSocket();
    void AcceptClient();
    void ReceiveData(ClientSession* session);
    void SendData(ClientSession* session);
};
