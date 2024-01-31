#include "framework.h"
#include "Camera.h"
//#include "Renderer/Renderer.h"
#include "Scene/ResourceManager.h"

Camera::Camera()
{
}

Camera::Camera(float fov, float aspect, float nearPlane, float farPlane) :
	m_Fov{ fov }, m_Aspect{ aspect }, m_Near{ nearPlane }, m_Far{ farPlane }
{
}

Camera::~Camera()
{
}

void Camera::Init(ResourceManager* resourceManager)
{
	// 행렬 생성
	BuildViewMatrix();
	BuildProjectionMatrix();

	//cbuffer CameraInfo : register(b1)
	//{
	//	matrix viewMatrix;
	//	matrix projectionMatrix;
	//	// float3 cameraPosition : packoffset(c8); <- 이건 뷰변환 행렬에서 뽑아 쓰자
	//};

	// todo 
	// map 하는 resource들은 따로 관리하자
	// 이건 씬데이터가 가지고 있게 바꾸었다.
	// 초기화 단계에서 씬에게 요청하게 할까?
	// 쉐이더 리소스가 필요한 오브젝트들의 경우는 어디서 해주어야 할까
	// init의 인자에 sceneData를 넣어?
	m_MappedShaderData = resourceManager->CreateObjectResource(
		sizeof(CameraShaderData), 
		"Camera Shader Data",
		(void**)(&m_ShaderData));

	UpdateShaderData();

	BoundingFrustum::CreateFromMatrix(m_BoundingFrustum, XMLoadFloat4x4(&m_ProjMatrix));
}

void Camera::Render(ComPtr<ID3D12GraphicsCommandList> commandList)
{
	BuildViewMatrix();
	UpdateShaderData();

	auto ptr = Renderer::GetInstance().GetVertexDataGPUAddress(m_MappedShaderData);

	commandList->SetGraphicsRootConstantBufferView(ROOT_SIGNATURE_IDX::CAMERA_DATA_CBV, ptr);
}

void Camera::BuildViewMatrix()
{

	// 아래 함수는 look이 아니라 at이라고 봐야함
	//m_ViewMatrix = Matrix4x4::LookAtLH(m_Position, m_Look, m_Up);

	m_Look = Vector3::Normalize(m_Look);
	m_Right = Vector3::CrossProduct(m_Up, m_Look, true);
	m_Up = Vector3::CrossProduct(m_Look, m_Right, true);

	m_ViewMatrix._11 = m_Right.x; m_ViewMatrix._12 = m_Up.x; m_ViewMatrix._13 = m_Look.x;
	m_ViewMatrix._21 = m_Right.y; m_ViewMatrix._22 = m_Up.y; m_ViewMatrix._23 = m_Look.y;
	m_ViewMatrix._31 = m_Right.z; m_ViewMatrix._32 = m_Up.z; m_ViewMatrix._33 = m_Look.z;
	m_ViewMatrix._41 = -Vector3::DotProduct(m_Position, m_Right);
	m_ViewMatrix._42 = -Vector3::DotProduct(m_Position, m_Up);
	m_ViewMatrix._43 = -Vector3::DotProduct(m_Position, m_Look);
	m_ViewMatrix._44 = 1.0f;
	XMFLOAT4X4 inverseView = Matrix4x4::Inverse(m_ViewMatrix);

	//m_BoundingFrustum.Transform(m_BoundingFrustum, XMLoadFloat4x4(&inverseView));
}

void Camera::BuildProjectionMatrix()
{
	m_ProjMatrix = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(m_Fov), m_Aspect, m_Near, m_Far);
}

void Camera::UpdateShaderData()
{
	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&m_ViewMatrix)));

	XMFLOAT4X4 proj;
	XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&m_ProjMatrix)));

	memcpy(&m_ShaderData->m_ViewMatrix, &view, sizeof(XMFLOAT4X4));
	memcpy(&m_ShaderData->m_ProjMatrix, &proj, sizeof(XMFLOAT4X4));
	memcpy(&m_ShaderData->m_CameraPosition, &m_Position, sizeof(XMFLOAT3));
}
