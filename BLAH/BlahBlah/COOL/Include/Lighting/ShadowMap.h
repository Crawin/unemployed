#pragma once

#include "Light.h"


//struct CameraDataShader {
//	XMFLOAT4X4 m_ViewMatrix;
//	XMFLOAT4X4 m_ProjMatrix;
//	XMFLOAT3 m_CameraPosition;
//};

class ShadowMap
{
	static const int m_ShadowMapWidth = 2048;
	static const int m_ShadowMapHeight = 2048;

	static XMFLOAT4X4 m_ShadowPerspectiveProj;
	static XMFLOAT4X4 m_ShadowOrthographicProj;

	SIZE m_MapSize = { 2048, 2048 };

	// ShadowMap Camera Setting
	// 6개가 될 수도 있다.
	CameraDataShader* m_ShaderData = nullptr;
	int m_ShaderDataIdx = -1;
	D3D12_GPU_VIRTUAL_ADDRESS m_ShaderDataGPUAddr = 0;

	// 위치
	XMFLOAT4X4 m_ViewMatrix = Matrix4x4::Identity();

	int m_RenderTargetIdx = -1;

	LIGHT_TYPES m_Type = LIGHT_TYPES::DIRECTIONAL_LIGHT;

public:
	ShadowMap();
	~ShadowMap();

	void SetCameraShaderData(CameraDataShader* data) { m_ShaderData = data; }
	void SetCameraShaderDataIdx(int idx) { m_ShaderDataIdx = idx; }
	void SetCameraShaderDataGPUAddr(D3D12_GPU_VIRTUAL_ADDRESS gpuAddr) { m_ShaderDataGPUAddr = gpuAddr; }
	void SetRenderTargetIdx(int idx) { m_RenderTargetIdx = idx; }

	int GetRenderTargetIdx() const { return m_RenderTargetIdx; }
	int GetCameraDataIdx() const { return m_ShaderDataIdx; }


	// update view matrix by light data
	void UpdateViewMatrixByLight(const LightData& light);

	void SetCameraData(ComPtr<ID3D12GraphicsCommandList> commandList) const;

};

