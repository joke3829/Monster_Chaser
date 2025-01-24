#include "GameObject.h"

bool CGameObject::InitializeObjectFromFile(std::ifstream& inFile)
{
	int temp;	// 불필요한 정보들을 빼낸다.
	inFile.read((char*)&temp, sizeof(int));	// 프레임 번호
	inFile.read((char*)&temp, sizeof(int));	// 텍스쳐 수

	// 이름 읽고 저장
	char strLength{};
	inFile.read(&strLength, sizeof(char));
	m_strName.assign(strLength, ' ');
	inFile.read(m_strName.data(), strLength);

	std::string strLabel{};
	auto readLabel = [&]() {
		char nStrLength{};
		inFile.read(&nStrLength, sizeof(char));
		strLabel.assign(nStrLength, ' ');
		inFile.read(strLabel.data(), nStrLength);
		};

	// <Transform>:
	readLabel();
	float dummyFloat[4];
	inFile.read((char*)&m_xmf3Pos, sizeof(XMFLOAT3));
	inFile.read((char*)dummyFloat, sizeof(float) * 3);
	inFile.read((char*)&m_xmf3Scale, sizeof(XMFLOAT3));
	inFile.read((char*)dummyFloat, sizeof(float) * 4);

	// <TransformMatrix>:
	readLabel();
	inFile.read((char*)&m_xmf4x4LocalMatrix, sizeof(XMFLOAT4X4));

	InitializeAxis();
}

Material& CGameObject::getMaterial()
{
	return m_Material;
}

void CGameObject::SetMeshIndex(int index)
{
	m_nMeshIndex = index;
}

void CGameObject::SetParentIndex(int index)
{
	m_nParentIndex = index;
}

void CGameObject::InitializeAxis()
{
	auto normalizeFloat3 = [](XMFLOAT3& xmf) {
		XMStoreFloat3(&xmf, XMVector3Normalize(XMLoadFloat3(&xmf)));
		};
	m_xmf3Right = XMFLOAT3(m_xmf4x4LocalMatrix._11, m_xmf4x4LocalMatrix._12, m_xmf4x4LocalMatrix._13);
	m_xmf3Up = XMFLOAT3(m_xmf4x4LocalMatrix._21, m_xmf4x4LocalMatrix._22, m_xmf4x4LocalMatrix._23);
	m_xmf3Look = XMFLOAT3(m_xmf4x4LocalMatrix._31, m_xmf4x4LocalMatrix._32, m_xmf4x4LocalMatrix._33);

	normalizeFloat3(m_xmf3Right);
	normalizeFloat3(m_xmf3Up);
	normalizeFloat3(m_xmf3Look);
}
