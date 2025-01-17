#pragma once

#include "stdafx.h"

extern DXResources g_DxResource;

class CCamera {
public:
	
protected:
private:
	XMFLOAT4X4 m_xmf4x4View;
	XMFLOAT4X4 m_xmf4x4Proj;
};

