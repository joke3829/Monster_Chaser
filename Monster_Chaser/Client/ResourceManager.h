#pragma once

#include "Mesh.h"
#include "GameObject.h"

extern DXResources g_DxResource;

class CResourceManager {
public:
	bool AddResourceFromFile(wchar_t* FilePath);

	void AddGameObjectFromFile(std::ifstream& inFile, int nParentIndex = -1);
	void AddMaterialFromFile(std::ifstream& inFile, int nCurrentIndex);
	// getter
private:
	std::vector<std::unique_ptr<CGameObject>> m_vGameObjectList;
	std::vector<std::unique_ptr<Mesh>> m_vMeshList;
	std::vector<std::unique_ptr<CTexture>> m_vTextureList;
};

