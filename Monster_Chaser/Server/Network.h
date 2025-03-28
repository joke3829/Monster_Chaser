#pragma once

#include "stdafx.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 9000
#define BUF_SIZE 1024

// ��Ŷ ����ü
struct CS_MOVE_PACKET {
    short size;
    short type;
    short id;
    char direction;
};

// Ŭ���̾�Ʈ ���� ����ü
struct ClientSession {
    SOCKET socket;
    OVERLAPPED overlapped;
    WSABUF wsabuf;
    char buffer[BUF_SIZE];
};

// ��Ʈ��ũ Ŭ���� (Overlapped I/O ���)
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
