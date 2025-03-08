//-----------------------------------------------------------------------------
// File: Texture.h
// Texture �����Ϳ� ������ �����ϴ� Ŭ����
// 
// �����ϸ鼭 ���� �߰� ����
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "DDSTextureLoader12.h"

extern DXResources g_DxResource;

class CTexture {
public:
	CTexture(const wchar_t* pszFileName, bool bDDS = true);
	void CreateSRV();

	ID3D12DescriptorHeap* getView() const;

	void SetTextureName(std::string name);
	std::string getName() const;
private:
	std::string m_strName{};

	ComPtr<ID3D12Resource> m_pd3dTexture{};
	ComPtr<ID3D12DescriptorHeap> m_pd3dShaderResourceView{};
};

