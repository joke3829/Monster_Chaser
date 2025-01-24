#pragma once
#include "stdafx.h"

extern DXResources g_DxResource;

class CScene {
public:
	virtual void Render() {};
private:
	bool m_bRayTracing = false;

};

