#include "stdafx.h"

// �������� ���� �� ���ҽ�
DXResources g_DxResource{};

void Flush()
{
	static UINT64 nFenceValue = 1;
	g_DxResource.cmdQueue->Signal(g_DxResource.fence, nFenceValue);
	// ���� ������������ �۵� �� WaitForSingleObject�� �߰��Ѵ�.
	g_DxResource.fence->SetEventOnCompletion(nFenceValue++, *g_DxResource.pFenceHandle);
	::WaitForSingleObject(*g_DxResource.pFenceHandle, INFINITE);
}