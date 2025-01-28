#pragma once

#include "GameObject.h"

extern DXResources g_DxResource;

class CResourceManager {
public:
	bool AddResourceFromFile(wchar_t* FilePath);

	void AddGameObjectFromFile(std::ifstream& inFile, int nParentIndex = -1);
	void AddMaterialFromFile(std::ifstream& inFile, int nCurrentIndex);
	void InitializeGameObjectCBuffer();

	// getter
	std::vector<std::unique_ptr<CGameObject>>& getGameObjectList();
	std::vector<std::unique_ptr<Mesh>>& getMeshList();
	std::vector<std::unique_ptr<CTexture>>& getTextureList();
private:
	std::vector<std::unique_ptr<CGameObject>> m_vGameObjectList;
	std::vector<std::unique_ptr<Mesh>> m_vMeshList;
	std::vector<std::unique_ptr<CTexture>> m_vTextureList;
};

