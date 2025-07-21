#pragma once

#define NOMINMAX

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




constexpr int NotStart = 0; // 게임 시작 전 상태
constexpr int Stage1 = 1;  // 스테이지1	
constexpr int Stage2 = 2;  // 스테이지2
constexpr int Stage3 = 2;  // 스테이지3

#define MAX_USER 5000


enum MoveAnimationState
{
	IDLE = 0,
	WALK_FORWARD = 5,
	WALK_LEFT_UP = 6,
	WALK_RIGHT_UP = 7,
	WALK_LEFT = 8,
	WALK_RIGHT = 9,
	WALK_BACKWARD = 10,
	WALK_LEFT_DOWN = 11,
	WALK_RIGHT_DOWN = 12,
	RUN_FORWARD = 13,
	RUN_LEFT_UP = 14,
	RUN_RIGHT_UP = 15,
	RUN_LEFT = 16,
	RUN_RIGHT = 17,
	RUN_BACKWARD = 18,
	RUN_LEFT_DOWN = 19,
	RUN_RIGHT_DOWN = 20,
};
enum Character:char {
	Wizard=0,
	Warrior,
	Priest
};


enum class MonsterType {
	None = 0,
	Feroptere,
	Pistiripere,
	RostrokarackLarvae,
	XenokarceBoss,
	Occisodonte,
	Limadon,
	Fulgurodonte,
	RostrokarckBoss,
	GorhorridBoss,
};