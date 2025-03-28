#include "stdafx.h"
#include "Network.h"


Network::Network() : listenSocket(INVALID_SOCKET) {}

Network::~Network() {
    closesocket(listenSocket);
    WSACleanup();
}

bool Network::Run() {
    if (!InitializeSocket()) {
        cerr << "���� �ʱ�ȭ ����!" << endl;
        return false;
    }

    cout << "������ ���� ���Դϴ�..." << endl;
    AcceptClient(); // Ŭ���̾�Ʈ ����
    return true;
}

// ���� �ʱ�ȭ
bool Network::InitializeSocket() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup ����" << endl;
        return false;
    }

    listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "���� ���� ����" << endl;
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "���ε� ����" << endl;
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "������ ����" << endl;
        return false;
    }

    cout << "������ ��Ʈ " << SERVER_PORT << "���� ������ ��..." << endl;
    return true;
}

// Ŭ���̾�Ʈ ����
void Network::AcceptClient() {
    sockaddr_in clientAddr;
    int addrSize = sizeof(sockaddr_in);

    SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrSize);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Ŭ���̾�Ʈ ���� ����" << endl;
        return;
    }

    cout << "Ŭ���̾�Ʈ ���� ����! IP: " << inet_ntoa(clientAddr.sin_addr) << endl;

    // Ŭ���̾�Ʈ ���� ����
    ClientSession* session = new ClientSession();
    session->socket = clientSocket;

    // �񵿱� ������ ���� ����

}

// �񵿱� ������ ���� (WSARecv ���)
void Network::ReceiveData(ClientSession* session) {
    session->wsabuf.buf = session->buffer;
    session->wsabuf.len = BUF_SIZE;

    DWORD bytesReceived = 0, flags = 0;
    memset(&session->overlapped, 0, sizeof(OVERLAPPED));

    if (WSARecv(session->socket, &session->wsabuf, 1, &bytesReceived, &flags, &session->overlapped, NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            cerr << "WSARecv ����!" << endl;
            closesocket(session->socket);
            delete session;
            return;
        }
    }

    // ���� �Ϸ� �� �۽� (����)
    SendData(session);
}

// �񵿱� ������ �۽� (WSASend ���)
void Network::SendData(ClientSession* session) {
    DWORD bytesSent = 0;
    memset(&session->overlapped, 0, sizeof(OVERLAPPED));

    if (WSASend(session->socket, &session->wsabuf, 1, &bytesSent, 0, &session->overlapped, NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            cerr << "WSASend ����!" << endl;
            closesocket(session->socket);
            delete session;
        }
    }
}
