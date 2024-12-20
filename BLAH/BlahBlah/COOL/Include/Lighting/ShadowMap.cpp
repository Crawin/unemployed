﻿#include "framework.h"
#include "ShadowMap.h"

XMFLOAT4X4 ShadowMap::m_ShadowOrthographicProj = Matrix4x4::Orthographic(m_ShadowMapWidth, m_ShadowMapHeight, 1.0f, 7000.0f);
XMFLOAT4X4 ShadowMap::m_ShadowPerspectiveProj = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(90.0f), 1.0f, 0.1f, 1500.0f);
//XMFLOAT4X4 ShadowMap::m_ShadowPerspectiveProj = Matrix4x4::Transpose(m_ShadowPerspectiveProj);


ShadowMap::ShadowMap()
{
	BoundingFrustum::CreateFromMatrix(m_BoundingFrustumOrtho, XMLoadFloat4x4(&m_ShadowOrthographicProj));
	BoundingFrustum::CreateFromMatrix(m_BoundingFrustumPerspective, XMLoadFloat4x4(&m_ShadowPerspectiveProj));
}

ShadowMap::~ShadowMap()
{
}

void ShadowMap::UpdateViewMatrixByLight(const LightData& light)
{
	if (light.m_CastShadow == false) {
		DebugPrint("ERROR!! this light dont cast shadows");
		return;
	}

	// todo 작성해야 함
	// do something fo cascaded shadow
	if (light.m_CascadedShadow) {

	}

	m_Type = static_cast<LIGHT_TYPES>(light.m_LightType);

	// light position이 갱신 되었다고 치자
	XMFLOAT3 zAxis = { 0,0,1 };
	XMFLOAT3 allAx = { 1,1,1 };

	XMVECTOR from = XMLoadFloat3(&zAxis);
	XMVECTOR to = XMLoadFloat3(&light.m_Direction);
	XMVECTOR axis = XMVector3Normalize(XMVector3Cross(from, to));

	float axisLength = XMVectorGetX(XMVector3Length(axis));
	float dotProduct = XMVectorGetX(XMVector3Dot(from, to));

	float angle = acosf(dotProduct);
	
	XMMATRIX rot;
		if (axisLength == 0) rot = XMMatrixIdentity();
		else rot = XMMatrixRotationAxis(axis, angle);

	XMMATRIX trs = XMMatrixTranslationFromVector(XMLoadFloat3(&light.m_Position));

	XMMATRIX result = rot * trs;

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixInverse(nullptr, result));

	//XMFLOAT3 x;
	//XMFLOAT3 y = { 0,1,0 };
	//XMFLOAT3 z = light.m_Direction;
	//
	//XMVECTOR zVec = XMVector3Normalize(XMLoadFloat3(&z));
	//XMVECTOR yVec = XMLoadFloat3(&y);
	//XMVECTOR xVec = XMVector3Cross(yVec, zVec);
	//yVec = XMVector3Cross(zVec, xVec);

	//XMStoreFloat3(&x, xVec);
	//XMStoreFloat3(&y, yVec);
	//XMStoreFloat3(&z, zVec);

	//XMVECTOR pos = XMLoadFloat3(&light.m_Position);

	//XMMATRIX result = {
	//	x.x, x.y, x.z, 0.0f,
	//	y.x, y.y, y.z, 0.0f,
	//	z.x, z.y, z.z, 0.0f,
	//	-XMVectorGetX(XMVector3Dot(xVec, pos)),
	//	-XMVectorGetX(XMVector3Dot(yVec, pos)),
	//	-XMVectorGetX(XMVector3Dot(zVec, pos)),
	//	1.0f
	//};

	//XMStoreFloat4x4(&m_ViewMatrix, result);

	//XMStoreFloat4x4(&m_ViewMatrix, XMMatrixLookAtRH(pos, lookAt, XMLoadFloat3(&u)));
	//DebugPrint(std::format("angle: {}", XMConvertToDegrees(angle)));
	if (light.m_CastShadow != FALSE) {
		switch (m_Type) {
		case LIGHT_TYPES::DIRECTIONAL_LIGHT:
			// bounding frustum update
			m_BoundingFrustumOrtho.Transform(m_BoundingFrustumWorld, result);

			break;
		case LIGHT_TYPES::SPOT_LIGHT:
			//m_ShadowPerspectiveProj = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(90.0f), 1.0f, 0.1f, light.m_Distance * 1.1f);
			//BoundingFrustum::CreateFromMatrix(m_BoundingFrustumPerspective, XMLoadFloat4x4(&m_ShadowPerspectiveProj));
			m_BoundingFrustumPerspective.Transform(m_BoundingFrustumWorld, result);

			//DebugPrint("No current shadow map setting for spot light now");

			break;
		case LIGHT_TYPES::POINT_LIGHT:
			DebugPrint("No current shadow map setting for point light now");
			break;

		};
	}
}

void ShadowMap::SetCameraData(ComPtr<ID3D12GraphicsCommandList> commandList) const
{
	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&m_ViewMatrix)));

	XMFLOAT4X4 proj = Matrix4x4::Identity();
	switch (m_Type) {
	case LIGHT_TYPES::DIRECTIONAL_LIGHT:
		XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&m_ShadowOrthographicProj)));
		break;

	case LIGHT_TYPES::SPOT_LIGHT:
		//proj = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(90.0f), 1.0f, 0.1f, 500.0f);
		XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&m_ShadowPerspectiveProj)));
		break;

	case LIGHT_TYPES::POINT_LIGHT:
		// todo 만약 point light 라면 6개의 데이터를 set 해줘야 한다.
		XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&m_ShadowPerspectiveProj)));
		break;


	};

	XMFLOAT4X4 temp;
	XMStoreFloat4x4(&temp, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_ViewMatrix)));

	XMFLOAT3 position = { temp._41, temp._42, temp._43 };
	//if (m_Type == LIGHT_TYPES::SPOT_LIGHT) 
	//{
	//	DebugPrint(std::format("camPos: {}, {}, {}", temp._41, temp._42, temp._43));
	//	DebugPrint(std::format("camDir: {}, {}, {}", temp._31, temp._32, temp._33));
	//}

	//DebugPrint(std::format("pos: {}, {}, {}", position.x, position.y, position.z));

	// todo 만약 point light 라면 6개의 데이터를 set 해줘야 한다.
	memcpy(&m_ShaderData->m_ViewMatrix, &view, sizeof(XMFLOAT4X4));
	memcpy(&m_ShaderData->m_ProjMatrix, &proj, sizeof(XMFLOAT4X4));
	memcpy(&m_ShaderData->m_CameraPosition, &position, sizeof(XMFLOAT3));

	commandList->SetGraphicsRootConstantBufferView(static_cast<int>(ROOT_SIGNATURE_IDX::CAMERA_DATA_CBV), m_ShaderDataGPUAddr);
}
