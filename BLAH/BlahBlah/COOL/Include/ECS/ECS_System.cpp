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

		std::function<void(/*Name*, */Transform*, SelfEntity*)> func = [&func, &manager](/*Name* name, */Transform* trans, SelfEntity* self) {
			Entity* ent = self->GetEntity();

			const std::list<Entity*>& children = ent->GetChildren();

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
			
			parent = XMLoadFloat4x4(&tr->GetWorldTransform());

			Entity* ent = self->GetEntity();

			const std::list<Entity*>& children = ent->GetChildren();

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

	void SyncWithTransform::Update(ECSManager* manager, float deltaTime)
	{
		// sync with camera
		// first person cam
		std::function<void(component::Transform*, component::Camera*)> func1 = [](component::Transform* tr, component::Camera* cam) {
			//XMMATRIX invWorld = XMMatrixInverse(nullptr, XMLoadFloat4x4(&tr->GetWorldTransform()));
			//XMVECTOR pos = XMLoadFloat3(&tr->GetWorldPosition());
			//XMVECTOR look = {XMVectorGetX(invWorld.r[2]), XMVectorGetY(invWorld.r[2]), XMVectorGetZ(invWorld.r[2]) };
			//XMVECTOR up = { 0,1,0 };
			//XMVECTOR right = XMVector3Cross(up, look);
			//up = XMVector3Cross(look, right);
			//
			//XMStoreFloat4x4(&(cam->m_ViewMatrix), XMMatrixLookToLH(pos, look, up));
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

			XMFLOAT4X4 world = tr->GetWorldTransform();
			world._41 = 0;
			world._42 = 0;
			world._43 = 0;

			XMMATRIX rot = XMLoadFloat4x4(&world);
			
			// light.m_Direction;
			light.m_Position = tr->GetWorldPosition();
			XMStoreFloat3(&light.m_Direction, XMVector3Normalize(XMVector3Transform(XMLoadFloat3(&direction), rot)));
			};
		


		
		manager->Execute(func1);
		manager->Execute(func2);
		manager->Execute(func3);

	}

	void UpdateInput::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		POINT mouseMove = InputManager::GetInstance().GetMouseMove();

		// tick input
		std::function<void(PlayerController*)> tickAndUpdateInput = [&mouseMove](PlayerController* ctrl) {
			Pawn* curPawn = ctrl->GetControllingPawn();
			
			// tick first
			curPawn->TickInput();

			// update inputs
			for (int i = 0; i < static_cast<int>(GAME_INPUT::GAME_INPUT_END); ++i) {
				GAME_INPUT input = static_cast<GAME_INPUT>(i);
				if (GetAsyncKeyState(ConvertGameInputEnumToKeyIntValue(input)) & 0x8000) 
					curPawn->PressInput(input);
			}

			// set mouse input
			curPawn->SetMouseMove(mouseMove);
			InputManager::GetInstance().ResetMouseMove();
			};

		// input으로 pysics 업데이트
		std::function<void(Transform*, Pawn*, Physics*)> inputFunc = [deltaTime, &mouseMove](Transform* tr, Pawn* pawn, Physics* sp) {
			// keyboard input
			XMFLOAT3 tempMove = { 0.0f, 0.0f, 0.0f };
			bool move = false;
			if (pawn->IsPressing(GAME_INPUT::FORWARD)) { 
				tempMove.z += 1.0f; move = true;
			}
			if (pawn->IsPressing(GAME_INPUT::BACKWARD)) { tempMove.z -= 1.0f; move = true; }
			if (pawn->IsPressing(GAME_INPUT::LEFT)) { tempMove.x -= 1.0f; move = true; }
			if (pawn->IsPressing(GAME_INPUT::RIGHT)) { tempMove.x += 1.0f; move = true; }

			if (pawn->GetInputState(GAME_INPUT::SPACE_BAR) == KEY_STATE::START_PRESS)
			{ 
				XMFLOAT3 temp = sp->GetVelocity();
				sp->SetVelocity(XMFLOAT3(temp.x, temp.y + 400.0f, temp.z));
			}

			float maxSpeed = 200.0f;
			if (pawn->IsPressing(GAME_INPUT::SHIFT)) maxSpeed += 400.0f;
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
				XMFLOAT3 rot = tr->GetRotation();
				const float rootSpeed = 10.0f;
				rot.y += (mouseMove.x / rootSpeed);
				//rot.x += (mouseMove.y / rootSpeed);
				tr->SetRotation(rot);
			}
		};

		// mouse - cameras
		std::function<void(Transform*, Camera*)> mouseInput = [deltaTime, &mouseMove](Transform* tr, Camera* cam) {
			if (cam->m_IsMainCamera == false) return;

			// todo rotate must not be orbit

			XMFLOAT3 rot = tr->GetRotation();
			const float rootSpeed = 10.0f;
			//rot.y += (mouseMove.x / rootSpeed);
			rot.x += (mouseMove.y / rootSpeed);
			tr->SetRotation(rot);
			};

		// mouse - attachInputs
		std::function<void(Transform*, AttachInput*)> attachinput = [deltaTime, &mouseMove](Transform* tr, AttachInput* cam) {
			// todo rotate must not be orbit

			XMFLOAT3 rot = tr->GetRotation();
			const float rootSpeed = 10.0f;
			//rot.y += (mouseMove.x / rootSpeed);
			rot.x += (mouseMove.y / rootSpeed);
			tr->SetRotation(rot);
			};

		//////////////////////////////////////////
		// Content
		//////////////////////////////////////////
		// Pawn Inventory Action
		std::function<void(Pawn*, Inventory*)> pawnInvenAction = [deltaTime, manager](Pawn* pawn, Inventory* inven) {
			Entity* holding = inven->GetCurrentHoldingItem();

			if (holding == nullptr) return;

			Holdable* holdComp = manager->GetComponent<Holdable>(holding);

			if (holdComp == nullptr) {
				Name* name = manager->GetComponent<Name>(holding);
				DebugPrint(std::format("ERROR!! no holdable component on this entity, name: {}", name->getName()));
				return;
			}

			auto& maps = holdComp->GetActionMap();
			//auto& inputState = pawn->GetInputState();

			for (auto& [actionKey, actionFunc] : maps) 
				if (actionKey.GetState() == pawn->GetInputState(actionKey.GetInput()))
					actionFunc(deltaTime);
			

			};

		manager->Execute(tickAndUpdateInput);
	
		// if ui state, no more update
		if (InputManager::GetInstance().IsUIState() == false) {
			manager->Execute(inputFunc);
			manager->Execute(mouseInput);
			manager->Execute(attachinput);
			manager->Execute(pawnInvenAction);
		}
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
				{
					server->setID(playerSock[0]);
				}
			}
			if (playerSock[1])
			{
				if (server->getID() == NULL && n.compare("Player2") == 0)
				{
					server->setID(playerSock[1]);
				}
			}
			else //상대방 플레이어가 로그아웃 했을때
			{
				if (n.compare("Player2") == 0 && server->getID() != NULL)
					server->setID(NULL);
			}
			if (n.compare("Guard") == 0 && server->getID() == NULL)
				server->setID(1);

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
				//if (id == 1)
				//{
					//auto speed = client.characters[id].getSpeed();
					//std::cout << speed.x << "," << speed.y << "," << speed.z << std::endl;
				//}
				XMFLOAT3 pos = tr->GetPosition();
				//DebugPrint(std::format("befPos: {}, {}, {}", pos.x, pos.y, pos.z));

				client.characters[id].SetUpdate(false);

				if (id != playerSock[0]) {	// 상대방 플레이어면 회전값 적용
					tr->SetPosition(client.characters[id].getPos());
					tr->SetRotation(client.characters[id].getRot());
				}
				sp->SetVelocity(client.characters[id].getSpeed());
				//XMFLOAT3 vel = sp->GetVelocityOnXZ();
				//if (id != 1) {
				//}
				//DebugPrint(std::format("name: {},\tspeed: {}", n, XMVectorGetX(XMVector3Length(XMLoadFloat3(&vel)))));
				//XMVECTOR dif = XMLoadFloat3(&pos) - XMLoadFloat3(&tr->GetPosition());
				//XMStoreFloat3(&pos, dif);
			}

		};

		manager->Execute(func);
	}

	void CollideHandle::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// sync box first
		std::function<void(Transform*, Collider*)> syncStatic = [](Transform* tr, Collider* col) {
			XMMATRIX trans = XMLoadFloat4x4(&tr->GetWorldTransform());

			// reset collider;
			col->UpdateCollidedList();
			col->SetCollided(false);

			col->UpdateBoundingBox(trans);
			};

		std::function<void(Transform*, DynamicCollider*)> syncDynamic = [](Transform* tr, DynamicCollider* col) {
			XMMATRIX trans = XMLoadFloat4x4(&tr->GetWorldTransform());

			// reset collider;
			col->UpdateCollidedList();
			col->SetCollided(false);

			col->UpdateBoundingBox(trans);
			};


		auto circleBoxCol = CheckCollisionRectCircle;


		// collide check
		std::function<void(DynamicCollider*, SelfEntity*)> dynamicWithStatic =
			[&circleBoxCol, manager](DynamicCollider* a, SelfEntity* aEnt) {
			auto& boxA = a->GetBoundingBox();
			auto& originBoxA = a->GetOriginalBox();

			std::function<void(Collider*, SelfEntity*)> check = [manager, a, &boxA, &originBoxA, aEnt, &circleBoxCol](Collider* b, SelfEntity* bEnt) {
				auto& boxB = b->GetBoundingBox();

				// check and insert box
				if (boxA.Intersects(boxB)) {
					// if capsule, check
					//if (a->IsCapsule()) {
					//	auto& originBoxB = b->GetOriginalBox();
					//	XMFLOAT3 temp;

					//	XMVECTOR 

					//	XMStoreFloat3(&temp, XMVector3Rotate(XMLoadFloat3(&boxA.Center), -XMLoadFloat4(&boxB.Orientation)));

					//	if (false == circleBoxCol(originBoxB, XMFLOAT2(temp.x, temp.z), boxA.Extents.x)) return;
					//}
					//if (b->IsCapsule()) {
					//	XMFLOAT3 temp;
					//	XMStoreFloat3(&temp, XMVector3Rotate(XMLoadFloat3(&boxB.Center), -XMLoadFloat4(&boxA.Orientation)));

					//	if (false == circleBoxCol(originBoxA, XMFLOAT2(temp.x, temp.z), boxB.Extents.x)) return;
					//}

					// if collided, add to coll list, handle later
					if (a->IsStaticObject() == false)
						a->InsertCollidedEntity(bEnt->GetEntity());
					if (b->IsStaticObject() == false)
						b->InsertCollidedEntity(aEnt->GetEntity());

					a->SetCollided(true);
					b->SetCollided(true);
				}

				};

			manager->Execute(check);
			};

		std::function<void(DynamicCollider*, SelfEntity*, DynamicCollider*, SelfEntity*)> dynamicWithDynamic =
			[&circleBoxCol](DynamicCollider* a, SelfEntity* aEnt, DynamicCollider* b, SelfEntity* bEnt) {

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
				//if (a->IsCapsule()) {
				//	auto& originBoxB = b->GetOriginalBox();
				//	XMFLOAT3 temp;
				//	XMStoreFloat3(&temp, XMVector3Rotate(XMLoadFloat3(&boxA.Center), -XMLoadFloat4(&boxB.Orientation)));

				//	if (false == circleBoxCol(originBoxB, XMFLOAT2(temp.x, temp.z), boxA.Extents.x)) return;
				//}
				//if (b->IsCapsule()) {
				//	auto& originBoxA = a->GetOriginalBox();
				//	XMFLOAT3 temp;
				//	XMStoreFloat3(&temp, XMVector3Rotate(XMLoadFloat3(&boxB.Center), -XMLoadFloat4(&boxA.Orientation)));

				//	if (false == circleBoxCol(originBoxA, XMFLOAT2(temp.x, temp.z), boxB.Extents.x)) return;
				//}

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
		std::function<void(Collider*, SelfEntity*)> handleEventStatic = [manager](Collider* col, SelfEntity* self) {
			auto& colVec = col->GetCollidedEntitiesList();

			for (auto& otherEntity : colVec) {
				Entity* other = otherEntity.m_Entity;
				auto eventType = otherEntity.m_Type;

				auto eventMap = col->GetEventMap(eventType);

				// execute event function 
				if (eventMap != nullptr)
					for (const auto& [bitset, func] : *eventMap) 
						if ((other->GetBitset() & bitset) == bitset) func(self->GetEntity(), other);
			}
			};

		std::function<void(DynamicCollider*, SelfEntity*)> handleEventDynamic = [manager](DynamicCollider* col, SelfEntity* self) {
			auto& colVec = col->GetCollidedEntitiesList();

			for (auto& otherEntity : colVec) {
				Entity* other = otherEntity.m_Entity;
				auto eventType = otherEntity.m_Type;

				auto eventMap = col->GetEventMap(eventType);

				// execute event function here
				if (eventMap != nullptr)
					for (const auto& [bitset, func] : *eventMap)
						if ((other->GetBitset() & bitset) == bitset) func(self->GetEntity(), other);
			}
			};

		// handle collide (move back, reduce speed)
		std::function<void(DynamicCollider*, Physics*, Transform*)> collideHandle =
			[&faces, deltaTime, manager](DynamicCollider* col, Physics* sp, Transform* tr) {
			if (col->GetCollided() == false || col->IsTrigger()) return;

			auto& colVec = col->GetCollidedEntitiesList();
			auto& myBox = col->GetBoundingBox();
			// 
			XMVECTOR boxCenter = XMLoadFloat3(&myBox.Center);
			for (auto& otherEntity : colVec) {
				Collider* other = manager->GetComponent<Collider>(otherEntity.m_Entity);
				if (other == nullptr) {
					// todo dynamic collider
					//DebugPrint("ERROR!! no collider found");
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
				XMVECTOR posBack = velocityBack * deltaTime;// *2.0f;
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

		manager->Execute(syncStatic);
		manager->Execute(syncDynamic);

		manager->Execute(dynamicWithStatic);
		manager->ExecuteSquare<component::DynamicCollider, component::SelfEntity>(dynamicWithDynamic);

		manager->Execute(handleEventStatic);
		manager->Execute(handleEventDynamic);

		manager->Execute(collideHandle);
	}

	void SimulatePhysics::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// limit max speed
		std::function<void(Physics*)> spLimit = [](Physics* py) {
			if (py->IsToCalculate() == false) return;

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
			if (sp->IsToCalculate() == false) return;

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
			if (py->IsToCalculate() == false) return;

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

		manager->Execute(move);
	}

	void HandleInteraction::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// interaction
		std::function<void(Pawn*, SelfEntity*)> interactionFunc = [manager](Pawn* pawn, SelfEntity* self) {
			Entity* interactionEntity = nullptr;

			// if press 'E'
			if (pawn->GetInputState(GAME_INPUT::INTERACTION) == KEY_STATE::START_PRESS) interactionEntity = interactionEntity = pawn->GetInteractionEntity();
			if (interactionEntity == nullptr) return;

			Interaction* interaction = manager->GetComponent<Interaction>(interactionEntity);

			auto interactionFunc = interaction->GetInteractionFunction();

			interactionFunc(self->GetEntity(), interactionEntity);
			};

		manager->Execute(interactionFunc);


	}

	void HandleUIComponent::Update(ECSManager* manager, float deltaTime)
	{
		// check the button is clicked!

		using namespace component;

		Pawn* controlledPawn = nullptr;

		std::function<void(PlayerController*)> getController = [&controlledPawn](PlayerController* control) {
			controlledPawn = control->GetControllingPawn();
			};
		manager->Execute(getController);

		std::function<void(Button*, UITransform*, SelfEntity*)> checkButtonPos = [controlledPawn](Button* but, UITransform* trans, SelfEntity* self) {
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

			if (PtInRect(&rect, mousePos) && controlledPawn->GetInputState(GAME_INPUT::MOUSE_LEFT) == KEY_STATE::END_PRESS) {

				const ButtonEventFunction& butEvent = but->GetButtonReleaseEvent();
				if (butEvent != nullptr) {
					butEvent(self->GetEntity());
					DebugPrint("Button Hit!!");


				}
			}

			};

		std::function<void(UICanvas*, SelfEntity*)> forAliveCanvas = [manager, &checkButtonPos](UICanvas* canvas, SelfEntity* selfEntity) {
			if (canvas->IsActive() == false) return;

			Entity* self = selfEntity->GetEntity();
			auto& children = self->GetChildren();

			for (Entity* child : children) 
				manager->ExecuteFromEntity(child->GetBitset(), child->GetInnerID(), checkButtonPos);

			};

		manager->Execute(forAliveCanvas);

	}

	void SendToServer::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;
		// send
		std::function<void(Transform*, Pawn*, Physics*)> send = [deltaTime](Transform* tr, Pawn* pawn, Physics* sp) {
			auto& client = Client::GetInstance();
			if (client.getRoomNum() && pawn->IsActive() == true)
			{
				if (sp->GetCurrentVelocityLen() > 0 || pawn->IsPressing(GAME_INPUT::MOUSE_LEFT))
					Client::GetInstance().Send_Pos(tr->GetPosition(), tr->GetRotation(), sp->GetVelocity(), deltaTime);
			}
			};

		manager->Execute(send);
	}
}
