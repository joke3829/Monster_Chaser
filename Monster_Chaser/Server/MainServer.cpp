
#include "stdafx.h"

#include <thread>
#include "Character.h"
#include "Network.h"

using namespace std;




int main() {
    Network server;
    if (!server.Run()) {
        cerr << "서버 실행 실패!" << endl;
        return -1;
    }

    // 서버는 멀티스레드로 실행 중이므로 무한 루프를 돌며 유지
    while (true) {
        Sleep(1000);
    }

    return 0;
}


