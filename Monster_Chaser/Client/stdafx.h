// header.h: 표준 시스템 포함 파일
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

#include <vector>
#include <algorithm>
#include <string>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <DirectXMath.h>
#include <DirectXCollision.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// =============================================================================================

// 상수 정의 ===========================================================================

#define DEFINED_GAME_WINDOW_WIDTH 1920	
#define DEFINED_GAME_WINDOW_HEIGHT 1080
#define DEFINED_UAV_BUFFER_WIDTH 960
#define DEFINED_UAV_BUFFER_HEIGHT 540
// 위 네개 정의는 바뀔 수 있다.

// 자주 쓰이는 설정들을 미리 지정
constexpr DXGI_SAMPLE_DESC NO_AA = { .Count = 1, .Quality = 0 };	// no anti_aliasing
constexpr D3D12_HEAP_PROPERTIES UPLOAD_HEAP = { .Type = D3D12_HEAP_TYPE_UPLOAD };
constexpr D3D12_HEAP_PROPERTIES DEFAULT_HEAP = { .Type = D3D12_HEAP_TYPE_DEFAULT };
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
};

static DXResources g_DxResource{};

// GPU 작업이 끝나기까지 기다린다.
inline void Flush()
{
	static UINT64 nFenceValue = 1;
	g_DxResource.cmdQueue->Signal(g_DxResource.fence, nFenceValue);
	g_DxResource.fence->SetEventOnCompletion(nFenceValue++, nullptr);	
	// 만약 비정상적으로 작동 시 WaitForSingleObject를 추가한다.
}

// size를 alignment의 배수로 할당
inline constexpr UINT Align(UINT size, UINT alignment)
{
	return (size + (alignment - 1)) & ~(alignment - 1);
}