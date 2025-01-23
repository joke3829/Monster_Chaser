#include "MeshRender.h"

void MeshRender::Release()
{
	m_Mesh = nullptr;
}

void MeshRender::ReleaseUploadBuffers()
{
	if (m_Mesh)
	{
		//m_Mesh->ReleaseUploadBuffers();
	}
	if (m_Material)
	{
		//m_Material->ReleaseUploadBuffers();
	}
}