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
#include <concurrent_unordered_map.h>
#include <MSWSock.h>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "d3d12.lib")

using namespace DirectX;
using namespace std;
using namespace concurrency;
using namespace chrono;

#define MAX_USER 5000
//enum 직업 {
//	전사 = 0,
//	마법사,
//	성직자
//};

struct vec3 {
	float x;
	float y;
	float z;
};
struct vec2 {
	float x;
	float y;
	
};