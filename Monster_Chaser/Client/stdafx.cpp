#include "stdafx.h"

// 공용으로 돌려 쓸 리소스
DXResources g_DxResource{};

void Flush()
{
	static UINT64 nFenceValue = 1;
	g_DxResource.cmdQueue->Signal(g_DxResource.fence, nFenceValue);
	// 만약 비정상적으로 작동 시 WaitForSingleObject를 추가한다.
	g_DxResource.fence->SetEventOnCompletion(nFenceValue++, *g_DxResource.pFenceHandle);
	::WaitForSingleObject(*g_DxResource.pFenceHandle, INFINITE);
}