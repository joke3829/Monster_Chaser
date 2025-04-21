#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <d3d12.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <WinSock2.h>
#include <thread>
#include <algorithm>
#include <DirectXMath.h>
#include <mutex>
#include <windows.h>


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "d3d12.lib")

using namespace DirectX;
using namespace std;

void MoveCursorTo(int x, int y) {
	COORD pos = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
enum 직업 {
	전사 = 0,
	마법사,
	성직자
};

struct vec3 {
	float x;
	float y;
	float z;
};
struct vec2 {
	float x;
	float y;
	
};