#include "framework.h"
#include "Component.h"
#include "Scene/ResourceManager.h"
#include <json/json.h>

namespace component 
{
	void Name::Create(Json::Value& v, ResourceManager* rm)
	{
		m_Name = v["Name"].asString();
	}

	void Name::ShowYourself() const
	{
		DebugPrint(std::format("Name Comp, {}", m_Name));
	}

	void Renderer::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value rend = v["Renderer"];

		//m_MeshID = rm->GetMeshToLoad(rend["Mesh"].asString());

		// todo
		// 일단 전부 late load로 해두긴 했는데
		// 추후에 이걸 놔두어도 될지 확인
		rm->AddLateLoad(rend["Mesh"].asString(), rend["Material"].asString(), this);

		// get failed
		//if (m_MeshID == -1) 
		//{
		//	// add to late load
		//	
		//}

		//m_MaterialID = rm->GetMaterialToLoad(rend["Material"].asString());

	}

	void Renderer::ShowYourself() const
	{
		DebugPrint(std::format("Renderer Comp, mesh: {}, material: {}", m_MeshID, m_MaterialID));
	}

	void Transform::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value trans = v["Transform"];

		m_Position.x = trans["Position"][0].asFloat();
		m_Position.y = trans["Position"][1].asFloat();
		m_Position.z = trans["Position"][2].asFloat();

		m_Rotate.x = trans["Rotate"][0].asFloat();
		m_Rotate.y = trans["Rotate"][1].asFloat();
		m_Rotate.z = trans["Rotate"][2].asFloat();
		m_Rotate.w = trans["Rotate"][3].asFloat();

		m_Scale.x = trans["Scale"][0].asFloat();
		m_Scale.y = trans["Scale"][1].asFloat();
		m_Scale.z = trans["Scale"][2].asFloat();
	}

	void Transform::ShowYourself() const
	{
		DebugPrint(std::format("Transform Comp"));
		DebugPrint(std::format("\tPosit: {}, {}, {}", m_Position.x, m_Position.y, m_Position.z));
		DebugPrint(std::format("\tRotat: {}, {}, {}, {}", m_Rotate.x, m_Rotate.y, m_Rotate.z, m_Rotate.w));
		DebugPrint(std::format("\tScale: {}, {}, {}", m_Scale.x, m_Scale.y, m_Scale.z));
	}

	void Camera::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value cam = v["Camera"];

		m_IsMainCamera = cam["IsMainCamera"].asBool();

		if (m_IsMainCamera) rm->SetMainCamera(this);

		m_Right.x = cam["Right"][0].asFloat();
		m_Right.y = cam["Right"][1].asFloat();
		m_Right.z = cam["Right"][2].asFloat();

		m_Up.x = cam["Up"][0].asFloat();
		m_Up.y = cam["Up"][1].asFloat();
		m_Up.z = cam["Up"][2].asFloat();

		m_Look.x = cam["Look"][0].asFloat();
		m_Look.y = cam["Look"][1].asFloat();
		m_Look.z = cam["Look"][2].asFloat();

		m_Fov = cam["Fov"].asFloat();
		m_Aspect = cam["Aspect"].asFloat();
		m_Near = cam["Near"].asFloat();
		m_Far = cam["Far"].asFloat();


		// create shader resource
		m_MappedShaderData = rm->CreateObjectResource(
			sizeof(CameraDataShader),
			"Camera Shader Data",
			(void**)(&m_ShaderData));

		m_ShaderDataGPUAddr = rm->GetVertexDataGPUAddress(m_MappedShaderData);

		BuildProjectionMatrix();

		UpdateShaderData();

		BoundingFrustum::CreateFromMatrix(m_BoundingFrustum, XMLoadFloat4x4(&m_ProjMatrix));
	}

	void Camera::ShowYourself() const
	{
		DebugPrint(std::format("Cam\tPosit: {}, {}, {}", m_Position.x, m_Position.y, m_Position.z));
	}

	void Camera::SetCameraData(ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		// todo 여기 확인해보고 build matrix를 대신 하는 곳이 있는지 찾아봐라
		BuildViewMatrix();
		if (m_ProjChanged) BuildProjectionMatrix();
		UpdateShaderData();

		commandList->SetGraphicsRootConstantBufferView(ROOT_SIGNATURE_IDX::CAMERA_DATA_CBV, m_ShaderDataGPUAddr);
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

	void Input::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void Input::ShowYourself() const
	{
		DebugPrint("Input Comp, nothing");
	}
}
