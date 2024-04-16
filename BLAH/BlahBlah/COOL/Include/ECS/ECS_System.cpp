#include "framework.h"
#include "ECS_System.h"
#include "Entity.h"
#include "Component.h"
#include "ECSManager.h"
#include "App/InputManager.h"
#include "Network/Client.h"

namespace ECSsystem {

	void LocalToWorldTransform::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		std::function<void(Name*, Transform*, Children*)> func = [&func, &manager](Name* name, Transform* trans, Children* childComp) {
			Entity* ent = childComp->GetEntity();

			const std::vector<Entity*>& children = ent->GetChildren();

			// build world matrix for child
			XMFLOAT4X4 parentMatrix = trans->GetWorldTransform();

			XMFLOAT4X4 temp;

			//DebugPrint(std::format("parent: {}", name->getName()));


			for (Entity* child : children) {
				auto bit = child->GetBitset();
				int innerId = child->GetInnerID();

				// todo 지우자
				Name* childName = manager->GetComponent<Name>(bit, innerId);
				//DebugPrint(std::format("\tchild: {}", childName->getName()));

				Transform* childTrans = manager->GetComponent<Transform>(bit, innerId);
				if (childTrans != nullptr) {
					XMFLOAT4X4 myTransform = childTrans->GetLocalTransform();
					XMStoreFloat4x4(&temp, XMLoadFloat4x4(&myTransform) * XMLoadFloat4x4(&parentMatrix));

					// do st trans to child
					// make world transform from this trans
					childTrans->SetParentTransform(temp);

					// execute this func to child
					manager->ExecuteFromEntity(bit, innerId, func);
				}
				else DebugPrint("ERROR!! no transform ");
			}
			};

		manager->ExecuteRoot(func);

		//DebugPrint(std::format("entities: {}", temp));

	}

	void AnimationPlayTimeAdd::Update(ECSManager* manager, float deltaTime)
	{
		std::function<void(component::AnimationController*)> func = [manager, deltaTime](component::AnimationController* anim) {
			anim->UpdateTime(deltaTime);
			};

		manager->Execute(func);
	}

	void SyncWithTransform::Update(ECSManager* manager, float deltaTime)
	{
		// sync with camera
		// first person cam
		std::function<void(component::Transform*, component::Camera*)> func1 = [](component::Transform* tr, component::Camera* cam) {

			//XMFLOAT3 rotate = tr->GetRotation();
			//XMFLOAT3 rad;
			//rad.x = XMConvertToRadians(rotate.x);
			//rad.y = XMConvertToRadians(rotate.y);
			//rad.z = XMConvertToRadians(rotate.z);

			//XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&(rad)));
			//XMMATRIX trs = XMMatrixTranslationFromVector(XMLoadFloat3(&(tr->GetPosition())));

			//XMFLOAT3 pos = tr->GetPosition();
			//DebugPrint(std::format("cam: {}, {}, {}", pos.x, pos.y, pos.z));

			//cam->SetPosition(tr->GetPosition());
			//XMStoreFloat4x4(&(cam->m_ViewMatrix), XMMatrixInverse(nullptr, rot * trs));
			XMStoreFloat4x4(&(cam->m_ViewMatrix), XMMatrixInverse(nullptr, XMLoadFloat4x4(&tr->GetWorldTransform())));

			};

		manager->Execute(func1);

		// sync with renderer world matrix
		std::function<void(component::Transform*, component::Renderer*)> func2 = [](component::Transform* tr, component::Renderer* ren) {
			ren->SetWorldMatrix(tr->GetWorldTransform());

			};

		manager->Execute(func2);

		// sync with Light
		std::function<void(component::Transform*, component::Light*)> func3 = [manager](component::Transform* tr, component::Light* li) {
			LightData& light = li->GetLightData();

			XMFLOAT3 direction = { 0.0f, 0.0f, 1.0f };
			XMFLOAT3 rotate = tr->GetRotation();
			XMFLOAT3 rad;
			rad.x = XMConvertToRadians(rotate.x);
			rad.y = XMConvertToRadians(rotate.y);
			rad.z = XMConvertToRadians(rotate.z);

			XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&(rad)));
			
			// light.m_Direction;
			XMStoreFloat3(&light.m_Direction, XMVector3Normalize(XMVector3Transform(XMLoadFloat3(&direction), rot)));
			light.m_Position = tr->GetPosition();
			};

		manager->Execute(func3);

	}

	void Friction::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;
		std::function<void(Speed*)> func = [deltaTime](Speed* sp) {
			const float friction = 200.0f;
			// if moving
			float speed = sp->GetCurrentVelocityLen();// sp->GetCurrentVelocity();

			if (speed > 0) {
				// do friction here
				XMVECTOR vel = XMLoadFloat3(&sp->GetVelocity());
				XMVECTOR nor = XMVector3Normalize(vel);
				XMVECTOR res = vel - nor * friction * deltaTime;

				XMFLOAT3 temp;
				XMStoreFloat3(&temp, res);

				// 부호가 바뀜
				if (XMVectorGetX(vel) * XMVectorGetX(res) < 0) temp.x = 0;
				if (XMVectorGetY(vel) * XMVectorGetY(res) < 0) temp.y = 0;
				if (XMVectorGetZ(vel) * XMVectorGetZ(res) < 0) temp.z = 0;

				// update
				sp->SetVelocity(temp);
				speed = sp->GetCurrentVelocityLen();

				if (abs(speed) < 0.01f) {
					sp->SetVelocity({ 0,0,0 });
				}
			}


			};

		manager->Execute(func);
	}

	void MoveByInput::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// input으로 speed 업데이트
		std::function<void(Transform*, Input*, Speed*)> inputFunc = [deltaTime](Transform* tr, Input* in, Speed* sp) {
			// keyboard input
			XMFLOAT3 tempMove = { 0.0f, 0.0f, 0.0f };
			bool move = false;
			if (GetAsyncKeyState('W') & 0x8000) { tempMove.z += 1.0f; move = true; }
			if (GetAsyncKeyState('S') & 0x8000) { tempMove.z -= 1.0f; move = true; }
			if (GetAsyncKeyState('A') & 0x8000) { tempMove.x -= 1.0f; move = true; }
			if (GetAsyncKeyState('D') & 0x8000) { tempMove.x += 1.0f; move = true; }
			if (GetAsyncKeyState('Q') & 0x8000) { tempMove.y -= 1.0f; move = true; }
			if (GetAsyncKeyState('E') & 0x8000) { tempMove.y += 1.0f; move = true; }

			float speed = sp->GetCurrentVelocityLen();

			auto& client = Client::GetInstance();

			// update speed if key down
			if (move) {
				XMVECTOR vec = XMLoadFloat3(&tempMove);
				XMFLOAT4X4 tpRot = tr->GetLocalTransform();
				tpRot._41 = 0.0f;
				tpRot._42 = 0.0f;
				tpRot._43 = 0.0f;

				vec = XMVector3Transform(vec, XMLoadFloat4x4(&tpRot));
				
				XMStoreFloat3(&tempMove, vec);
				sp->AddVelocity(tempMove, deltaTime);
			}

			// mouse
			//if (InputManager::GetInstance().GetDrag()) 
			{
				const auto& mouseMove = InputManager::GetInstance().GetMouseMove();
				XMFLOAT3 rot = tr->GetRotation();
				const float rootSpeed = 10.0f;
				rot.y += (mouseMove.x / rootSpeed);
				rot.x += (mouseMove.y / rootSpeed);
				tr->SetRotation(rot);
			}

			};

		std::function<void(Transform*, TestInput*, Speed*)> inputFunc2 = [deltaTime](Transform* tr, TestInput* in, Speed* sp) {
			// keyboard input
			bool move = false;
			XMFLOAT3 tempMove = { 0.0f, 0.0f, 0.0f };
			if (GetAsyncKeyState(VK_UP) & 0x8000)		{ tempMove.z += 1.0f; move = true; }
			if (GetAsyncKeyState(VK_DOWN) & 0x8000)		{ tempMove.z -= 1.0f; move = true; }
			if (GetAsyncKeyState(VK_LEFT) & 0x8000)		{ tempMove.x -= 1.0f; move = true; }
			if (GetAsyncKeyState(VK_RIGHT) & 0x8000)	{ tempMove.x += 1.0f; move = true; }

			float speed = sp->GetCurrentVelocityLen();

			// update speed if key down
			if (move) {
				XMVECTOR vec = XMLoadFloat3(&tempMove);
				XMFLOAT4X4 tpRot = tr->GetWorldTransform();
				tpRot._41 = 0.0f;
				tpRot._42 = 0.0f;
				tpRot._43 = 0.0f;

				vec = XMVector3Transform(vec, XMLoadFloat4x4(&tpRot));

				XMStoreFloat3(&tempMove, vec);
				sp->AddVelocity(tempMove, deltaTime);
			}


			//XMVECTOR vec = XMLoadFloat3(&tempMove) * speed * deltaTime;

			//XMFLOAT4X4 tpRot = tr->GetWorldTransform();
			//tpRot._41 = 0.0f;
			//tpRot._42 = 0.0f;
			//tpRot._43 = 0.0f;

			//vec = XMVector3Transform(vec, XMLoadFloat4x4(&tpRot));
			//XMStoreFloat3(&tempMove, vec);


			//if (move)
			//	sp->SetForce(tempMove);

			////DebugPrint(std::format("speed: {}", speed));
			//if (speed != 0)
			//	tr->SetPosition(Vector3::Add(sp->GetForce(), tr->GetPosition()));

			};
		// mouse

		// move by speed
		std::function<void(Transform*, Speed*)> move = [deltaTime](Transform* tr, Speed* sp) {
			XMVECTOR pos = XMLoadFloat3(&tr->GetPosition());
			XMVECTOR vel= XMLoadFloat3(&sp->GetVelocity());

			XMFLOAT3 temp;
			pos += vel * deltaTime;
			XMStoreFloat3(&temp, pos);

			tr->SetPosition(temp);
			};


		// send
		std::function<void(Transform*, Input*, Speed*)> send = [deltaTime](Transform* tr, Input* in, Speed* sp) {

			if(sp->GetCurrentVelocityLen() > 0 || InputManager::GetInstance().GetDrag())
				Client::GetInstance().Send_Pos(tr->GetPosition(), tr->GetRotation());
			};
		manager->Execute(inputFunc);
		manager->Execute(inputFunc2);
		manager->Execute(move);
		manager->Execute(send);
	}

	void ChangeAnimationTest::Update(ECSManager* manager, float deltaTime)
	{
		std::function<void(component::Speed*, component::AnimationController*, component::DiaAnimationControl*)> func1 = 
			[](component::Speed* sp, component::AnimationController* animCtrl, component::DiaAnimationControl* dia) {

			float speed = sp->GetCurrentVelocityLen();

			if (dia->m_BefSpeed > 10.0f) {
				if (speed <= 10.0f)
				animCtrl->ChangeAnimationTo("Idle");
			}
			else {
				if (speed >= 10.0f)
				animCtrl->ChangeAnimationTo("Walk");
			}

			if (GetAsyncKeyState(VK_END) & 0x0001) {
				animCtrl->ChangeAnimationTo("FallingDown");
				animCtrl->ChangeAnimationTo("GetUp");
				animCtrl->ChangeAnimationTo("Idle");
			}


			dia->m_BefSpeed = speed;

			};

		manager->Execute(func1);

	}

	void DayLight::Update(ECSManager* manager, float deltaTime)
	{
		std::function<void(component::Transform*, component::DayLight*, component::Light*)> func = 
			[deltaTime](component::Transform* transform, component::DayLight* dayLight, component::Light* light) {
			float rotSpeed = 360.0f / dayLight->GetDayCycle();

			XMFLOAT3 newRot = transform->GetRotation();
			newRot.x += rotSpeed * deltaTime;
			if (newRot.x > 360.0f) newRot.x = 0.0f;

			transform->SetRotation(newRot);

			dayLight->SetLightAngle(newRot.x);

			// change light color
			// weight == sin(angle.x)
			//XMFLOAT4 curLight = 

			// 0, 90, 180
			// 0   1   0
			// day time
			float weight = pow((sin(XMConvertToRadians(newRot.x))), 2);

			LightData& li = light->GetLightData();

			// day time
			if (newRot.x < 180.0f) {
				XMStoreFloat4(&li.m_LightColor,
					XMVectorLerp(
						XMLoadFloat4(&dayLight->GetSunSetLight()),
						XMLoadFloat4(&dayLight->GetNoonLight()),
						weight));
			}
			// night time
			else {
				XMStoreFloat4(&li.m_LightColor,
					XMVectorLerp(
						XMLoadFloat4(&dayLight->GetSunSetLight()),
						XMLoadFloat4(&dayLight->GetMoonLight()),
						weight));
			}



			//DebugPrint(std::format("angle : {}\tweight : {}", newRot.x, weight));

			};

		manager->Execute(func);
	}

	void SyncPosition::Update(ECSManager* manager, float deltaTime)
	{
		std::function<void(component::Server*, component::Name*, component::Transform*)> func = [](component::Server* server, component::Name* name, component::Transform* tr) {
			auto& client = Client::GetInstance();
			const SOCKET* playerSock = client.getPSock();
			if (playerSock[0])
			{
				auto& n = name->getName();
				if (server->getID() == NULL && n.compare("Player1") == 0)
					server->setID(playerSock[0]);
			}
			if (playerSock[1])
			{
				auto& n = name->getName();
				if (server->getID() == NULL && n.compare("Player2") == 0)
					server->setID(playerSock[1]);
			}
			// client의 1P, 2P 소켓 아이디 적용

			if(server->getID())
			{
				tr->SetPosition(client.characters[server->getID()].getPos());
				tr->SetRotation(client.characters[server->getID()].getRot());
			}

		};

		manager->Execute(func); 
	}

	//void SendToServer::Update(ECSManager* manager, float deltaTime)
	//{
	//	std::function<void(component::Transform*, component::Speed*)> func = [](component::Transform* tr, component::Speed* sp) {
	//		auto& client = Client::GetInstance();
	//		if (client.getRoomNum())
	//		{
	//			
	//		}
	//	};
	//	manager->Execute(func);
	//}
}
