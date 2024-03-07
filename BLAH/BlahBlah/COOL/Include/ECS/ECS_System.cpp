#include "framework.h"
#include "ECS_System.h"
#include "Component.h"
#include "ECSManager.h"

namespace ECSsystem {

	void SyncWithTransform::Update(ECSManager* manager, float deltaTime)
	{
		// sync with camera
		std::function<void(component::Transform*, component::Camera*)> func1 = [](component::Transform* tr, component::Camera* cam) {
			// build view matrix here

			// todo 지금은 position만 바꿔주지만, 회전 관련 정보들 또한 바꿔줘야 한다.
			cam->SetPosition(tr->GetPosition());
			//cam->ShowYourself();
			};

		manager->Execute(func1);

		std::function<void(component::Transform*, component::Renderer*)> func2 = [](component::Transform* tr, component::Renderer* ren) {
			// build world matrix here

			auto& s = tr->GetScale();
			auto& r = tr->GetRotation();
			auto& t = tr->GetPosition();
			
			XMMATRIX mat = XMMatrixMultiply(
				XMMatrixTranslation(t.x, t.y, t.z), XMMatrixMultiply(
					XMMatrixRotationQuaternion(XMLoadFloat4(&r)), 
					XMMatrixScaling(s.x, s.y, s.z)));

			XMFLOAT4X4 world;
			XMStoreFloat4x4(&world, mat);
			ren->SetWorldMatrix(world);

			};

		manager->Execute(func2);

	}

	void MoveByInput::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;
		std::function<void(Transform*, Input*)> func = [deltaTime](Transform* tr, Input* in) {
			XMFLOAT3 tempMove = { 0.0f, 0.0f, 0.0f };
			if (GetAsyncKeyState('W') & 0x8000) tempMove.z += 1.0f;
			if (GetAsyncKeyState('S') & 0x8000) tempMove.z -= 1.0f;
			if (GetAsyncKeyState('A') & 0x8000) tempMove.x -= 1.0f;
			if (GetAsyncKeyState('D') & 0x8000) tempMove.x += 1.0f;
			if (GetAsyncKeyState('Q') & 0x8000) tempMove.y -= 1.0f;
			if (GetAsyncKeyState('E') & 0x8000) tempMove.y += 1.0f;

			// 임시
			tempMove = Vector3::ScalarProduct(tempMove, deltaTime * 15.0f);

			tr->SetPosition(Vector3::Add(tempMove, tr->GetPosition()));
			
			tempMove = tr->GetPosition();
			//DebugPrint(std::format("x: {}, y: {}, z: {}", tempMove.x, tempMove.y, tempMove.z));
			};

		manager->Execute(func);
	}

}
