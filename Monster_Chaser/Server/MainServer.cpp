
#include "stdafx.h"

#include <thread>
#include "Character.h"
#include "Network.h"

using namespace std;




int main() {
    Network server;
    if (!server.Run()) {
        cerr << "���� ���� ����!" << endl;
        return -1;
    }

    // ������ ��Ƽ������� ���� ���̹Ƿ� ���� ������ ���� ����
    while (true) {
        Sleep(1000);
    }

    return 0;
}


