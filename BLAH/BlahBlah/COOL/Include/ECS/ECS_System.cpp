#include "framework.h"
#include "ECS_System.h"
#include "Entity.h"
#include "Component.h"
#include "ECSManager.h"
#include "App/InputManager.h"
#include "Network/Client.h"
#include "ECSMacro.h"
#include "App/Application.h"

namespace ECSsystem {

	void LocalToWorldTransform::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		std::function<void(/*Name*, */Transform*, Children*)> func = [&func, &manager](/*Name* name, */Transform* trans, Children* childComp) {
			Entity* ent = childComp->GetEntity();

			const std::vector<Entity*>& children = ent->GetChildren();

			// build world matrix for child
			XMFLOAT4X4 parentMatrix = trans->GetWorldTransform();

			for (Entity* child : children) {
				auto bit = child->GetBitset();
				int innerId = child->GetInnerID();

				Transform* childTrans = manager->GetComponent<Transform>(bit, innerId);
				if (childTrans != nullptr) {
					//XMFLOAT4X4 myTransform = childTrans->GetLocalTransform();
					//XMStoreFloat4x4(&temp, XMLoadFloat4x4(&myTransform) * XMLoadFloat4x4(&parentMatrix));

					// do st trans to child
					// make world transform from this trans
					childTrans->SetParentTransform(parentMatrix);

					// execute this func to child
					manager->ExecuteFromEntity(bit, innerId, func);
				}
				else DebugPrint("ERROR!! no transform ");
			}
			};

		manager->ExecuteRoot(func);

		// sync with Attach
		std::function<void(component::Transform*, component::Attach*, component::SelfEntity*)> attacheSync = [&func, manager](component::Transform* tr, component::Attach* at, SelfEntity* self) {
			//XMMATRIX boneInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&at->GetBone()));
			XMMATRIX attachedMat = (at->GetAnimatedBone());
			XMMATRIX parent = XMLoadFloat4x4(&tr->GetParentTransfrom());
			parent = attachedMat * parent;

			XMFLOAT4X4 temp;
			XMStoreFloat4x4(&temp, parent);

			tr->SetParentTransform(temp);
			
			Entity* ent = self->GetEntity();

			const std::vector<Entity*>& children = ent->GetChildren();

			for (Entity* child : children) {
				auto bit = child->GetBitset();
				int innerId = child->GetInnerID();

				Transform* childTrans = manager->GetComponent<Transform>(bit, innerId);
				if (childTrans != nullptr) {
					XMFLOAT4X4 myTransform = childTrans->GetLocalTransform();
					XMStoreFloat4x4(&temp, XMLoadFloat4x4(&myTransform) * parent);

					// do st trans to child
					// make world transform from this trans
					childTrans->SetParentTransform(temp);

					// execute this func to child
					manager->ExecuteFromEntity(bit, innerId, func);
				}
				else DebugPrint("ERROR!! no transform ");
			}

			};

		manager->Execute(attacheSync);

	}

	void AnimationPlayTimeAdd::Update(ECSManager* manager, float deltaTime)
	{
		std::function<void(component::AnimationController*)> func = [manager, deltaTime](component::AnimationController* anim) {
			anim->UpdateTime(deltaTime);
			};

		manager->Execute(func);
	}

	void SyncWithTransform::OnInit(ECSManager* manager)
	{
		std::function<void(component::Transform*, component::Attach*)> func = [manager](component::Transform* tr, component::Attach* at) {
			at->SetOriginalPosition(tr->GetPosition());
			at->SetOriginalRotation(tr->GetRotation());
			at->SetOriginalScale(tr->GetScale());

			};

		manager->Execute(func);
	}

	void SyncWithTransform::Update(ECSManager* manager, float deltaTime)
	{
		// sync with camera
		// first person cam
		std::function<void(component::Transform*, component::Camera*)> func1 = [](component::Transform* tr, component::Camera* cam) {
			XMStoreFloat4x4(&(cam->m_ViewMatrix), XMMatrixInverse(nullptr, XMLoadFloat4x4(&tr->GetWorldTransform())));

			};
		
		// sync with renderer world matrix
		std::function<void(component::Transform*, component::Renderer*)> func2 = [](component::Transform* tr, component::Renderer* ren) {
			ren->SetWorldMatrix(tr->GetWorldTransform());

			};

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
		


		
		manager->Execute(func1);
		manager->Execute(func2);
		manager->Execute(func3);

	}

	void UpdateInput::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// input으로 pysics 업데이트
		std::function<void(Transform*, Input*, Physics*)> inputFunc = [deltaTime](Transform* tr, Input* in, Physics* sp) {
			// keyboard input
			XMFLOAT3 tempMove = { 0.0f, 0.0f, 0.0f };
			bool move = false;
			if (GetAsyncKeyState('W') & 0x8000) { tempMove.z += 1.0f; move = true; }
			if (GetAsyncKeyState('S') & 0x8000) { tempMove.z -= 1.0f; move = true; }
			if (GetAsyncKeyState('A') & 0x8000) { tempMove.x -= 1.0f; move = true; }
			if (GetAsyncKeyState('D') & 0x8000) { tempMove.x += 1.0f; move = true; }
			//if (GetAsyncKeyState('Q') & 0x8000) { tempMove.y -= 1.0f; move = true; }

			if (GetAsyncKeyState(VK_SPACE) & 0x0001) 
			{ 
				XMFLOAT3 temp = sp->GetVelocity();
				sp->SetVelocity(XMFLOAT3(temp.x, temp.y + 400.0f, temp.z));
			}

			float maxSpeed = 200.0f;
			if (GetAsyncKeyState(VK_SHIFT) * 0x8000) maxSpeed += 400.0f;
			sp->SetMaxSpeed(maxSpeed);

			float speed = sp->GetCurrentVelocityLenOnXZ();

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
				//rot.x += (mouseMove.y / rootSpeed);
				tr->SetRotation(rot);
			}

			};

		std::function<void(Transform*, TestInput*, Physics*)> inputFunc2 = [deltaTime](Transform* tr, TestInput* in, Physics* sp) {
			// keyboard input
			bool move = false;
			XMFLOAT3 tempMove = { 0.0f, 0.0f, 0.0f };
			if (GetAsyncKeyState(VK_UP) & 0x8000)		{ tempMove.z += 1.0f; move = true; }
			if (GetAsyncKeyState(VK_DOWN) & 0x8000)		{ tempMove.z -= 1.0f; move = true; }
			if (GetAsyncKeyState(VK_LEFT) & 0x8000)		{ tempMove.x -= 1.0f; move = true; }
			if (GetAsyncKeyState(VK_RIGHT) & 0x8000)	{ tempMove.x += 1.0f; move = true; }

			float speed = sp->GetCurrentVelocityLenOnXZ();

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
		std::function<void(Transform*, Camera*)> mouseInput = [deltaTime](Transform* tr, Camera* cam) {
			if (cam->m_IsMainCamera == false) return;

			// todo rotate must not be orbit

			const auto& mouseMove = InputManager::GetInstance().GetMouseMove();
			XMFLOAT3 rot = tr->GetRotation();
			const float rootSpeed = 10.0f;
			//rot.y += (mouseMove.x / rootSpeed);
			rot.x += (mouseMove.y / rootSpeed);
			tr->SetRotation(rot);
			};

		manager->Execute(inputFunc);
		manager->Execute(inputFunc2);
		manager->Execute(mouseInput);
	}

	void ChangeAnimationTest::OnInit(ECSManager* manager)
	{
		// do init (make state machine)
		
		using namespace component;

		// dia
		std::function<void(AnimationController*, DiaAnimationControl*)> diaBuildGraph =
			[](AnimationController* ctrl, DiaAnimationControl* dia) {
			
			using COND = std::function<bool(void*)>;

			// make conditions
			COND idle = [](void* data) {
				float* sp = reinterpret_cast<float*>(data);
				return *sp < 30.0f;
				};

			COND walk = [](void* data) {
				float* sp = reinterpret_cast<float*>(data);
				return *sp >= 30.0f;
			};

			COND hit = [](void* data) {
				return GetAsyncKeyState(VK_END) & 0x8000;
				};

			COND falldown = [ctrl](void* data) {
				return ctrl->GetCurrentPlayTime() - ctrl->GetCurrentPlayEndTime() > 2.0f;
				};

			COND getup = [ctrl](void* data) {
				return ctrl->GetCurrentPlayTime() - ctrl->GetCurrentPlayEndTime() > 0.0f;
				};

			// insert transition (graph)
			ctrl->InsertCondition(ANIMATION_STATE::IDLE, ANIMATION_STATE::WALK, walk);
			ctrl->InsertCondition(ANIMATION_STATE::IDLE, ANIMATION_STATE::FALLINGDOWN, hit);
			ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::IDLE, idle);
			ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::FALLINGDOWN, hit);
			ctrl->InsertCondition(ANIMATION_STATE::FALLINGDOWN, ANIMATION_STATE::GETUP, falldown);
			ctrl->InsertCondition(ANIMATION_STATE::GETUP, ANIMATION_STATE::IDLE, getup);

			// set start animation
			ctrl->ChangeAnimationTo(ANIMATION_STATE::IDLE);

			};

		// PlayerAnimControll
		std::function<void(AnimationController*, PlayerAnimControll*)> playerbuildGraph =
			[](AnimationController* ctrl, PlayerAnimControll* ply) {

			using COND = std::function<bool(void*)>;

			// make conditions
			COND idle = [](void* data) {
				float* sp = reinterpret_cast<float*>(data);
				return *sp < 30.0f;
				};

			COND idleToWalk = [](void* data) {
				float* sp = reinterpret_cast<float*>(data);
				return *sp >= 30.0f;
				};

			COND walkToRun = [](void* data) {
				float* sp = reinterpret_cast<float*>(data);
				return *sp >= 300.0f;
				};

			COND runToWalk = [](void* data) {
				float* sp = reinterpret_cast<float*>(data);
				return *sp < 300.0f;
				};

			COND hit = [](void* data) {
				return GetAsyncKeyState(VK_END) & 0x8000;
				};

			COND falldown = [ctrl](void* data) {
				return ctrl->GetCurrentPlayTime() - ctrl->GetCurrentPlayEndTime() > 2.0f;
				};

			COND getup = [ctrl](void* data) {
				return ctrl->GetCurrentPlayTime() - ctrl->GetCurrentPlayEndTime() > 0.0f;
				};

			// insert transition (graph)
			ctrl->InsertCondition(ANIMATION_STATE::IDLE, ANIMATION_STATE::WALK, idleToWalk);
			ctrl->InsertCondition(ANIMATION_STATE::IDLE, ANIMATION_STATE::FALLINGDOWN, hit);
			ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::IDLE, idle);
			ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::FALLINGDOWN, hit);
			ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::RUN, walkToRun);
			ctrl->InsertCondition(ANIMATION_STATE::RUN, ANIMATION_STATE::WALK, runToWalk);
			ctrl->InsertCondition(ANIMATION_STATE::RUN, ANIMATION_STATE::FALLINGDOWN, hit);
			ctrl->InsertCondition(ANIMATION_STATE::FALLINGDOWN, ANIMATION_STATE::GETUP, falldown);
			ctrl->InsertCondition(ANIMATION_STATE::GETUP, ANIMATION_STATE::IDLE, getup);

			// set start animation
			ctrl->ChangeAnimationTo(ANIMATION_STATE::IDLE);
			ctrl->ChangeAnimationTo(ANIMATION_STATE::IDLE);

			};

		manager->Execute(diaBuildGraph);
		manager->Execute(playerbuildGraph);

	}

	void ChangeAnimationTest::Update(ECSManager* manager, float deltaTime)
	{
		std::function<void(component::Physics*, component::AnimationController*, component::DiaAnimationControl*)> func1 =
			[](component::Physics* sp, component::AnimationController* animCtrl, component::DiaAnimationControl* dia) {

			float speed = sp->GetCurrentVelocityLenOnXZ();

			animCtrl->CheckTransition(&speed);
			};

		std::function<void(component::Physics*, component::AnimationController*, component::PlayerAnimControll*)> func2 =
			[](component::Physics* sp, component::AnimationController* animCtrl, component::PlayerAnimControll* play) {

			float speed = sp->GetCurrentVelocityLenOnXZ();

			animCtrl->CheckTransition(&speed);
			};

		manager->Execute(func1);
		manager->Execute(func2);

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
		std::function<void(component::Server*, component::Name*, component::Transform*, component::Physics*)> func = []
		(component::Server* server, component::Name* name, component::Transform* tr, component::Physics* sp) {
			auto& client = Client::GetInstance();
			const SOCKET* playerSock = client.getPSock();
			short type = client.getCharType();
			auto& n = name->getName();
			if (playerSock[0])
			{
				if (server->getID() == NULL && n.compare("Player1") == 0)
					server->setID(playerSock[0]);
			}
			if (playerSock[1])
			{
				if (server->getID() == NULL && n.compare("Player2") == 0)
					server->setID(playerSock[1]);
			}

			//if (playerSock[0])				// 클라 본인의 캐릭터가 할당되었을 때
			//{
			//	switch (type)				// 클라 본인이 호스트인가 게스트인가?
			//	{
			//	case 0:
			//		// 아직 방 생성 전
			//		break;
			//	case 1:						// 호스트
			//		if (server->getID() == NULL && n.compare("Player1") == 0)
			//			server->setID(playerSock[0]);
			//		break;
			//	case 2:						// 게스트
			//		if (server->getID() == NULL && n.compare("Player2") == 0)
			//		{
			//			server->setID(playerSock[0]);
			//		}
			//		break;
			//	default:
			//		std::cout << "클라이언트 주인의 캐릭터 타입 오류" << std::endl;
			//		while (1);
			//		break;
			//	}
			//}
			//if (playerSock[1])				// 상대편의 클라가 할당되었을 때
			//{
			//	switch (type)				// 클라 본인이 호스트인가 게스트인가?
			//	{
			//	case 1:						// 클라 본인이 호스트이므로, 상대편 클라는 게스트로 할당
			//		if (server->getID() == NULL && n.compare("Player2") == 0)
			//			server->setID(playerSock[1]);
			//		break;
			//	case 2:						// 클라 본인이 게스트이므로, 상대편 클라는 호스트로 할당
			//		if (server->getID() == NULL && n.compare("Player1") == 0)
			//		{
			//			server->setID(playerSock[1]);
			//		}
			//		break;
			//	default:
			//		std::cout << "클라이언트 주인의 캐릭터 타입 오류" << std::endl;
			//		while (1);
			//		break;
			//	}
			//}
			// client의 1P, 2P 소켓 아이디 적용

			auto id = server->getID();
			if (id && client.characters[id].IsUpdated())
			{
				XMFLOAT3 pos = tr->GetPosition();
				//DebugPrint(std::format("befPos: {}, {}, {}", pos.x, pos.y, pos.z));

				client.characters[id].SetUpdate(false);
				tr->SetPosition(client.characters[id].getPos());
				if (id != playerSock[0])	// 상대방 플레이어면 회전값 적용
					tr->SetRotation(client.characters[id].getRot());
				sp->SetVelocity(client.characters[id].getSpeed());

				XMVECTOR dif = XMLoadFloat3(&pos) - XMLoadFloat3(&tr->GetPosition());
				XMStoreFloat3(&pos, dif);
				DebugPrint(std::format("diff: {}, {}, {}", pos.x, pos.y, pos.z));
			}

		};

		manager->Execute(func); 
	}

	void CollideHandle::OnInit(ECSManager* manager)
	{
		std::function<void(component::Collider*)> func = [](component::Collider* col) {
			col->ResetList();
			};

		manager->Execute(func);

		// make collide events here!!

		// todo
		// 지금은 player를 Input으로 표현하지만 추후에 ControlPlayer같은게 생기면 대체해야한다.

		// input <-> door
		INSERT_COLLIDE_EVENT(manager, component::Input, component::DoorControl, COLLIDE_EVENT_TYPE::BEGIN, [manager](Entity* self, Entity* other) {
			auto door = manager->GetComponent<component::DoorControl>(other);
			auto player = manager->GetComponent<component::Input>(self);
			if (door && player)
				player->SetInteractionEntity(other);
			});

		INSERT_COLLIDE_EVENT(manager, component::Input, component::DoorControl, COLLIDE_EVENT_TYPE::ING, [manager](Entity* self, Entity* other) {
			//DebugPrint("ING");
			});

		INSERT_COLLIDE_EVENT(manager, component::Input, component::DoorControl, COLLIDE_EVENT_TYPE::END, [manager](Entity* self, Entity* other) {
			auto door = manager->GetComponent<component::DoorControl>(other);
			auto player = manager->GetComponent<component::Input>(self);
			if (door && player)
				player->SetInteractionEntity(nullptr);
			});
	}

	void CollideHandle::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// sync box first
		std::function<void(Transform*, Collider*)> sync = [](Transform* tr, Collider* col) {
			XMMATRIX trans = XMLoadFloat4x4(&tr->GetWorldTransform());

			// reset collider;
			col->UpdateCollidedList();
			col->SetCollided(false);

			col->UpdateBoundingBox(trans);
			};


		auto circleBoxCol = CheckCollisionRectCircle;

		// collide check
		std::function<void(Collider*, SelfEntity*, Collider*, SelfEntity*)> collideCheck =
			[circleBoxCol](Collider* a, SelfEntity* aEnt, Collider* b, SelfEntity* bEnt) {

			// if both static, skip
			if (a->IsStaticObject() && b->IsStaticObject()) return;

			auto& boxA = a->GetBoundingBox();
			auto& boxB = b->GetBoundingBox();

			XMVECTOR acualLen = XMVector3Length(XMLoadFloat3(&boxA.Center) - XMLoadFloat3(&boxB.Center));
			XMVECTOR maximumLen = XMVector3Length(XMLoadFloat3(&boxA.Extents) + XMLoadFloat3(&boxB.Extents));

			float dist = XMVectorGetX(maximumLen - acualLen);

			if (dist > 0 && boxA.Intersects(boxB))
			{
				// if capsule, check
				if (a->IsCapsule()) {
					XMVECTOR circleCenter = XMVector3Transform(
						XMLoadFloat3(&boxA.Center), 
						XMMatrixInverse(nullptr, XMMatrixRotationQuaternion(XMLoadFloat4(&boxB.Orientation))));

					if (false == circleBoxCol(
						XMFLOAT2(boxB.Center.x, boxB.Center.z),
						XMFLOAT2(boxB.Extents.x, boxB.Extents.z), 
						XMFLOAT2(XMVectorGetX(circleCenter), XMVectorGetZ(circleCenter)), 
						boxA.Extents.x)) return;
				}
				if (b->IsCapsule()) {
					XMVECTOR circleCenter = XMVector3Transform(
						XMLoadFloat3(&boxB.Center),
						XMMatrixInverse(nullptr, XMMatrixRotationQuaternion(XMLoadFloat4(&boxA.Orientation))));

					if (false == circleBoxCol(
						XMFLOAT2(boxA.Center.x, boxA.Center.z),
						XMFLOAT2(boxA.Extents.x, boxA.Extents.z),
						XMFLOAT2(XMVectorGetX(circleCenter), XMVectorGetZ(circleCenter)),
						boxB.Extents.x)) return;
				}

				// if collided, add to coll list, handle later
				if (a->IsStaticObject() == false)
					a->InsertCollidedEntity(bEnt->GetEntity());
				if (b->IsStaticObject() == false) 
					b->InsertCollidedEntity(aEnt->GetEntity());

				a->SetCollided(true);
				b->SetCollided(true);
			}

			};

		XMVECTOR faces[6] = {
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f },
			{ -1.0f, 0.0f, 0.0f },
			{ 0.0f, -1.0f, 0.0f },
			{ 0.0f, 0.0f, -1.0f },
		};

		// collision events
		std::function<void(Collider*, SelfEntity*)> handleEvent = [](Collider* col, SelfEntity* self) {
			auto& colVec = col->GetCollidedEntitiesList();

			for (auto& otherEntity : colVec) {
				Entity* other = otherEntity.m_Entity;
				auto eventType = otherEntity.m_Type;

				auto& eventMap = col->GetEventMap(eventType);

				// execute event function here
				for (const auto& [bitset, func] : eventMap) 
					if ((other->GetBitset() & bitset) == bitset) func(self->GetEntity(), other);
			}
			};

		// handle collide (move back, reduce speed)
		std::function<void(Collider*, Physics*, Transform*)> collideHandle =
			[&faces, deltaTime, manager](Collider* col, Physics* sp, Transform* tr) {
			if (col->GetCollided() == false || col->IsTrigger()) return;

			auto& colVec = col->GetCollidedEntitiesList();
			auto& myBox = col->GetBoundingBox();
			// 
			XMVECTOR boxCenter = XMLoadFloat3(&myBox.Center);
			for (auto& otherEntity : colVec) {
				Collider* other = manager->GetComponent<Collider>(otherEntity.m_Entity);
				if (other == nullptr) {
					DebugPrint("ERROR!! no collider found");
					continue;
				}
				
				if (other->IsTrigger()) continue;

				auto& otherBox = other->GetBoundingBox();
				XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&otherBox.Orientation));

				XMVECTOR hitFace;
				float max = -10.0f;
				int idx = 0;

				for (int i = 0; i < _countof(faces); ++i) {
					XMVECTOR faceNormal = XMVector3Transform(faces[i], rot);
					XMVECTOR faceCenterPos = faces[i] * XMLoadFloat3(&otherBox.Extents) + XMLoadFloat3(&otherBox.Center);
					XMVECTOR lay = XMVector3Normalize(boxCenter - faceCenterPos);

					// dot and max is hit face
					float dot = XMVectorGetX(XMVector3Dot(faceNormal, lay));
					if (dot > max) {
						max = dot;
						hitFace = faceNormal;
						idx = i;
					}
				}

				XMVECTOR velocity = XMLoadFloat3(&sp->GetVelocity());
				float dot = XMVectorGetX(XMVector3Dot(velocity, hitFace));
				if (dot > 0) continue;

				// reduce velocity here
				XMVECTOR velocityBack = dot * hitFace;// *sp->GetElasticity();
				XMVECTOR result = velocity - velocityBack;

				XMFLOAT3 res;
				XMStoreFloat3(&res, result);
				sp->SetVelocity(res);

				// move back before hit
				XMVECTOR posBack = velocityBack * deltaTime * 2.0f;
				XMVECTOR newPos = XMLoadFloat3(&tr->GetPosition()) - posBack;

				XMFLOAT3 backedPos;
				XMStoreFloat3(&backedPos, newPos);
				tr->SetPosition(backedPos);


				//DebugPrint(std::format("hit face: {}", idx));
				//DebugPrint(std::format("\t{}, {}, {}", XMVectorGetX(hitFace), XMVectorGetY(hitFace), XMVectorGetZ(hitFace)));
				//DebugPrint(std::format("\tvelocity: {}, {}, {}", XMVectorGetX(velocity), XMVectorGetY(velocity), XMVectorGetZ(velocity)));
				//DebugPrint(std::format("\tafterhit: {}, {}, {}", XMVectorGetX(result), XMVectorGetY(result), XMVectorGetZ(result)));
			}
			};

		manager->Execute(sync);
		manager->ExecuteSquare<component::Collider, component::SelfEntity>(collideCheck);
		manager->Execute(handleEvent);
		manager->Execute(collideHandle);
	}

	void SimulatePhysics::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// limit max speed
		std::function<void(Physics*)> spLimit = [](Physics* py) {
			XMVECTOR vel = XMLoadFloat3(&py->GetVelocityOnXZ());

			float maxSpeed = py->GetMaxVelocity();
			float curSpeed = XMVectorGetX(XMVector3Length(vel));

			if (curSpeed > maxSpeed)
			{
				vel *= maxSpeed / curSpeed;
				XMFLOAT3 t;
				XMStoreFloat3(&t, vel);
				py->SetVelocityOnXZ(t);
			}

			};


		// friction on xz
		std::function<void(Physics*)> friction = [deltaTime](Physics* sp) {
			const float friction = 1100.0f;
			// if moving
			float speed = sp->GetCurrentVelocityLenOnXZ();// sp->GetCurrentVelocity();

			if (speed > 0) {
				// do friction here
				XMFLOAT3 velocity = sp->GetVelocityOnXZ();
				XMVECTOR vel = XMLoadFloat3(&velocity);
				XMVECTOR nor = XMVector3Normalize(vel);
				XMVECTOR res = vel - nor * friction * deltaTime;

				XMFLOAT3 temp;
				XMStoreFloat3(&temp, res);

				// 부호가 바뀜
				if (XMVectorGetX(vel) * XMVectorGetX(res) < 0) temp.x = 0;
				//if (XMVectorGetY(vel) * XMVectorGetY(res) < 0) temp.y = 0;
				if (XMVectorGetZ(vel) * XMVectorGetZ(res) < 0) temp.z = 0;

				// update
				sp->SetVelocityOnXZ(temp);
				speed = sp->GetCurrentVelocityLenOnXZ();

				if (abs(speed) < 0.01f) {
					sp->SetVelocityOnXZ({ 0,0,0 });
				}
			}


			};

		// gravity
		std::function<void(Physics*)> gravity = [deltaTime](Physics* py) {
			// todo
			// if is not static
			XMVECTOR gravity = { 0.0, -980.0f, 0.0f };
			gravity *= deltaTime;

			XMVECTOR vel = XMLoadFloat3(&py->GetVelocity()) + gravity;
			XMFLOAT3 temp;

			XMStoreFloat3(&temp, vel);

			py->SetVelocity(temp);
			};

		manager->Execute(spLimit);
		manager->Execute(friction);
		manager->Execute(gravity);

	}

	void MoveByPhysics::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// move by speed
		std::function<void(Transform*, Physics*)> move = [deltaTime](Transform* tr, Physics* sp) {
			XMVECTOR pos = XMLoadFloat3(&tr->GetPosition());
			XMVECTOR vel = XMLoadFloat3(&sp->GetVelocity());
			
			XMFLOAT3 temp;
			pos += vel * deltaTime;
			XMStoreFloat3(&temp, pos);

			tr->SetPosition(temp);
			};

		// send
		std::function<void(Transform*, Input*, Physics*)> send = [deltaTime](Transform* tr, Input* in, Physics* sp) {
			auto& client = Client::GetInstance();
			if (client.getRoomNum())
			{
				if (sp->GetCurrentVelocityLen() > 0 || InputManager::GetInstance().GetDrag())
					Client::GetInstance().Send_Pos(tr->GetPosition(), tr->GetRotation(), sp->GetVelocity(), deltaTime);
			}
			};

		manager->Execute(move);
		manager->Execute(send);
	}

	void HandleInteraction::OnInit(ECSManager* manager)
	{
		// build interaction function
		InteractionFuncion withDoor = [manager](Entity* player, Entity* door) {
			DebugPrint("Interaction, DOOR!!");

			// if opened
			component::DoorControl* doorCtrl = manager->GetComponent<component::DoorControl>(door);
			auto trans = manager->GetComponent<component::Transform>(door);
			if (doorCtrl != nullptr && doorCtrl->IsLocked() == false) {
				auto rot = trans->GetRotation();
				rot.y -= 90.0f;
				trans->SetRotation(rot);
			}
			else {
				// todo
				// door에 연결된 미니게임이 무엇인지 정보를 찾고
				// 키패드인지 패턴인지 등등을 확인한 후
				// 정답에 대한 정보를 넣어준다. set answer
				// 정답에 대한 정보는 door가 가지고 있다. set door
				// KeyPadUI는 확인 버튼이 눌렸을 시 전달받은 정보를 토대로 통과 여부를 판단

				std::function<void(component::UICanvas*, component::UIKeypad*)> openUI = [door](component::UICanvas* can, component::UIKeypad* kpd) {
					DebugPrint("UI SHOW!!");

					// test
					kpd->SetAnswer(123);
					kpd->SetDoor(door);

					can->ShowUI();
					};

				manager->Execute(openUI);
			}
			};

		SET_INTERACTION_EVENT(manager, component::DoorControl, withDoor);
		

	}

	void HandleInteraction::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// interaction
		std::function<void(Input*, SelfEntity*)> interactionFunc = [manager](Input* in, SelfEntity* self) {
			Entity* interactionEntity = nullptr;

			if (GetAsyncKeyState('E') & 0x0001) interactionEntity = in->GetInteractionEntity();
			if (interactionEntity == nullptr) return;

			Collider* col = manager->GetComponent<Collider>(interactionEntity);
			if (col == nullptr) return;

			auto& interactionFunc = col->GetInteractionFunction();
			if (interactionFunc == nullptr) return;

			interactionFunc(self->GetEntity(), interactionEntity);
			};

		manager->Execute(interactionFunc);




	}

	void HandleUIComponent::OnInit(ECSManager* manager)
	{
		using namespace component;
		std::function<void(UICanvas*, UIKeypad*, Children*)> setUIButtonCallbackFunc = [manager](UICanvas* can, UIKeypad* kpd, Children* childComp) {
			Entity* self = childComp->GetEntity();
			auto& children = self->GetChildren();

			for (Entity* child : children) {
				Name* childName = manager->GetComponent<Name>(child);
				// 확인 버튼
				if (childName != nullptr) {
					std::string name = childName->getName();

					if (name == "Check") {
						Button* button = manager->GetComponent<Button>(child);
						ButtonEventFunction check = [can, kpd, manager](Entity* ent) {
							// hide ui;
							can->HideUI();

							// open door
							Entity* door = kpd->GetDoor();
							DoorControl* doorCtrl = manager->GetComponent<DoorControl>(door);
							doorCtrl->SetLock(false);

							};
						button->SetButtonEvent(check);
					}
				}
			}

			};

		manager->Execute(setUIButtonCallbackFunc);
	}

	void HandleUIComponent::Update(ECSManager* manager, float deltaTime)
	{
		// check the button is clicked!

		using namespace component;

		std::function<void(Button*, UITransform*, SelfEntity*)> checkButtonPos = [](Button* but, UITransform* trans, SelfEntity* self) {
			POINT mousePos =  InputManager::GetInstance().GetMouseCurrentPosition();
			ScreenToClient(Application::GetInstance().GethWnd(), &mousePos);


			SIZE center = trans->GetCenter();
			SIZE size = trans->GetSize();

			RECT rect = {
				center.cx - size.cx / 2,
				center.cy - size.cy / 2,
				center.cx + size.cx / 2,
				center.cy + size.cy / 2,
			};

			if (PtInRect(&rect, mousePos) && InputManager::GetInstance().IsMouseLeftDown()) {

				const ButtonEventFunction& butEvent = but->GetButtonEvent();
				if (butEvent != nullptr) {
					butEvent(self->GetEntity());
					DebugPrint("Button Hit!!");


				}
			}

			};

		std::function<void(UICanvas*, Children*)> forAliveCanvas = [manager, &checkButtonPos](UICanvas* canvas, Children* childComp) {
			if (canvas->IsActive() == false) return;

			Entity* self = childComp->GetEntity();
			auto& children = self->GetChildren();

			for (Entity* child : children) 
				manager->ExecuteFromEntity(child->GetBitset(), child->GetInnerID(), checkButtonPos);

			};

		manager->Execute(forAliveCanvas);

	}
}
