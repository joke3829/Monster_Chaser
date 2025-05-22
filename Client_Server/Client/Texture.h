//-----------------------------------------------------------------------------
// File: Texture.h
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "DDSTextureLoader12.h"

extern DXResources g_DxResource;

class CTexture {
public:
	CTexture(const wchar_t* pszFileName, bool cubeMap = false, bool bDDS = true);
	void CreateSRV(bool cubeMap = false);

	ID3D12DescriptorHeap* getView() const;
	ID3D12Resource* getTexture() const { return m_pd3dTexture.Get(); }
	void SetTextureName(std::string name);
	std::string getName() const;
private:
	std::string m_strName{};

	ComPtr<ID3D12Resource> m_pd3dTexture{};
	ComPtr<ID3D12DescriptorHeap> m_pd3dShaderResourceView{};
};

