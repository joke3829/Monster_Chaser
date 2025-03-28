#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <WinSock2.h>
#include <thread>
enum 직업 {
	전사 = 0,
	마법사,
	성직자
};

using namespace std;

struct vec3 {
	float x;
	float y;
	float z;
};
struct vec2 {
	float x;
	float y;
	
};