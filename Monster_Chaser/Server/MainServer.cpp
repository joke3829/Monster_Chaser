
// MainServer.cpp
#include "stdafx.h"
#include "Room.h"
#include "Network.h"
#include <MSWSock.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")



void do_accept(Network& server) {
    SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
    EXP_OVER* accept_over = new EXP_OVER(IO_ACCEPT);
    accept_over->accept_socket = c_socket;

    AcceptEx(server.listen_socket, c_socket, accept_over->buffer, 0,
        sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
        NULL, &accept_over->over);
}

int main() {
    Network server;
    server.Init();

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes;
    

    do_accept(server);

    while (true) {
        DWORD bytes;
        ULONG_PTR key;
        WSAOVERLAPPED* over;

        BOOL success = GetQueuedCompletionStatus(server.iocp, &bytes, &key, &over, INFINITE);
        if (!success) continue;

        EXP_OVER* exp = reinterpret_cast<EXP_OVER*>(over);

        if (exp->io_op == IO_ACCEPT) {
            int client_id = server.next_client_id++;
            CreateIoCompletionPort((HANDLE)exp->accept_socket, server.iocp, client_id, 0);
            server.users[client_id] = new SESSION(client_id, exp->accept_socket);
            for (int i = 0; i < MAX_ROOM; ++i) {
                s2c_packet_room_info pkt;
                pkt.size = sizeof(pkt);
                pkt.type = S2C_P_ROOM_INFO;
                pkt.room_number = static_cast<char>(i);
                pkt.player_count = static_cast<char>(Network::rooms[i].GetPlayerCount());
                server.users[client_id]->do_send(&pkt);
            }
            std::cout << "[클라이언트 접속] ID: " << client_id << std::endl;
            do_accept(server);
            delete exp;
        }
        else if (exp->io_op == IO_RECV) {
            SESSION* user = server.users[key];
            char* ptr = exp->buffer;
            int processed = 0;
            while (processed < bytes) {
                int size = ptr[0];
                user->process_packet(ptr);
                ptr += size;
                processed += size;
            }
            user->do_recv();
        }
        else if (exp->io_op == IO_SEND) {
            delete exp;
        }
    }
    return 0;
}


