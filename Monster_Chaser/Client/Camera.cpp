#include "Camera.h"


void CCamera::Setup(int nRootParameterIndex)
{
	m_nRootParameterIndex = nRootParameterIndex;
	
	auto desc = BASIC_BUFFER_DESC;
	desc.Width = Align(sizeof(CB_CAMERA_INFO), 256);
	g_DxResource.device->CreateCommittedResource(&UPLOAD_HEAP, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pd3dCameraBuffer.GetAddressOf()));
	m_pd3dCameraBuffer->Map(0, nullptr, (void**)&m_pCameraInfo);

	XMStoreFloat4x4(&m_xmf4x4Proj, XMMatrixPerspectiveFovLH(XMConvertToRadians(m_fFOV), m_fAspect, m_fNear, m_fFar));
}

void CCamera::Rotate(int cxDelta, int cyDelta)
{
	float cx = (float)cxDelta / 3.0f;
	float cy = (float)cyDelta / 3.0f;

	XMMATRIX mtxrotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(cx));
	XMStoreFloat3(&m_xmf3Dir, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Dir), mtxrotate));

	XMVECTOR xmvRight = XMVector3Cross(XMLoadFloat3(&m_xmf3Up), XMLoadFloat3(&m_xmf3Dir));
	mtxrotate = XMMatrixRotationAxis(xmvRight, XMConvertToRadians(cy));
	XMStoreFloat3(&m_xmf3Dir, XMVector3TransformNormal(XMLoadFloat3(&m_xmf3Dir), mtxrotate));
}

void CCamera::UpdateViewMatrix()
{
	XMStoreFloat3(&m_xmf3At, (XMLoadFloat3(&m_xmf3Eye) + XMLoadFloat3(&m_xmf3Dir)));
	XMStoreFloat4x4(&m_xmf4x4View, XMMatrixLookAtLH(XMLoadFloat3(&m_xmf3Eye), XMLoadFloat3(&m_xmf3At), XMLoadFloat3(&m_xmf3Up)));

	XMMATRIX viewProj = XMLoadFloat4x4(&m_xmf4x4View) * XMLoadFloat4x4(&m_xmf4x4Proj);
	m_pCameraInfo->xmf3Eye = m_xmf3Eye;
	XMStoreFloat4x4(&m_pCameraInfo->xmf4x4ViewProj, viewProj);
	XMStoreFloat4x4(&m_pCameraInfo->xmf4x4InverseViewProj, XMMatrixInverse(nullptr, viewProj));
}

void CCamera::SetShaderVariable()
{
	g_DxResource.cmdList->SetComputeRootConstantBufferView(m_nRootParameterIndex, m_pd3dCameraBuffer->GetGPUVirtualAddress());
}