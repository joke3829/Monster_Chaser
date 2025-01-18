#pragma once
#include "stdafx.h"

class CScene {
public:
	virtual void Render() {};
private:
	bool m_bRayTracing = false;

};

