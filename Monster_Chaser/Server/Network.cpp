#include "stdafx.h"
#include "Network.h"


Network::Network() : listenSocket(INVALID_SOCKET) {}

Network::~Network() {
    closesocket(listenSocket);
    WSACleanup();
}

bool Network::Run() {
    if (!InitializeSocket()) {
        cerr << "소켓 초기화 실패!" << endl;
        return false;
    }

    cout << "서버가 실행 중입니다..." << endl;
    AcceptClient(); // 클라이언트 수락
    return true;
}

// 소켓 초기화
bool Network::InitializeSocket() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup 실패" << endl;
        return false;
    }

    listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (listenSocket == INVALID_SOCKET) {
        cerr << "소켓 생성 실패" << endl;
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "바인딩 실패" << endl;
        return false;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "리스닝 실패" << endl;
        return false;
    }

    cout << "서버가 포트 " << SERVER_PORT << "에서 리스닝 중..." << endl;
    return true;
}

// 클라이언트 수락
void Network::AcceptClient() {
    sockaddr_in clientAddr;
    int addrSize = sizeof(sockaddr_in);

    SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrSize);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "클라이언트 연결 실패" << endl;
        return;
    }

    cout << "클라이언트 연결 성공! IP: " << inet_ntoa(clientAddr.sin_addr) << endl;

    // 클라이언트 세션 생성
    ClientSession* session = new ClientSession();
    session->socket = clientSocket;

    // 비동기 데이터 수신 시작

}

// 비동기 데이터 수신 (WSARecv 사용)
void Network::ReceiveData(ClientSession* session) {
    session->wsabuf.buf = session->buffer;
    session->wsabuf.len = BUF_SIZE;

    DWORD bytesReceived = 0, flags = 0;
    memset(&session->overlapped, 0, sizeof(OVERLAPPED));

    if (WSARecv(session->socket, &session->wsabuf, 1, &bytesReceived, &flags, &session->overlapped, NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            cerr << "WSARecv 실패!" << endl;
            closesocket(session->socket);
            delete session;
            return;
        }
    }

    // 수신 완료 후 송신 (에코)
    SendData(session);
}

// 비동기 데이터 송신 (WSASend 사용)
void Network::SendData(ClientSession* session) {
    DWORD bytesSent = 0;
    memset(&session->overlapped, 0, sizeof(OVERLAPPED));

    if (WSASend(session->socket, &session->wsabuf, 1, &bytesSent, 0, &session->overlapped, NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            cerr << "WSASend 실패!" << endl;
            closesocket(session->socket);
            delete session;
        }
    }
}
