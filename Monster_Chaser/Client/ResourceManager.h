#pragma once

//#include "GameObject.h"
#include "AnimationManager.h"

extern DXResources g_DxResource;

class CResourceManager {
public:
	bool AddResourceFromFile(wchar_t* FilePath, std::string textureFilePathFront);
	bool AddSkinningResourceFromFile(wchar_t* FilePath, std::string textureFilePathFront);

	void AddGameObjectFromFile(std::ifstream& inFile, int nParentIndex = -1);
	void AddMaterialFromFile(std::ifstream& inFile, int nCurrentIndex);
	void InitializeGameObjectCBuffer();

	void PrepareObject();

	void UpdateSkinningMesh(float fElapsedTime);
	void ReBuildBLAS();
	void UpdateWorldMatrix();	// UpdateWorldMatrix

	// getter
	std::vector<std::unique_ptr<CGameObject>>& getGameObjectList();
	std::vector<std::unique_ptr<Mesh>>& getMeshList();
	std::vector<std::unique_ptr<CTexture>>& getTextureList();

	std::vector<std::unique_ptr<CSkinningObject>>& getSkinningObjectList() { return m_vSkinningObject; }
	std::vector<std::unique_ptr<CAnimationManager>>& getAnimationManagers() { return m_vAnimationManager; }
private:
	std::string FilePathFront{};

	std::vector<std::unique_ptr<CGameObject>> m_vGameObjectList;
	std::vector<std::unique_ptr<Mesh>> m_vMeshList;
	std::vector<std::unique_ptr<CTexture>> m_vTextureList;

	// Skinning animation
	std::vector<std::unique_ptr<CSkinningObject>> m_vSkinningObject;
	std::vector<std::unique_ptr<CAnimationManager>> m_vAnimationManager;
};

