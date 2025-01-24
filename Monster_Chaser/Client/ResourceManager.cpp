#include "ResourceManager.h"

bool CResourceManager::AddResourceFromFile(wchar_t* FilePath)
{
	std::ifstream inFile{ FilePath, std::ios::binary };
	if (!inFile) {
		OutputDebugString(L"Can't File Open!\n");
		return false;
	}
	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};

	while (1) {
		readLabel();
		if (strLabel == "</Hierarchy>:")
			break;
		else if (strLabel == "<Frame>:")
			AddGameObjectFromFile(inFile);
	}
	return true;
}

void CResourceManager::AddGameObjectFromFile(std::ifstream& inFile, int nParentIndex)
{
	UINT nCurrentObjectIndex = m_vGameObjectList.size();
	m_vGameObjectList.push_back(std::make_unique<CGameObject>());
	m_vGameObjectList[nCurrentObjectIndex]->InitializeObjectFromFile(inFile);

	if (nParentIndex != -1) {		// 부모가 존재한다는 뜻
		m_vGameObjectList[nCurrentObjectIndex]->SetParentIndex(nParentIndex);
	}

	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};
	int tempData{};
	while (1) {
		readLabel();
		if (strLabel == "<Mesh>:") {
			inFile.read((char*)&tempData, sizeof(int));
			readLabel();	// 메시의 이름 읽기
			auto p = std::find_if(m_vMeshList.begin(), m_vMeshList.end(), [&](std::unique_ptr<Mesh>& tempMesh) {
				return tempMesh->getName() == strLabel;
				});
			if (p != m_vMeshList.end()) {	// 이미 리스트에 해당 이름을 가진 메시가 존재
				// 주의할게 이름이 아예 중복이 없는지는 아직 확인을 못함
				m_vGameObjectList[nCurrentObjectIndex]->SetMeshIndex(std::distance(m_vMeshList.begin(), p));
			}
			else {	// 없으면 새로 생성과 동시에 인덱스 지정
				m_vGameObjectList[nCurrentObjectIndex]->SetMeshIndex(m_vMeshList.size());
				m_vMeshList.push_back(std::make_unique<Mesh>(inFile, strLabel));
			}
		}
		else if (strLabel == "<Materials>:") {

		}
		else if (strLabel == "<Children>:") {
			inFile.read((char*)&tempData, sizeof(int));
			if (tempData > 0) {
				for (int i = 0; i < tempData; ++i) {
					readLabel();	// <Frame>: 부분을 빼준다
					AddGameObjectFromFile(inFile, nCurrentObjectIndex);
				}
			}
		}
		else if (strLabel == "</Frame>")
			break;
	}
}