#include "framework.h"
#include "ShadowMap.h"

XMFLOAT4X4 ShadowMap::m_ShadowOrthographicProj = Matrix4x4::Orthographic(m_ShadowMapWidth, m_ShadowMapHeight, 1.0f, 7000.0f);
XMFLOAT4X4 ShadowMap::m_ShadowPerspectiveProj = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(XMConvertToRadians(90.0f)), 1.0f, 0.1f, 10000.0f);
//XMFLOAT4X4 ShadowMap::m_ShadowPerspectiveProj = Matrix4x4::Transpose(m_ShadowPerspectiveProj);


ShadowMap::ShadowMap()
{

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

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixInverse(nullptr, rot * trs));

	//XMStoreFloat4x4(&m_ViewMatrix, XMMatrixLookAtRH(pos, lookAt, XMLoadFloat3(&u)));

	//switch (m_Type) {
	//case LIGHT_TYPES::DIRECTIONAL_LIGHT:

	//	break;
	//case LIGHT_TYPES::SPOT_LIGHT:
	//	DebugPrint("No current shadow map setting for spot light now");

	//	break;
	//case LIGHT_TYPES::POINT_LIGHT:
	//	DebugPrint("No current shadow map setting for point light now");
	//	break;

	//};
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

	//DebugPrint(std::format("pos: {}, {}, {}", position.x, position.y, position.z));

	// todo 만약 point light 라면 6개의 데이터를 set 해줘야 한다.
	memcpy(&m_ShaderData->m_ViewMatrix, &view, sizeof(XMFLOAT4X4));
	memcpy(&m_ShaderData->m_ProjMatrix, &proj, sizeof(XMFLOAT4X4));
	memcpy(&m_ShaderData->m_CameraPosition, &position, sizeof(XMFLOAT3));

	commandList->SetGraphicsRootConstantBufferView(static_cast<int>(ROOT_SIGNATURE_IDX::CAMERA_DATA_CBV), m_ShaderDataGPUAddr);
}
