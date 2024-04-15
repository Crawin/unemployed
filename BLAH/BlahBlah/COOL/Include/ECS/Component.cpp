﻿#include "framework.h"
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
	}

	void Renderer::ShowYourself() const
	{
		DebugPrint(std::format("Renderer Comp\n\tmesh: {}, material: {}", m_MeshID, m_MaterialID));
	}

	void AnimationController::Create(Json::Value& v, ResourceManager* rm)
	{
		//  todo
		// resource mamager에게 toLoad 뭐시기를 추가해야 한다.
		Json::Value anim = v["AnimationController"];

		rm->AddLateLoadAnimController(anim["Player"].asString(), this);
	}

	void AnimationController::ShowYourself() const
	{
		DebugPrint(std::format("AnimationController Comp"));
	}

	void AnimationController::UpdateTime(float deltaTime)
	{
		m_AnimationPlayer->Update(deltaTime);
	}

	void AnimationController::ChangeAnimationTo(const std::string& animDef)
	{
		m_AnimationPlayer->ChangeToAnimation(animDef);
	}

	void AnimationExecutor::Create(Json::Value& v, ResourceManager* rm)
	{
		//  todo
		// resource mamager에게 toLoad 뭐시기를 추가해야 한다.
		Json::Value anim = v["AnimationExecutor"];

		rm->AddLateLoadAnimExecutor(anim["Player"].asString(), this);
	}

	void AnimationExecutor::ShowYourself() const
	{
		DebugPrint(std::format("AnimationExecutor Comp"));
	}

	void AnimationExecutor::SetData(ComPtr<ID3D12GraphicsCommandList> commandList, ResourceManager* manager)
	{
		m_AnimationPlayer->SetAnimationData(commandList, manager);
	}

	void Root::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void Root::ShowYourself() const
	{
		DebugPrint("Root Comp");
	}

	void Children::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void Children::ShowYourself() const
	{
		DebugPrint("Children Comp");
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

		m_Scale.x = trans["Scale"][0].asFloat();
		m_Scale.y = trans["Scale"][1].asFloat();
		m_Scale.z = trans["Scale"][2].asFloat();
	}

	void Transform::ShowYourself() const
	{
		DebugPrint(std::format("Transform Comp"));
		DebugPrint(std::format("\tPosit: {}, {}, {}", m_Position.x, m_Position.y, m_Position.z));
		DebugPrint(std::format("\tRotat: {}, {}, {}", m_Rotate.x, m_Rotate.y, m_Rotate.z));
		DebugPrint(std::format("\tScale: {}, {}, {}", m_Scale.x, m_Scale.y, m_Scale.z));

	}

	XMFLOAT4X4 Transform::GetWorldTransform()
	{
		// TODO: 여기에 return 문을 삽입합니다.
		//XMMATRIX mat = XMMatrixMultiply(
		//	XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z), XMMatrixMultiply(
		//		XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_Rotate)),
		//		XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z)));

		XMFLOAT3 rotRad;
		rotRad.x = XMConvertToRadians(m_Rotate.x);
		rotRad.y = XMConvertToRadians(m_Rotate.y);
		rotRad.z = XMConvertToRadians(m_Rotate.z);
		XMMATRIX mat = XMMatrixMultiply(
			XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotRad)),
			XMMatrixMultiply(XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z),
				XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z)));


		XMFLOAT4X4 worldMat = Matrix4x4::Identity();
		XMStoreFloat4x4(&worldMat, XMMatrixMultiply(mat, XMLoadFloat4x4(&m_ParentTransform)));

		return worldMat;
	}

	void Camera::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value cam = v["Camera"];

		m_IsMainCamera = cam["IsMainCamera"].asBool();

		// todo entity에 등록하고 해당 함수를 넣어버려
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

		m_ShaderDataGPUAddr = rm->GetResourceDataGPUAddress(RESOURCE_TYPES::OBJECT, m_MappedShaderData);

		BuildProjectionMatrix();

		UpdateShaderData();

		BoundingFrustum::CreateFromMatrix(m_BoundingFrustum, XMLoadFloat4x4(&m_ProjMatrix));
	}

	void Camera::ShowYourself() const
	{
		DebugPrint(std::format("Camera Component\n\tPosit: {}, {}, {}", m_Position.x, m_Position.y, m_Position.z));
	}

	void Camera::SetCameraData(ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		// todo 여기 확인해보고 build matrix를 대신 하는 곳이 있는지 찾아봐라
		//BuildViewMatrix();
		if (m_ProjChanged) BuildProjectionMatrix();
		UpdateShaderData();

		commandList->SetGraphicsRootConstantBufferView(static_cast<int>(ROOT_SIGNATURE_IDX::CAMERA_DATA_CBV), m_ShaderDataGPUAddr);
	}

	void Camera::BuildViewMatrix()
	{
		// 현재 사용하지 않음
		// 아래 함수는 look이 아니라 at이라고 봐야함
		//m_ViewMatrix = Matrix4x4::LookAtLH(m_Position, m_Look, m_Up);

		m_Look = Vector3::Normalize(m_Look);
		m_Right = Vector3::CrossProduct(m_Up, m_Look, true);
		m_Up = Vector3::CrossProduct(m_Look, m_Right, true);

		//m_ViewMatrix._11 = m_Right.x; m_ViewMatrix._12 = m_Up.x; m_ViewMatrix._13 = m_Look.x;
		//m_ViewMatrix._21 = m_Right.y; m_ViewMatrix._22 = m_Up.y; m_ViewMatrix._23 = m_Look.y;
		//m_ViewMatrix._31 = m_Right.z; m_ViewMatrix._32 = m_Up.z; m_ViewMatrix._33 = m_Look.z;
		//m_ViewMatrix._41 = -Vector3::DotProduct(m_Position, m_Right);
		//m_ViewMatrix._42 = -Vector3::DotProduct(m_Position, m_Up);
		//m_ViewMatrix._43 = -Vector3::DotProduct(m_Position, m_Look);
		//m_ViewMatrix._44 = 1.0f;
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

		XMFLOAT4X4 temp;
		XMStoreFloat4x4(&temp, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_ViewMatrix)));

		m_Position= { temp._41, temp._42, temp._43 };

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

	void Speed::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value s = v["Speed"];

		m_MaxVelocity = s["MaxVelocity"].asFloat();
		m_Acceleration = s["Acceleration"].asFloat();
		//m_CurrentVelocity = s["MaxSpeed"].asFloat();
	}

	void Speed::ShowYourself() const
	{
		DebugPrint("Speed Comp");
		DebugPrint(std::format("\tcur speed: {}, max speed: {}, acc : {}", m_CurrentVelocity, m_MaxVelocity, m_CurrentVelocity));

	}

	void Light::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value light = v["Light"];

		m_IsMainLight = light["MainLight"].asBool();

		// todo
		// rm에게 나 light 쓸래요라고 등록 해야함
		rm->AddLightData();

		m_LightData.m_LightColor.x = light["LightColor"][0].asFloat();
		m_LightData.m_LightColor.y = light["LightColor"][1].asFloat();
		m_LightData.m_LightColor.z = light["LightColor"][2].asFloat();
		m_LightData.m_LightColor.w = light["LightColor"][3].asFloat();

		m_LightData.m_Falloff = light["FallOff"].asFloat();
		m_LightData.m_LightType = light["LightType"].asInt();

		m_LightData.m_Active = light["Active"].asBool();
		m_LightData.m_CastShadow = light["CastShadow"].asBool();
	}

	void Light::ShowYourself() const
	{
		DebugPrint("Light Comp");
		//DebugPrint(std::format("Light map material: {}", m_ShadowMapMaterial));
	}

	void TestInput::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void TestInput::ShowYourself() const
	{
		DebugPrint("TestInput Comp");
	}

	void DiaAnimationControl::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void DiaAnimationControl::ShowYourself() const
	{
		DebugPrint("DiaAnimationControl Comp");
	}

	void DayLight::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value d = v["DayLight"];
		m_DayCycle = d["DayCycle"].asFloat();

		m_NoonLight.x = d["NoonLight"][0].asFloat();
		m_NoonLight.y = d["NoonLight"][1].asFloat();
		m_NoonLight.z = d["NoonLight"][2].asFloat();
		m_NoonLight.w = d["NoonLight"][3].asFloat();

		m_SunSetLight.x = d["SunSetLight"][0].asFloat();
		m_SunSetLight.y = d["SunSetLight"][1].asFloat();
		m_SunSetLight.z = d["SunSetLight"][2].asFloat();
		m_SunSetLight.w = d["SunSetLight"][3].asFloat();

		m_MoonLight.x = d["MoonLight"][0].asFloat();
		m_MoonLight.y = d["MoonLight"][1].asFloat();
		m_MoonLight.z = d["MoonLight"][2].asFloat();
		m_MoonLight.w = d["MoonLight"][3].asFloat();

	}

	void DayLight::ShowYourself() const
	{
		DebugPrint("DayLight Comp");
		DebugPrint(std::format("\tDay Cycle: {}", m_DayCycle));
	}


	void Server::Create(Json::Value& v, ResourceManager* rm)
	{
		m_id = NULL;
	}

	void Server::ShowYourself() const
	{
		DebugPrint("Server Comp");
	}

	void Server::setID(const unsigned int& id)
	{
		m_id = id;
	}

}
