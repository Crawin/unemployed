#include "framework.h"
#include "ECS_System.h"
#include "Entity.h"
#include "Component.h"
#include "ECSManager.h"
#include "App/InputManager.h"

namespace ECSsystem {

	void LocalToWorldTransform::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;
		
		std::function<void(Transform*, Children*)> func = [&func, &manager](Transform* trans, Children* childComp) {
			Entity* ent = childComp->GetEntity();

			const std::vector<Entity*>& children = ent->GetChildren();

			// build world matrix for child
			XMFLOAT4X4 temp = trans->GetWorldTransform();

			for (Entity* child : children) {
				auto bit = child->GetBitset();
				int innerId = child->GetInnerID();

				Transform* childTrans = manager->GetComponent<Transform>(bit, innerId);
				if (childTrans != nullptr) {
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

	void SyncWithTransform::Update(ECSManager* manager, float deltaTime)
	{
		// sync with camera
		// first person cam
		std::function<void(component::Transform*, component::Camera*)> func1 = [](component::Transform* tr, component::Camera* cam) {

			XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&(tr->GetRotation())));
			XMMATRIX trs = XMMatrixTranslationFromVector(XMLoadFloat3(&(tr->GetPosition())));

			XMStoreFloat4x4(&(cam->m_ViewMatrix), XMMatrixInverse(nullptr, rot * trs));
			};

		manager->Execute(func1);

		std::function<void(component::Transform*, component::Renderer*)> func2 = [](component::Transform* tr, component::Renderer* ren) {
			ren->SetWorldMatrix(tr->GetWorldTransform());

			};

		manager->Execute(func2);

	}

	void Friction::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;
		std::function<void(Speed*)> func = [deltaTime](Speed* sp) {
			const float friction = 100.0f;
			// if moving
			if (abs(sp->GetCurrentVelocity()) > 0) {
				float slowdown = friction * deltaTime;

				if (sp->GetCurrentVelocity() > 0) {
					sp->SetCurrentSpeed(sp->GetCurrentVelocity() - slowdown);
					if (sp->GetCurrentVelocity() < 0) sp->SetCurrentSpeed(0);
				}
				else {
					sp->SetCurrentSpeed(sp->GetCurrentVelocity() - slowdown);
					if (sp->GetCurrentVelocity() > 0) sp->SetCurrentSpeed(0);
				}
			}
			};

		manager->Execute(func);
	}

	void MoveByInput::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;
		std::function<void(Transform*, Input*, Speed*)> func = [deltaTime](Transform* tr, Input* in, Speed* sp) {
			// keyboard input

			XMFLOAT3 tempMove = { 0.0f, 0.0f, 0.0f };
			if (GetAsyncKeyState('W') & 0x8000) tempMove.z += 1.0f;
			if (GetAsyncKeyState('S') & 0x8000) tempMove.z -= 1.0f;
			if (GetAsyncKeyState('A') & 0x8000) tempMove.x -= 1.0f;
			if (GetAsyncKeyState('D') & 0x8000) tempMove.x += 1.0f;
			if (GetAsyncKeyState('Q') & 0x8000) tempMove.y -= 1.0f;
			if (GetAsyncKeyState('E') & 0x8000) tempMove.y += 1.0f;

			// move to look at;
			XMVECTOR vec = XMLoadFloat3(&tempMove) * sp->GetMaxVelocity() * deltaTime;
			XMMATRIX rot = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&(tr->GetRotation())));

			vec = XMVector3Transform(vec, rot);
			XMStoreFloat3(&tempMove, vec);

			tr->SetPosition(Vector3::Add(tempMove, tr->GetPosition()));
			
			tempMove = tr->GetPosition();
			//DebugPrint(std::format("x: {}, y: {}, z: {}", tempMove.x, tempMove.y, tempMove.z));

			// mouse

			//if (InputManager::GetInstance().GetDrag()) 
			{
				const auto& mouseMove = InputManager::GetInstance().GetMouseMove();
				XMFLOAT3 rot = tr->GetRotation();
				const float rootSpeed = 500.0f;
				rot.y += mouseMove.x / rootSpeed;
				rot.x += mouseMove.y / rootSpeed;
				tr->SetRotation(rot);

				//DebugPrint(std::format("x: {}, y: {}", mouseMove.cx, mouseMove.cy));// , rot.z));
			}

			};

		// mouse


		manager->Execute(func);
	}

}
