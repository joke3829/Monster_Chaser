﻿// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
// Windows 헤더 파일
#include <windows.h>
// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// 필요한 헤더 및 라이브러리를 여기에 추가한다 =================================================

#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <string>
#include <fstream>
#include <timeapi.h>
#include <random>
#include <numeric>
#include <cmath>
#include <format>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <d3dcompiler.h>

#include <DirectXMath.h>
#include <DirectXCollision.h>

#include <d2d1_3.h>
#include <d3d11on12.h>
#include <dwrite.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Winmm.lib")

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// =============================================================================================

// enumerate ============================================================================
enum MaterialIndex {	// 사용할지 고민중, 안쓰는게 더 편할지도...?
	ALBEDO_COLOR, EMISSIVE_COLOR, SPECULAR_COLOR, GLOSSINESS
};
//========================================================================================

// 상수 정의 ===========================================================================

// 바운딩 박스를 렌더링 여부-> 0 = false , 1 = true
constexpr short g_ShowBoundingBox = 0;

constexpr unsigned short DEFINED_GAME_WINDOW_WIDTH = 1920;
constexpr unsigned short DEFINED_GAME_WINDOW_HEIGHT = 1080;
constexpr unsigned short DEFINED_UAV_BUFFER_WIDTH = 1280;
constexpr unsigned short DEFINED_UAV_BUFFER_HEIGHT = 720;
// 위 네개 정의는 바뀔 수 있다.

// define Scene
constexpr unsigned short SCENE_TITLE = 0;
constexpr unsigned short SCENE_PLAIN = 1;
constexpr unsigned short SCENE_CAVE = 2;
constexpr unsigned short SCENE_WINTERLAND = 3;

// define job
constexpr unsigned short JOB_NOTHING = 0;
constexpr unsigned short JOB_MAGE = 1;
constexpr unsigned short JOB_WARRIOR = 2;
constexpr unsigned short JOB_HEALER = 3;
constexpr unsigned short MONSTER = 4;

// 조명 관련 정의
constexpr int MAX_LIGHTS = 64;

constexpr unsigned int DIRECTIONAL_LIGHT = 0;
constexpr unsigned int POINT_LIGHT = 1;
constexpr unsigned int SPOT_LIGHT = 2;

// 자주 쓰이는 설정들을 미리 지정
constexpr DXGI_SAMPLE_DESC NO_AA = { .Count = 1, .Quality = 0 };	// no anti_aliasing
constexpr D3D12_HEAP_PROPERTIES UPLOAD_HEAP = { .Type = D3D12_HEAP_TYPE_UPLOAD };
constexpr D3D12_HEAP_PROPERTIES DEFAULT_HEAP = { .Type = D3D12_HEAP_TYPE_DEFAULT };
constexpr D3D12_HEAP_PROPERTIES READBACK_HEAP = { .Type = D3D12_HEAP_TYPE_READBACK };
constexpr D3D12_RESOURCE_DESC BASIC_BUFFER_DESC = {
	.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
	.Width = 0,
	.Height = 1,
	.DepthOrArraySize = 1,
	.MipLevels = 1,
	.SampleDesc = NO_AA,
	.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR
};

// DirectX에 필요한 리소스들을 담는 구조체
struct DXResources {
	ID3D12Device5* device{ nullptr };
	ID3D12CommandAllocator* cmdAlloc{ nullptr };
	ID3D12GraphicsCommandList4* cmdList{ nullptr };
	ID3D12CommandQueue* cmdQueue{ nullptr };
	ID3D12Fence* fence{ nullptr };
	HANDLE* pFenceHandle{ nullptr };

	ComPtr<ID3D12Resource> nullBuffer{};
	ComPtr<ID3D12Resource> nullTexture{};
};

// GPU 작업이 끝나기까지 기다린다.
void Flush();

// size를 alignment의 배수로 할당
inline constexpr UINT Align(UINT size, UINT alignment)
{
	return (size + (alignment - 1)) & ~(alignment - 1);
}