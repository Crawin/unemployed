﻿#include "framework.h"
#include "EssentialComponents.h"
#include "Scene/ResourceManager.h"
#include "ECS/ECSManager.h"
#include "json/json.h"
#include "App/Application.h"


namespace component {
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

		rm->AddLateLoad(rend["Mesh"].asString(), rend["Material"].asString(), this);
	}

	void Renderer::ShowYourself() const
	{
		DebugPrint(std::format("Renderer Comp\n\tmesh: {}, material: {}", m_MeshID, m_MaterialID));
	}

	void AnimationController::Create(Json::Value& v, ResourceManager* rm)
	{
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

	void AnimationController::CheckTransition(void* data)
	{
		auto& possiblity = m_TransitionGraph[m_CurrentState];

		// check condition in every possiblity
		for (auto toState : possiblity) {
			if (m_ChangeCondition[std::pair(m_CurrentState, toState)](data) == true) {
				// success
				//m_OnEnter[toState]();

				ChangeAnimationTo(toState);

				m_CurrentState = toState;

				return;
			}
		}
	}

	void AnimationController::ChangeAnimationTo(ANIMATION_STATE animSet)
	{
		if (m_CurrentState != animSet) {
			m_CurrentState = animSet;
			m_AnimationPlayer->ChangeToAnimation(animSet);
		}
	}

	float AnimationController::GetCurrentPlayTime() const
	{
		return m_AnimationPlayer->GetCurrentPlayTime();
	}

	float AnimationController::GetCurrentPlayEndTime() const
	{
		return m_AnimationPlayer->GetCurrentPlayEndTime();
	}

	//void AnimationController::WaitFor(float waitTime)
	//{
	//	m_AnimationPlayer->AddWaitQueue(waitTime);
	//}

	void AnimationExecutor::Create(Json::Value& v, ResourceManager* rm)
	{
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

	void Attach::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value att = v["Attach"];

		m_BoneIndex = att["BoneIDX"].asInt();

		rm->AddLateLoadAttach(att["Player"].asString(), this, att["Mesh"].asString(), m_BoneIndex);
	}

	void Attach::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Transform* tr = manager->GetComponent<Transform>(selfEntity);

		SetOriginalPosition(tr->GetPosition());
		SetOriginalRotation(tr->GetRotation());
		SetOriginalScale(tr->GetScale());
	}

	void Attach::ShowYourself() const
	{
		DebugPrint(std::format("Attach Comp"));
	}

	XMMATRIX& Attach::GetAnimatedBone()
	{
		return m_AnimationPlayer->GetAnimatedBone(m_BoneIndex);
	}


	void Transform::ShowYourself() const
	{
		DebugPrint(std::format("Transform Comp"));
		DebugPrint(std::format("\tPosit: {}, {}, {}", m_Position.x, m_Position.y, m_Position.z));
		DebugPrint(std::format("\tRotat: {}, {}, {}", m_Rotate.x, m_Rotate.y, m_Rotate.z));
		DebugPrint(std::format("\tScale: {}, {}, {}", m_Scale.x, m_Scale.y, m_Scale.z));

	}

	XMFLOAT3 Transform::GetWorldPosition() const
	{
		XMFLOAT3 temp;
		XMStoreFloat3(&temp, XMVector3Transform(XMLoadFloat3(&m_Position), XMLoadFloat4x4(&m_ParentTransform)));

		return temp;
	}

	XMFLOAT3 Transform::GetWorldRotation() const
	{
		XMFLOAT4X4 worldTr = GetWorldTransform();
		XMMATRIX mat = XMLoadFloat4x4(&worldTr);
		
		XMVECTOR s, r, t;
		XMMatrixDecompose(&s, &r, &t, mat);

		float angle;
		XMVECTOR axis;

		XMQuaternionToAxisAngle(&axis, &angle, r);

		XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(r);

		XMFLOAT3 euler;
		euler.x = asinf(-rotationMatrix.r[1].m128_f32[2]); // Pitch
		euler.y = atan2f(rotationMatrix.r[0].m128_f32[2], rotationMatrix.r[2].m128_f32[2]); // Yaw
		euler.z = atan2f(rotationMatrix.r[1].m128_f32[0], rotationMatrix.r[1].m128_f32[1]); // Roll

		euler.x = XMConvertToDegrees(euler.x);
		euler.y = XMConvertToDegrees(euler.y);
		euler.z = XMConvertToDegrees(euler.z);

		return euler;
	}

	XMFLOAT4X4 Transform::GetWorldTransform() const
	{
		XMFLOAT3 rotRad;
		rotRad.x = XMConvertToRadians(m_Rotate.x);
		rotRad.y = XMConvertToRadians(m_Rotate.y);
		rotRad.z = XMConvertToRadians(m_Rotate.z);
		XMMATRIX mat =
			XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
			XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotRad)) *
			XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

		XMFLOAT4X4 worldMat = Matrix4x4::Identity();
		XMStoreFloat4x4(&worldMat, mat * XMLoadFloat4x4(&m_ParentTransform));

		return worldMat;
	}

	XMFLOAT4X4 Transform::GetLocalTransform() const
	{
		XMFLOAT3 rotRad;
		rotRad.x = XMConvertToRadians(m_Rotate.x);
		rotRad.y = XMConvertToRadians(m_Rotate.y);
		rotRad.z = XMConvertToRadians(m_Rotate.z);
		//XMMATRIX mat = XMMatrixMultiply(
		//	XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotRad)),
		//	XMMatrixMultiply(XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z),
		//		XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z)));
		XMMATRIX mat =
			XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
			XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotRad)) *
			XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

		XMFLOAT4X4 worldMat = Matrix4x4::Identity();
		XMStoreFloat4x4(&worldMat, mat);

		return worldMat;
	}

	void Camera::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value cam = v["Camera"];

		m_IsMainCamera = cam["IsMainCamera"].asBool();

		m_Active = cam["Active"].asBool();
		if (m_IsMainCamera) {
			rm->SetMainCamera(this);
			m_ActiveStateOnRender = m_Active = true;
		}
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

		m_NoiseType = cam["Noise"].asInt();
		m_Vignetting = cam["Vignetting"].asBool();

		auto size = Application::GetInstance().GetSize();
		m_Width = size.cx;
		m_Height = size.cy;

		// get camera rendertarget idx
		m_RenderTargetDataIndex = rm->AddCamera(static_cast<int>(MULTIPLE_RENDER_TARGETS::MRT_END));

		// create shader resource
		m_MappedShaderData = rm->CreateObjectResource(
			sizeof(CameraDataShader),
			"Camera Shader Data",
			(void**)(&m_ShaderData));

		m_ShaderDataGPUAddr = rm->GetResourceDataGPUAddress(RESOURCE_TYPES::OBJECT, m_MappedShaderData);

		BuildProjectionMatrix();

		UpdateShaderData();

		BoundingFrustum::CreateFromMatrix(m_BoundingOriginFrustum, XMLoadFloat4x4(&m_ProjMatrix));
	}

	void Camera::ShowYourself() const
	{
		DebugPrint(std::format("Camera Component\n\tPosit: {}, {}, {}", m_Position.x, m_Position.y, m_Position.z));
	}

	void Camera::SetCameraData(ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		if (m_ProjChanged) BuildProjectionMatrix();
		UpdateShaderData();

		commandList->SetGraphicsRootConstantBufferView(static_cast<int>(ROOT_SIGNATURE_IDX::CAMERA_DATA_CBV), m_ShaderDataGPUAddr);
	}

	void Camera::BuildProjectionMatrix()
	{
		m_ProjMatrix = Matrix4x4::PerspectiveFovLH(XMConvertToRadians(m_Fov), m_Aspect, m_Near, m_Far);
	}

	void Camera::UpdateShaderData()
	{
		//XMFLOAT3 viewDir = GetWorldDirection();
		
		//XMFLOAT3 pos = GetWorldPosition();

		//XMVECTOR look = { viewDir.x, viewDir.y, viewDir.z };
		//XMVECTOR up = { 0,1,0 };
		//XMVECTOR right = XMVector3Cross(up, look);
		//up = XMVector3Cross(look, right);
		//XMFLOAT3 pos = GetWorldPosition();

		//XMFLOAT3 r = {m_ViewMatrix._11, m_ViewMatrix._12, m_ViewMatrix._13};
		//XMFLOAT3 u = {m_ViewMatrix._21, m_ViewMatrix._22, m_ViewMatrix._23};
		//XMFLOAT3 v = {m_ViewMatrix._31, m_ViewMatrix._32, m_ViewMatrix._33};

		//DebugPrintVector(r, "x");
		//DebugPrintVector(u, "y");
		//DebugPrintVector(v, "z");
		//DebugPrint("");

		XMFLOAT4X4 view;
		//XMStoreFloat4x4(&view, XMMatrixLookToLH(XMLoadFloat3(&pos), look, up));
		XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&m_ViewMatrix)));
		                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
		XMFLOAT4X4 proj;
		XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&m_ProjMatrix)));

		XMFLOAT4X4 temp;
		XMStoreFloat4x4(&temp, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_ViewMatrix)));

		m_Position = GetWorldPosition();// { temp._41, temp._42, temp._43 };

		memcpy(&m_ShaderData->m_ViewMatrix, &view, sizeof(XMFLOAT4X4));
		memcpy(&m_ShaderData->m_ProjMatrix, &proj, sizeof(XMFLOAT4X4));
		memcpy(&m_ShaderData->m_CameraPosition, &m_Position, sizeof(XMFLOAT3));


		XMFLOAT4X4 inverseView = Matrix4x4::Inverse(m_ViewMatrix);
		m_BoundingOriginFrustum.Transform(m_BoundingFrustum, XMLoadFloat4x4(&inverseView));

	}

	//void Input::Create(Json::Value& v, ResourceManager* rm)
	//{
	//}

	//void Input::ShowYourself() const
	//{
	//	DebugPrint("Input Comp, nothing");
	//}

	void Physics::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value s = v["Physics"];

		m_CurrentMaxVelocity = m_OriginalMaxVelocity = s["MaxVelocity"].asFloat();
		//m_Acceleration = s["Acceleration"].asFloat();

		m_Acceleration.x = s["Acceleration"][0].asFloat();
		m_Acceleration.y = s["Acceleration"][1].asFloat();
		m_Acceleration.z = s["Acceleration"][2].asFloat();

		m_CalculatePhysics = s["Calculate"].asBool();
		m_Friction = s["Friction"].asBool();
		//m_CurrentVelocity = s["MaxSpeed"].asFloat();
	}

	void Physics::ShowYourself() const
	{
		DebugPrint("Speed Comp");
		DebugPrint(std::format("\tcur speed: ({}, {}, {}), max speed: {}, acc : {}", m_Velocity.x, m_Velocity.y, m_Velocity.z, m_OriginalMaxVelocity, m_Acceleration.x));

	}

	void Physics::AddVelocity(const XMFLOAT3& direction, float deltaTime)
	{
		// add direction * deltatime * acceleration

		XMVECTOR dir = XMLoadFloat3(&direction);
		XMVECTOR vel = XMLoadFloat3(&m_Velocity);
		XMVECTOR acc = XMLoadFloat3(&m_Acceleration);

		vel += dir * acc * deltaTime;

		XMStoreFloat3(&m_Velocity, vel);
	}

	void Light::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value light = v["Light"];

		m_IsMainLight = light["MainLight"].asBool();

		rm->AddLightData();

		m_LightData.m_LightColor.x = light["LightColor"][0].asFloat();
		m_LightData.m_LightColor.y = light["LightColor"][1].asFloat();
		m_LightData.m_LightColor.z = light["LightColor"][2].asFloat();
		m_LightData.m_LightColor.w = light["LightColor"][3].asFloat();

		m_LightData.m_LightAmbient.x = light["AmbientColor"][0].asFloat();
		m_LightData.m_LightAmbient.y = light["AmbientColor"][1].asFloat();
		m_LightData.m_LightAmbient.z = light["AmbientColor"][2].asFloat();
		m_LightData.m_LightAmbient.w = light["AmbientColor"][3].asFloat();

		m_LightData.m_Distance = light["Distance"].asFloat();
		m_LightData.m_LightType = light["LightType"].asInt();

		m_LightData.m_Active = light["Active"].asBool();
		m_CastShadow = m_LightData.m_CastShadow = light["CastShadow"].asBool();

		m_LightData.m_Angle = XMConvertToRadians(light["Angle"].asFloat());

		m_LightData.m_Intensity = light["Intensity"].asFloat();

	}

	void Light::ShowYourself() const
	{
		DebugPrint("Light Comp");
		//DebugPrint(std::format("Light map material: {}", m_ShadowMapMaterial));
	}

	void Light::CalculateScore(const XMFLOAT3& camPos, const XMFLOAT3& camDir)
	{
		m_Score = 0;

		switch (static_cast<LIGHT_TYPES>(m_LightData.m_LightType)) {
		case LIGHT_TYPES::DIRECTIONAL_LIGHT:
			if (m_LightData.m_Direction.y > 0) m_Score = -10000.0f;
			else m_Score = 10000000000;
			m_Score = 10000000000;
			break;

		case LIGHT_TYPES::SPOT_LIGHT:
		{
			// if the light is down
			if (camPos.y > m_LightData.m_Position.y) {
				m_Score = -10000.0f;
				break;
			}
			XMVECTOR camDirVec = XMVector3Normalize(XMLoadFloat3(&camDir));
			XMVECTOR lightDirVec = XMVector3Normalize(XMLoadFloat3(&m_LightData.m_Direction));
			XMVECTOR camToLight = XMLoadFloat3(&m_LightData.m_Position) - XMLoadFloat3(&camPos);
			XMVECTOR camLightDirAdd = XMVector3Normalize(lightDirVec + XMVector3Normalize(camToLight));

			float distFactor = std::clamp(700.0f / std::max(XMVectorGetX(XMVector3Length(camToLight)), 0.1f), 0.0f, 1.0f);
			float dotFactor = XMVectorGetX(XMVector3Dot(camLightDirAdd, camDirVec));
			dotFactor = pow(dotFactor, 2.0f);
			m_Score = distFactor + dotFactor;
			//DebugPrint(std::format("distFactor: {}\ndotFactor: {}\n", distFactor, dotFactor));
			break;
		}

		}
	}

	void TestInput::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void TestInput::ShowYourself() const
	{
		DebugPrint("TestInput Comp");
	}

	void DayLight::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value d = v["DayLight"];

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

		m_MinAmbient.x = d["MinAmbient"][0].asFloat();
		m_MinAmbient.y = d["MinAmbient"][1].asFloat();
		m_MinAmbient.z = d["MinAmbient"][2].asFloat();
		m_MinAmbient.w = d["MinAmbient"][3].asFloat();

		m_MaxAmbient.x = d["MaxAmbient"][0].asFloat();
		m_MaxAmbient.y = d["MaxAmbient"][1].asFloat();
		m_MaxAmbient.z = d["MaxAmbient"][2].asFloat();
		m_MaxAmbient.w = d["MaxAmbient"][3].asFloat();

		m_RenderShader = d["RenderShader"].asBool();

		m_Day = d["Day"].asBool();

	}

	void DayLight::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		m_LightComponent = manager->GetComponent<Light>(selfEntity);
		m_OriginRotate = manager->GetComponent<Transform>(selfEntity)->GetRotation();
	}

	void DayLight::ShowYourself() const
	{
		DebugPrint("DayLight Comp");
	}


	void Server::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value ser = v["Server"];

		m_id = ser["ID"].asInt();
	}

	void Server::ShowYourself() const
	{
		DebugPrint("Server Comp");
	}

	void Server::setID(const unsigned int& id)
	{
		m_id = id;
	}

	void SelfEntity::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void SelfEntity::ShowYourself() const
	{
		DebugPrint("SelfEntity Comp");
	}

	void Collider::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value col = v["Collider"];

		m_StaticObject = col["Static"].asBool();
		m_Trigger = col["IsTrigger"].asBool();

		// if true on create, load collider by its mesh
		// else create here
		m_Collided = col["AutoMesh"].asBool();

		if (m_Collided == false) {
			XMFLOAT3 center;
			center.x = col["Center"][0].asFloat();
			center.y = col["Center"][1].asFloat();
			center.z = col["Center"][2].asFloat();

			XMFLOAT3 extent;
			extent.x = col["Extent"][0].asFloat();
			extent.y = col["Extent"][1].asFloat();
			extent.z = col["Extent"][2].asFloat();

			XMVECTOR rotate = { XMConvertToRadians(col["Rotate"][0].asFloat()), XMConvertToRadians(col["Rotate"][1].asFloat()), XMConvertToRadians(col["Rotate"][2].asFloat()) };
			XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(rotate);

			m_BoundingBoxOriginal = BoundingOrientedBox(center, extent, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
			m_BoundingBoxOriginal.Transform(m_BoundingBoxOriginal, rot);

			// if capsue, check dist
			m_IsCapsule = col["IsCapsule"].asBool();
		}

		bool isSyncMesh = col["SyncMesh"].asBool();

		if (isSyncMesh) {
			XMFLOAT3 center;
			center.x = col["Center"][0].asFloat();
			center.y = col["Center"][1].asFloat();
			center.z = col["Center"][2].asFloat();

			XMFLOAT3 extent;
			extent.x = col["Extent"][0].asFloat();
			extent.y = col["Extent"][1].asFloat();
			extent.z = col["Extent"][2].asFloat();

			XMVECTOR rotate = { XMConvertToRadians(col["Rotate"][0].asFloat()), XMConvertToRadians(col["Rotate"][1].asFloat()), XMConvertToRadians(col["Rotate"][2].asFloat()) };
			XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(rotate);

			m_BoundingBoxOriginal = BoundingOrientedBox(center, extent, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
			m_BoundingBoxOriginal.Transform(m_BoundingBoxOriginal, rot);

			m_IsCapsule = true;
			m_Collided = true;
		}


	}

	void Collider::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		ResetList();
	}

	void Collider::ShowYourself() const
	{
		DebugPrint("Collider Comp");
	}

	void Collider::UpdateBoundingBox(const XMMATRIX& transMat)
	{
		m_BoundingBoxOriginal.Transform(m_CurrentBox, transMat);
		XMStoreFloat4(&m_CurrentBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_CurrentBox.Orientation)));
	}

	void Collider::InsertCollidedEntity(Entity* ent)
	{
		auto iter = find_if(m_CollidedEntities.begin(), m_CollidedEntities.end(), [ent](const CollidedEntity& col) { return col.m_Entity == ent; });

		if (iter == m_CollidedEntities.end())
			m_CollidedEntities.emplace_back(COLLIDE_EVENT_TYPE::BEGIN, ent);
		else
			iter->m_Type = COLLIDE_EVENT_TYPE::ING;
	}

	void Collider::UpdateCollidedList()
	{
		for (auto iter = m_CollidedEntities.begin(); iter != m_CollidedEntities.end();) {
			switch (iter->m_Type) {
			case COLLIDE_EVENT_TYPE::BEGIN:
			case COLLIDE_EVENT_TYPE::ING:
				iter->m_Type = COLLIDE_EVENT_TYPE::END;
				++iter;
				break;

			case COLLIDE_EVENT_TYPE::END:
				iter = m_CollidedEntities.erase(iter);
				break;
			}
		}

	}

	const EventFunctionMap* Collider::GetEventMap(COLLIDE_EVENT_TYPE type) const
	{
		switch (type) {
		case COLLIDE_EVENT_TYPE::BEGIN:		return &m_EventFunctions.m_OnBeginOverlap;
		case COLLIDE_EVENT_TYPE::ING:		return &m_EventFunctions.m_OnOverlapping;
		case COLLIDE_EVENT_TYPE::END:		return &m_EventFunctions.m_OnEndOverlap;
		}
		return nullptr;
	}

	void DynamicCollider::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value col = v["DynamicCollider"];

		m_Active = col["Active"].asBool();
		m_StaticObject = col["Static"].asBool();
		m_Trigger = col["IsTrigger"].asBool();

		// if true on create, load collider by its mesh
		// else create here
		m_Collided = col["AutoMesh"].asBool();

		m_ColideWithDynamic = col["ColideWithDynamic"].asBool();

		m_PossibleClimbHeight = col["ClimbHeight"].asInt();

		if (m_Collided == false) {
			XMFLOAT3 center;
			center.x = col["Center"][0].asFloat();
			center.y = col["Center"][1].asFloat();
			center.z = col["Center"][2].asFloat();

			XMFLOAT3 extent;
			extent.x = col["Extent"][0].asFloat();
			extent.y = col["Extent"][1].asFloat();
			extent.z = col["Extent"][2].asFloat();

			m_BoundingBoxOriginal = BoundingOrientedBox(center, extent, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

			// if capsue, check dist
			m_IsCapsule = col["IsCapsule"].asBool();
		}
	}

	void DynamicCollider::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		ResetList();
	}

	void DynamicCollider::ShowYourself() const
	{
		DebugPrint("Collider Comp");
	}

	void DynamicCollider::UpdateBoundingBox(const XMMATRIX& transMat)
	{
		m_BoundingBoxOriginal.Transform(m_CurrentBox, transMat);
	}

	void DynamicCollider::InsertCollidedEntity(Entity* ent)
	{
		auto iter = find_if(m_CollidedEntities.begin(), m_CollidedEntities.end(), [ent](const CollidedEntity& col) { return col.m_Entity == ent; });

		if (iter == m_CollidedEntities.end())
			m_CollidedEntities.emplace_back(COLLIDE_EVENT_TYPE::BEGIN, ent);
		else
			iter->m_Type = COLLIDE_EVENT_TYPE::ING;
	}

	void DynamicCollider::UpdateCollidedList()
	{
		for (auto iter = m_CollidedEntities.begin(); iter != m_CollidedEntities.end();) {
			switch (iter->m_Type) {
			case COLLIDE_EVENT_TYPE::BEGIN:
			case COLLIDE_EVENT_TYPE::ING:
				iter->m_Type = COLLIDE_EVENT_TYPE::END;
				++iter;
				break;

			case COLLIDE_EVENT_TYPE::END:
				iter = m_CollidedEntities.erase(iter);
				break;
			}
		}
	}

	const EventFunctionMap* DynamicCollider::GetEventMap(COLLIDE_EVENT_TYPE type) const
	{
		switch (type) {
		case COLLIDE_EVENT_TYPE::BEGIN:		return &m_EventFunctions.m_OnBeginOverlap;
		case COLLIDE_EVENT_TYPE::ING:		return &m_EventFunctions.m_OnOverlapping;
		case COLLIDE_EVENT_TYPE::END:		return &m_EventFunctions.m_OnEndOverlap;
		}
		return nullptr;
	}

	void AttachInput::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void AttachInput::ShowYourself() const
	{
	}

	void Interaction::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void Interaction::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		auto col = manager->GetComponent<Collider>(selfEntity);
		auto dco = manager->GetComponent<DynamicCollider>(selfEntity);

		if (!((col && col->IsTrigger() == true) ||
			 (dco && dco->IsTrigger() == true)))
		{
			Name* name = manager->GetComponent<Name>(selfEntity);
			ERROR_QUIT(std::format("ERROR!! no collider on Interaction Component, name: {}", name->getName()));
		}
	}

	void Interaction::ShowYourself() const
	{
	}

	void Player::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value pl = v["Player"];

		m_OriginalPosition.x = pl["OriginalPosition"][0].asFloat();
		m_OriginalPosition.y = pl["OriginalPosition"][1].asFloat();
		m_OriginalPosition.z = pl["OriginalPosition"][2].asFloat();
		
		m_OriginalRotate.x = pl["OriginalRotate"][0].asFloat();
		m_OriginalRotate.y = pl["OriginalRotate"][1].asFloat();
		m_OriginalRotate.z = pl["OriginalRotate"][2].asFloat();
	}


	void Player::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		// make interaction enable
		EventFunction begin = [manager](Entity* playerEnt, Entity* interaction) {
			auto pawn = manager->GetComponent<component::Pawn>(playerEnt);
			pawn->SetInteractionEntity(interaction);
			};
		EventFunction ing = [manager](Entity* playerEnt, Entity* interaction) {
			//DebugPrint("ING");
			};
		EventFunction end = [manager](Entity* playerEnt, Entity* interaction) {
			auto pawn = manager->GetComponent<component::Pawn>(playerEnt);
			if (pawn->GetInteractionEntity() == interaction)
				pawn->SetInteractionEntity(nullptr);
			};

		DynamicCollider* dcol = manager->GetComponent<DynamicCollider>(selfEntity);

		if (dcol != nullptr) {
			dcol->InsertEvent<component::Interaction>(begin, COLLIDE_EVENT_TYPE::BEGIN);
			dcol->InsertEvent<component::Interaction>(ing, COLLIDE_EVENT_TYPE::ING);
			dcol->InsertEvent<component::Interaction>(end, COLLIDE_EVENT_TYPE::END);
		}
		else {
			ERROR_QUIT("ERROR!!! no dynamic collider on Player Entity!!");
		}

		//Transform* tr = manager->GetComponent<Transform>(selfEntity);
		//m_OriginalPosition = tr->GetPosition();
		//m_OriginalRotate = tr->GetRotation();
	}

	void Player::ShowYourself() const
	{
	}

	void Pawn::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value pa = v["Pawn"];
		m_CameraSocketName = pa["CameraName"].asString();

		if (pa["HudName"].isNull() == false) {
			m_HudName = pa["HudName"].asString();
		}
	}

	void Pawn::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		m_CameraEntity = manager->GetEntityFromRoute(m_CameraSocketName, selfEntity);
		m_Camera = manager->GetComponent<Camera>(m_CameraEntity);
		m_Physics = manager->GetComponent<Physics>(selfEntity);
		m_Inventory = manager->GetComponent<Inventory>(selfEntity);
		m_SelfEntity = selfEntity;

		if (m_HudName != "") {
			UICanvas* canvas = nullptr;

			std::function<void(UICanvas*, Name*)> findHud = [this](UICanvas* can, Name* name) { if (name->getName() == m_HudName) m_HudCanvas = can; };
			manager->Execute(findHud);
		}
	}

	void Pawn::ShowYourself() const
	{
	}

	void Pawn::ResetInput()
	{
		memset(m_KeyStates, sizeof(m_KeyStates), 0);
	}

	void Pawn::TickInput()
	{
		// if pressing
		if (m_ControledByServer == false) {				// 클라 본인
			for (KEY_STATE& state : m_KeyStates) {
				switch (state) {
				case KEY_STATE::START_PRESS:
				case KEY_STATE::PRESSING:
					state = KEY_STATE::END_PRESS;
					break;

				case KEY_STATE::END_PRESS:
					state = KEY_STATE::NONE;
					break;
				}
			}
		}
		else {											// 상대방
			for (KEY_STATE& state : m_KeyStates) {
				switch (state) {
				case KEY_STATE::START_PRESS:
				case KEY_STATE::PRESSING:
					state = KEY_STATE::PRESSING;
					break;

				case KEY_STATE::END_PRESS:
					state = KEY_STATE::NONE;
					break;
				}
			}
		}
		m_MouseDif = { 0, 0 };

	}

	void Pawn::PressInput(GAME_INPUT key)
	{
		auto& state = m_KeyStates[static_cast<long long int>(key)];

		switch (state) {
		case KEY_STATE::START_PRESS:
			// todo 여기를 hit 하는지 확인해보자
			// hit 된다면 input을 두번 넣어준다는 뜻이 될 것이다.
			DebugPrint("SHOULD NOT HIT HERE!!!");
			[[fallthrough]];
		case KEY_STATE::PRESSING:
		case KEY_STATE::END_PRESS:
			state = KEY_STATE::PRESSING;
			break;

		case KEY_STATE::NONE:
			state = KEY_STATE::START_PRESS;
			break;
		}

	}

	bool Pawn::IsPressing(GAME_INPUT key) const
	{
		return
			m_KeyStates[static_cast<long long int>(key)] == KEY_STATE::START_PRESS ||
			m_KeyStates[static_cast<long long int>(key)] == KEY_STATE::PRESSING;
	}

	void Pawn::SetActive(bool active, ECSManager* manager)
	{
		m_Active = active;
		m_Camera->SetActive(active);
		m_Camera->SetMainCamera(active);
		if (m_Physics) m_Physics->SetCalculateState(active);
		if (m_Inventory) {
			m_Inventory->SetMainMode(active, manager);
		}

		if (m_HudCanvas) {
			if (active) m_HudCanvas->ShowUI(false);
			else m_HudCanvas->HideUI(false);
		}
	}
	

	void PlayerController::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value ctrl = v["PlayerController"];

		m_TargetEntityName = ctrl["ControllEntity"].asString();

	}

	void PlayerController::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
	}

	void PlayerController::ShowYourself() const
	{
	}

	bool PlayerController::Possess(ECSManager* manager, const std::string& targetName)
	{
		// reset inputs
		if (m_CurrentPossess) {
			m_CurrentPossess->ResetInput();
			m_CurrentPossess->SetActive(false, manager);
		}

		Pawn* targetPawn = nullptr;
		std::function<void(Name*, Pawn*)> findTarget = [&targetName, &targetPawn](Name* name, Pawn* pawn) {
			if (name->getName() == targetName)
				targetPawn = pawn;
			};

		manager->Execute(findTarget);

		if (targetPawn) targetPawn->SetActive(true, manager);
		m_CurrentPossess = targetPawn;

		// returns success/fail
		return (m_CurrentPossess != nullptr);
	}

	bool PlayerController::Possess(Pawn* target, ECSManager* manager)
	{
		// reset inputs
		if (m_CurrentPossess) {
			m_CurrentPossess->ResetInput();
			m_CurrentPossess->SetActive(false, manager);
		}

		if (target == nullptr) return false;

		m_CurrentPossess = target;
		m_CurrentPossess->SetActive(true, manager);

		return true;
	}

	int DayLightManager::m_ManagerCount = 0;
	void DayLightManager::Create(Json::Value& v, ResourceManager* rm)
	{
		//m_ManagerCount++;
		//if (m_ManagerCount > 1) 
		//	ERROR_QUIT("ERROR!!, DayLightManager is two");

		Json::Value d = v["DayLightManager"];
		m_CurTime = d["Time"].asFloat();
		m_DayCycle = d["DayCycle"].asFloat();
	}

	void DayLightManager::TimeAdd(float deltaTime)
	{
		// 360.0f / 24.0f
		float timeAdd = 24.0f / m_DayCycle;
		m_CurTime += deltaTime * timeAdd;

		if (m_CurTime >= 24.0f) m_CurTime -= 24.0f;
	}

	void AI::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value ai = v["AI"];

		m_Type = ai["Type"].asInt();
	}


	void Particle::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value pc = v["Particle"];

		std::string particleName = pc["Particle"].asString();
		std::string fullRoute = "Particle/";
		fullRoute += particleName;

		m_ParticleType = ConvertStringToParticleType(particleName);
		m_MaxParticle = pc["MaxParticle"].asInt();
		m_DefaultLifeTime = pc["DefaultLifeTime"].asInt();

		m_ParticleSize.x = pc["Size"][0].asFloat();
		m_ParticleSize.y = pc["Size"][1].asFloat();

		// load material
		rm->AddLateLoadUI(fullRoute, nullptr);

		// request make vertex buffer view to upload buffer
		m_MappedShaderData = rm->CreateObjectResource(
			sizeof(XMFLOAT3) * m_MaxParticle,
			"Particle Data",
			(void**)(&m_ShaderData), D3D12_HEAP_TYPE_UPLOAD, RESOURCE_TYPES::VERTEX);

		m_ShaderDataGPUAddr = rm->GetResourceDataGPUAddress(RESOURCE_TYPES::VERTEX, m_MappedShaderData);

		m_ParticleBufferView.BufferLocation = m_ShaderDataGPUAddr;
		m_ParticleBufferView.StrideInBytes = sizeof(XMFLOAT3);
		m_ParticleBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_MaxParticle;

		// std::vector일 때 주석 풀 것
		//m_ParticlePositions.reserve(m_MaxParticle);
		//m_ParticleVelocityAndLifetime.reserve(m_MaxParticle);
	}

	void Particle::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		//std::string fullRoute = "Particle/";
		//fullRoute += ConvertParticleTypeToString(m_ParticleType);
		m_ParticleMaterial = rm->GetMaterial(rm->GetMaterial(ConvertParticleTypeToString(m_ParticleType)));
		
		// set glow
		m_ParticleMaterial->SetExtraDataIndex(0, 1.0f);

		m_ParticleDatas = std::list<ParticleData>();
	}

	void Particle::Tick(float deltaTime)
	{
		auto iter = m_ParticleDatas.begin();
		for (iter; iter != m_ParticleDatas.end(); ++iter) {
			// add velocity
			iter->m_Position.x += iter->m_Velocity.x * deltaTime;
			iter->m_Position.y += iter->m_Velocity.y * deltaTime;
			iter->m_Position.z += iter->m_Velocity.z * deltaTime;

			// gravity
			iter->m_Velocity.y -= 980.0f * deltaTime;

			// friction??
			iter->m_LifeTime -= deltaTime;
		}

	}

	void Particle::InsertParticle(XMFLOAT3 pos, XMFLOAT3 vel, int toInsert, float randRange)
	{
		std::random_device rd;
		std::uniform_real_distribution<float> xPos(pos.x - randRange, pos.x + randRange);
		std::uniform_real_distribution<float> yPos(pos.y - randRange, pos.y + randRange);
		std::uniform_real_distribution<float> zPos(pos.z - randRange, pos.z + randRange);
		std::uniform_real_distribution<float> xVel(vel.x - abs(vel.x / 5.0f), vel.x + abs(vel.x / 5.0f));
		std::uniform_real_distribution<float> yVel(vel.y - abs(vel.y / 5.0f), vel.y + abs(vel.y / 5.0f));
		std::uniform_real_distribution<float> zVel(vel.z - abs(vel.z / 5.0f), vel.z + abs(vel.z / 5.0f));
		std::uniform_real_distribution<float> lifeTimeRange(m_DefaultLifeTime - 1.0f, m_DefaultLifeTime + 1.0f);
		std::default_random_engine dre(rd());

		XMFLOAT3 newPos;
		XMFLOAT4 newVelAndLifeTime;
		float newLifeTime;
		for (int i = 0; i < toInsert; ++i) {
			if (m_ParticleDatas.size() >= m_MaxParticle) break;

			ParticleData newData;
			newData.m_Position = { xPos(dre), yPos(dre), zPos(dre) };
			newData.m_Velocity = { xVel(dre), yVel(dre), zVel(dre) };
			newData.m_LifeTime = lifeTimeRange(dre);

			m_ParticleDatas.push_back(newData);
		}
	}

	void Particle::SyncParticle()
	{
		// delete if velocity over
		auto iter = m_ParticleDatas.begin();
		for (iter; iter != m_ParticleDatas.end();) {
			if (iter->m_LifeTime <= 0)
				iter = m_ParticleDatas.erase(iter);
			else
				++iter;
		}

		int cnt = 0;
		for (const auto& pos : m_ParticleDatas) {
			memcpy(m_ShaderData + cnt++, &pos.m_Position, sizeof(XMFLOAT3));
		}
	}

	void Particle::OnRender(ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		// todo
		// set size here
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		m_ParticleMaterial->SetDatas(commandList, static_cast<int>(ROOT_SIGNATURE_IDX::DESCRIPTOR_IDX_CONSTANT));
		commandList->SetGraphicsRoot32BitConstants(static_cast<int>(ROOT_SIGNATURE_IDX::WORLD_MATRIX), 2, &m_ParticleSize, 0);

		//commandList->IASetPrimitiveTopology()
		commandList->IASetVertexBuffers(0, 1, &m_ParticleBufferView);
		commandList->DrawInstanced(m_ParticleDatas.size(), 1, 0, 0);
	}

	void ParticleEmitter::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value pe = v["ParticleEmitter"];

		m_ParticleType = ConvertStringToParticleType(pe["Type"].asString());

		m_EmitteMount = pe["EmitteMount"].asInt();
		m_EmittePerSec = pe["EmittePerSec"].asFloat();
		m_RandomMount = pe["RandomMount"].asFloat();
	}

	void ParticleEmitter::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
	}

	bool ParticleEmitter::Tick(float deltaTime)
	{
		m_TimeElapsed += deltaTime;

		if (1.0f / m_EmittePerSec <= m_TimeElapsed) {
			//m_TimeElapsed -= m_EmittePerSec / 60.0f;
			m_TimeElapsed = 0.0f;
			return true;
		}

		return false;
	}

}
