#include "stdafx.h"
#include "Network.h"
#include <MSWSock.h>
std::vector<Room> Network::rooms;



EXP_OVER::EXP_OVER(IO_OP op) : io_op(op) {
    ZeroMemory(&over, sizeof(over));
    wsabuf[0].buf = buffer;
    wsabuf[0].len = BUF_SIZE;
    accept_socket = INVALID_SOCKET;
}

SESSION::SESSION(int i, SOCKET s) : id(i), socket(s) {
    recv_over = new EXP_OVER(IO_RECV);
    do_recv();
}

SESSION::~SESSION() {
    closesocket(socket);
    delete recv_over;
}

void SESSION::do_recv() {
    DWORD flags = 0;
    recv_over->wsabuf[0].buf = recv_over->buffer;
    recv_over->wsabuf[0].len = BUF_SIZE;
    WSARecv(socket, recv_over->wsabuf, 1, nullptr, &flags, &recv_over->over, nullptr);
}

void SESSION::do_send(void* packet) {
    EXP_OVER* over = new EXP_OVER(IO_SEND);
    int len = reinterpret_cast<unsigned char*>(packet)[0];
    memcpy(over->buffer, packet, len);
    over->wsabuf[0].len = len;
    WSASend(socket, over->wsabuf, 1, nullptr, 0, &over->over, nullptr);
}

void SESSION::process_packet(char* p) {
    char type = p[1];
    switch (type) {
    case C2S_P_ENTER_ROOM: {
        cs_packet_enter_room* pkt = (cs_packet_enter_room*)p;
        room_num = pkt->room_number;
        is_ready = false;
        Room& room = Network::rooms[room_num];
        room.AddPlayer(this);
        room.BroadcastRoomInfo();
        break;
    }
    case C2S_P_READY: {
        is_ready = true;
        Room& room = Network::rooms[room_num];
        room.BroadcastReady(id);
        break;
    }
    }
}

void Network::Init() {
    WSAData wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    listen_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    SOCKADDR_IN addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listen_socket, (sockaddr*)&addr, sizeof(addr));
    listen(listen_socket, SOMAXCONN);

    iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    CreateIoCompletionPort((HANDLE)listen_socket, iocp, 0, 0);

    for (int i = 0; i < MAX_ROOM; ++i)
        rooms.emplace_back(i);

    std::cout << "[서버 초기화 완료]" << std::endl;
}


