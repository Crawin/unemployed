﻿#include "framework.h"
#include "ECS_System.h"
#include "ECS/Entity/Entity.h"
#include "ECS/Component/Component.h"
#include "ECS/ECSManager.h"
#include "ECS/ECSMacro.h"
#include "App/InputManager.h"
#include "Network/Client.h"
#include "App/Application.h"
#include "ECS/TimeLine/TimeLine.h"
#include "FMODsound/FmodSound.h"

#ifdef _DEBUG
#include "Scene/SceneManager.h"
#endif

namespace ECSsystem {

	void LocalToWorldTransform::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		std::function<void(/*Name*, */Transform*, SelfEntity*)> func = [&func, &manager](/*Name* name, */Transform* trans, SelfEntity* self) {
			Entity* ent = self->GetEntity();

			const std::list<Entity*>& children = ent->GetChildren();

			// build world matrix for child
			if (self->GetEntity()->GetParent() == nullptr)
				trans->SetParentTransform(Matrix4x4::Identity());
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
			
			XMFLOAT4X4 wordTrans = tr->GetWorldTransform();
			parent = XMLoadFloat4x4(&wordTrans);

			Entity* ent = self->GetEntity();

			const std::list<Entity*>& children = ent->GetChildren();

			for (Entity* child : children) {
				auto bit = child->GetBitset();
				int innerId = child->GetInnerID();

				Transform* childTrans = manager->GetComponent<Transform>(bit, innerId);
				if (childTrans != nullptr) {
					// do st trans to child
					// make world transform from this trans
					childTrans->SetParentTransform(tr->GetWorldTransform());

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
			XMFLOAT4X4 worldTr = tr->GetWorldTransform();
			XMStoreFloat4x4(&(cam->m_ViewMatrix), XMMatrixInverse(nullptr, XMLoadFloat4x4(&worldTr)));

			// sync states to
			cam->SyncActiveState();
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
		

		// sync with extra shader data
		std::function<void(component::DoorControl*, component::Renderer*)> func4 = [](component::DoorControl* door, component::Renderer* ren) {
			for (int i = 0; i < 2; ++i)
				ren->SetExtraShaderData(door->GetShaderData(i), i);
			};

		std::function<void(component::PlayerController*)> sendClientTalk = [manager](component::PlayerController* ctrl) {
			auto& client = Client::GetInstance();
			component::Pawn* curPawn = ctrl->GetControllingPawn();
			component::Transform* tr = manager->GetComponent<component::Transform>(curPawn->GetCameraEntity());

			if (client.is_talking)
			{
				auto pos = tr->GetWorldPosition();
				pos.y -= 160;
				cs_packet_sound_start sound(pos, SOUND_TYPE::VOICE);
				
				client.send_packet(&sound);
				//std::cout << pos.x << "," << pos.y << "," << pos.z << " 전송 완" << std::endl;
			}

			// fmod sync
			XMFLOAT3 rot = tr->GetRotation();
			FMOD_INFO::GetInstance().set_self_position(tr->GetWorldPosition());
			FMOD_INFO::GetInstance().set_player1_rotation_x(rot.x);
			//FMOD_INFO::GetInstance().set_player1_rotation_y(rot.y);
			//printf("%f 할당", rot.y);
			};

		manager->Execute(func1);
		manager->Execute(func2);
		manager->Execute(func3);
		manager->Execute(func4);
		manager->Execute(sendClientTalk);

	}

	void UpdateInput::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		POINT mouseMove = InputManager::GetInstance().GetMouseMove();

		// tick input
		std::function<void(PlayerController*)> tickAndUpdateInput = [&mouseMove, manager](PlayerController* ctrl) {
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

			// update inputs
			for (int i = SEND_SERVER_START; i < static_cast<int>(GAME_INPUT::GAME_INPUT_END); ++i) {
				GAME_INPUT input = static_cast<GAME_INPUT>(i);
				KEY_STATE state = curPawn->GetInputState(input);

				// if start or end press, go press
				if (state == KEY_STATE::START_PRESS || state == KEY_STATE::END_PRESS) {
					if (InputManager::GetInstance().GetMouseCapture())
					{
						cs_packet_key_input packet(state, input);
						Client::GetInstance().send_packet(&packet);
					}
				}
			}

#ifdef _DEBUG
			std::function<void(component::PlayerController*)> possessToOne = [manager](component::PlayerController* ctrl) { ctrl->Possess(manager, "Player1"); };
			std::function<void(component::PlayerController*)> possessToTwo = [manager](component::PlayerController* ctrl) { ctrl->Possess(manager, "Player2"); };

			if (curPawn->GetInputState(GAME_INPUT::F4) == KEY_STATE::END_PRESS) manager->Execute(possessToOne);
			if (curPawn->GetInputState(GAME_INPUT::F5) == KEY_STATE::END_PRESS) manager->Execute(possessToTwo);

			if (curPawn->GetInputState(GAME_INPUT::F1) == KEY_STATE::END_PRESS) {
				std::function<void(component::DayLightManager*, component::SelfEntity*)> changeTime = [manager](component::DayLightManager* dayManager, component::SelfEntity* ent) {

					PlayerController* ctrler = nullptr;
					std::function<void(PlayerController*)> getCtrler = [&ctrler](PlayerController* control) { ctrler = control; };
					manager->Execute(getCtrler);

					Pawn* controlledPawn = ctrler->GetControllingPawn();

					Pawn* changeingPawn = nullptr;
					std::function<void(Pawn*, Name*)> getChangePawn = [&changeingPawn](Pawn* pawn, Name* name) { if(name->getName() == "ChangeTimePawn") changeingPawn = pawn; };
					manager->Execute(getChangePawn);
					
					// possess to camera
					ctrler->Possess(changeingPawn, manager);

					// end event
					std::function returnToPawn = [ctrler, controlledPawn, manager]() {ctrler->Possess(controlledPawn, manager); };

					TimeLine<float>* changeTime = new TimeLine<float>(dayManager->GetCurTimePtr());
					changeTime->AddKeyFrame(dayManager->GetCurTime(), 0);
					changeTime->AddKeyFrame(22.0f, 1);
					changeTime->AddKeyFrame(22.0f, 2);
					changeTime->SetEndEvent(returnToPawn);

					manager->AddTimeLine(changeTime);
					};
				manager->Execute(changeTime);

				// hide students
				std::function<void(component::AI*, component::SelfEntity*)> disable = [manager](component::AI* ai, component::SelfEntity* self) {
					if (ai->GetType() == 0) manager->SetEntityState(self->GetEntity(), false);
					};
				manager->Execute(disable);

				// set player position to original
				std::function<void(component::Player*, component::Transform*)> movePlayers = [manager](component::Player* pl, component::Transform* tr) {
					tr->SetPosition(pl->GetOriginalPosition());
					tr->SetRotation(pl->GetOriginalRotate());
					};
				manager->Execute(movePlayers);

			}

			// to st
			if (curPawn->GetInputState(GAME_INPUT::F2) == KEY_STATE::END_PRESS) {
				// 
				Application::GetInstance().GetSceneManager()->StartGame();
			}
#endif

			};

		// input으로 pysics 업데이트 - player
		std::function<void(Transform*, Pawn*, Physics*, SelfEntity*, Player*, AnimationController*)> inputFunc = 
			[deltaTime, &mouseMove, manager](Transform* tr, Pawn* pawn, Physics* sp, SelfEntity* self, Player* pl, AnimationController* ctrl) {
			
			if (pawn->IsActive() == false) return;

			Entity* ent = self->GetEntity();

			// keyboard input
			XMFLOAT3 tempMove = { 0.0f, 0.0f, 0.0f };
			bool move = false;
			
			// if sitting, cant move
			if (pl->IsSitting() == false) {
				if (pawn->IsPressing(GAME_INPUT::FORWARD)) { tempMove.z += 1.0f; move = true; }
				if (pawn->IsPressing(GAME_INPUT::BACKWARD)) { tempMove.z -= 1.0f; move = true; }
				if (pawn->IsPressing(GAME_INPUT::LEFT)) { tempMove.x -= 1.0f; move = true; }
				if (pawn->IsPressing(GAME_INPUT::RIGHT)) { tempMove.x += 1.0f; move = true; }

				if (pawn->GetInputState(GAME_INPUT::SPACE_BAR) == KEY_STATE::START_PRESS)
				{ 
					if (pl->IsAir() == false) {
						XMFLOAT3 temp = sp->GetVelocity();
						sp->SetVelocity(XMFLOAT3(temp.x, temp.y + 400.0f, temp.z));

						pl->SetJumping(true);
						ctrl->ChangeAnimationTo(ANIMATION_STATE::JUMP_START);
						cs_packet_anim_type anim(ANIMATION_STATE::JUMP_START);
						Client::GetInstance().send_packet(&anim);
					}
				}
			}

			sp->SetDash(false);
			if (pawn->IsPressing(GAME_INPUT::SHIFT)) sp->SetDash(true);

			float speed = sp->GetCurrentVelocityLenOnXZ();
			FMOD_INFO::GetInstance().set_self_speed(speed);

			// update speed if key down
			if (move) {
				XMFLOAT3 velOnXZ = sp->GetVelocityOnXZ();
				XMVECTOR vel = XMLoadFloat3(&velOnXZ);

				float maxSpeed = sp->GetMaxVelocity();
				if (sp->IsDashing()) maxSpeed += sp->GetDashSpeed();
				
				float curSpeed = XMVectorGetX(XMVector3Length(vel));
				
				XMVECTOR vec = XMLoadFloat3(&tempMove);
				XMFLOAT4X4 tpRot = tr->GetLocalTransform();
				tpRot._41 = 0.0f;
				tpRot._42 = 0.0f;
				tpRot._43 = 0.0f;

				vec = XMVector3Transform(vec, XMLoadFloat4x4(&tpRot));

				XMStoreFloat3(&tempMove, vec);
				sp->AddVelocity(tempMove, deltaTime);

				XMFLOAT3 velocity = sp->GetVelocity();
				XMFLOAT3 velocityOnXZ = sp->GetVelocityOnXZ();
				vel = XMLoadFloat3(&velocityOnXZ);
				curSpeed = XMVectorGetX(XMVector3Length(vel));

				// limit max speed
				if (curSpeed >= maxSpeed)
				{
					vel  = vel / curSpeed * maxSpeed;
					velocity.x = XMVectorGetX(vel);
					velocity.z = XMVectorGetZ(vel);
					sp->SetVelocity(velocity);
				}
			}

			// mouse
			//if (InputManager::GetInstance().GetDrag()) 
			{
				XMFLOAT3 rot = tr->GetRotation();
				const float rootSpeed = 10.0f;
				rot.y += (mouseMove.x / rootSpeed);
				//rot.x += (mouseMove.y / rootSpeed);
				tr->SetRotation(rot);
				FMOD_INFO::GetInstance().set_player1_rotation_y(rot.y);
			}
		};

		// mouse - cameras
		std::function<void(PlayerController*)> mouseInput = [deltaTime, manager](PlayerController* ctrl) {
			// todo rotate must not be orbit
			Pawn* curPawn = ctrl->GetControllingPawn();

			Entity* curCam = curPawn->GetCameraEntity();

			Transform* camTr = manager->GetComponent<Transform>(curCam);
			Camera* cam = manager->GetComponent<Camera>(curCam);

			auto& mouseMove = curPawn->GetMouseMove();

			// rotate on x
			XMFLOAT3 rot = camTr->GetRotation();
			const float rootSpeed = 10.0f;
			//rot.y += (mouseMove.x / rootSpeed);
			rot.x += (mouseMove.y / rootSpeed);
			camTr->SetRotation(rot);
			//FMOD_INFO::GetInstance().set_player1_rotation_x(rot.x);

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
			// if self && ui state
			if (InputManager::GetInstance().IsUIState() == true && pawn->GetControlServer() == false) return;

			// holding change
			for (int i = 0; i < MAX_INVENTORY; ++i) {
				int start = static_cast<int>(GAME_INPUT::NUM_1);

				GAME_INPUT key = static_cast<GAME_INPUT>(i + start);
				if (pawn->GetInputState(key) == KEY_STATE::START_PRESS) {
					inven->ChangeHoldingItem(i, manager);
					break;
				}
			}

			// go with holding action
			Entity* holding = inven->GetCurrentHoldingItem();
			if (holding == nullptr) return;

			Holdable* holdComp = manager->GetComponent<Holdable>(holding);

			if (holdComp == nullptr) {
				Name* name = manager->GetComponent<Name>(holding);
				DebugPrint(std::format("ERROR!! no holdable component on this entity, name: {}", name->getName()));
				return;
			}

			// action
			auto& maps = holdComp->GetActionMap();
			//auto& inputState = pawn->GetInputState();

			for (auto& [actionKey, actionFunc] : maps) 
				if (actionKey.GetState() == pawn->GetInputState(actionKey.GetInput()))
					actionFunc(deltaTime);
			

			};

		manager->Execute(tickAndUpdateInput);
		
		// if ui state, no more update
		if (InputManager::GetInstance().IsUIState() == false) 
		{
			manager->Execute(inputFunc);
			manager->Execute(mouseInput);
			manager->Execute(attachinput);
		}
		manager->Execute(pawnInvenAction);
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
		float time = 12.0f;

		std::function<void(component::DayLightManager*)> updateAndGetTime = [deltaTime, &time](component::DayLightManager* dayManager) {
			dayManager->TimeAdd(deltaTime);
			time = dayManager->GetCurTime();
			};

		manager->Execute(updateAndGetTime);


		std::function<void(component::Transform*, component::DayLight*, component::Light*)> func = 
			[time](component::Transform* transform, component::DayLight* dayLight, component::Light* light) {
			
			XMFLOAT3 newRot = dayLight->GetOriginRotate();
			newRot.x += time * (360.0f / 24.0f);

			if (newRot.x > 360.0f) newRot.x -= 360.0f;

			transform->SetRotation(newRot);

			dayLight->SetLightAngle(newRot.x);

			// change light color
			// weight == sin(angle.x)
			//XMFLOAT4 curLight = 

			// 0, 90, 180
			// 0   1   0
			// day time
			float ambientWeight = sin(XMConvertToRadians(newRot.x));
			float weight = powf(ambientWeight, 2);

			ambientWeight = (ambientWeight + 1.0f) / 2.0f;

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
			XMStoreFloat4(&li.m_LightAmbient,
					XMVectorLerp(
						XMLoadFloat4(&dayLight->GetMinAmbient()),
						XMLoadFloat4(&dayLight->GetMaxAmbient()),
						ambientWeight));


			//DebugPrint(std::format("angle : {}\tweight : {}", newRot.x, weight));

			};

		manager->Execute(func);
	}

	void SyncPosition::Update(ECSManager* manager, float deltaTime)
	{
		/*std::function<void(component::Server*, component::Name*, component::Transform*, component::Physics*)> func = []
		(component::Server* server, component::Name* name, component::Transform* tr, component::Physics* sp) {
			auto& client = Client::GetInstance();
			const SOCKET* playerSock = client.getPSock();
			short type = client.getCharType();
			auto& n = name->getName();

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

				sp->SetVelocity(client.characters[id].getSpeed());
				if (id != playerSock[0]) {	// 상대방 플레이어면 회전값 적용
					DirectX::XMFLOAT3 pos = client.characters[id].getPos();
					DirectX::XMFLOAT3 rot = client.characters[id].getRot();
					tr->SetPosition(pos);
					tr->SetRotation(rot);
					if (id == 1)	// 가드면 발자국 소리 위치 업데이트를 위해 FMOD_INFO에 위치 업데이트
					{
						auto& fmod = FMOD_INFO::GetInstance();
						fmod.set_guard_position(pos);
						fmod.set_guard_speed(sp->GetCurrentVelocityLenOnXZ());
					}
				}

				//XMFLOAT3 vel = sp->GetVelocityOnXZ();
				//if (id != 1) {
				//}
				//DebugPrint(std::format("name: {},\tspeed: {}", n, XMVectorGetX(XMVector3Length(XMLoadFloat3(&vel)))));
				//XMVECTOR dif = XMLoadFloat3(&pos) - XMLoadFloat3(&tr->GetPosition());
				//XMStoreFloat3(&pos, dif);
			}

		};


		manager->Execute(func);*/
	}

	void CollideHandle::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// sync box first
		std::function<void(Transform*, Collider*)> syncStatic = [](Transform* tr, Collider* col) {
			XMFLOAT4X4 transformTr = tr->GetWorldTransform();
			XMMATRIX trans = XMLoadFloat4x4(&transformTr);

			// reset collider;
			col->UpdateCollidedList();
			col->SetCollided(false);

			col->UpdateBoundingBox(trans);
			};

		std::function<void(Transform*, DynamicCollider*)> syncDynamic = [](Transform* tr, DynamicCollider* col) {
			XMFLOAT4X4 transformTr = tr->GetWorldTransform();
			XMMATRIX trans = XMLoadFloat4x4(&transformTr);

			// reset collider;
			col->UpdateCollidedList();
			col->SetCollided(false);

			col->UpdateBoundingBox(trans);
			};


		auto circleBoxCol = CheckCollisionRectCircle;


		// collide check
		std::function<void(DynamicCollider*, SelfEntity*)> dynamicWithStatic =
			[&circleBoxCol, manager](DynamicCollider* a, SelfEntity* aEnt) {
			if (a->IsActive() == false) return;

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

			if (a->IsActive() == false || b->IsActive() == false) return;

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
		std::function<void(DynamicCollider*, Physics*, Transform*, SelfEntity*)> collideHandle =
			[&faces, deltaTime, manager](DynamicCollider* col, Physics* sp, Transform* tr, SelfEntity* self) {
			if (col->GetCollided() == false || col->IsTrigger()) return;

			auto& colVec = col->GetCollidedEntitiesList();
			auto& myBox = col->GetBoundingBox();
			// 
			XMFLOAT3 movedCenter = myBox.Center;
			movedCenter.y += col->GetPossibleClimb();
			XMVECTOR boxCenter = XMLoadFloat3(&movedCenter);
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

				// if player
				Player* player = manager->GetComponent<Player>(self->GetEntity());
				if (player != nullptr) {
					XMVECTOR up{ 0,1,0 };
					float dot = XMVectorGetX(XMVector3Dot(up, hitFace));
					float angle = XMConvertToDegrees(acos(dot));


					player->SetOnGround(false);

					// if hit angle degree smaller than 40, it's land
					if (angle < 40.0f && player->IsAir() == true) {
						player->SetJumping(false);
						player->SetOnGround(true);

						AnimationController* ctrl = manager->GetComponent<AnimationController>(self->GetEntity());
						ctrl->ChangeAnimationTo(ANIMATION_STATE::JUMP_LAND);
					}
				}

				//DebugPrint(std::format("hit face: {}", idx));
				//DebugPrint(std::format("\t{}, {}, {}", XMVectorGetX(hitFace), XMVectorGetY(hitFace), XMVectorGetZ(hitFace)));
				//DebugPrint(std::format("\tvelocity: {}, {}, {}", XMVectorGetX(velocity), XMVectorGetY(velocity), XMVectorGetZ(velocity)));
				//DebugPrint(std::format("\tafterhit: {}, {}, {}", XMVectorGetX(result), XMVectorGetY(result), XMVectorGetZ(result)));
			}
			};

		// set players to falling if they are on air
		std::function<void(Physics*, Player*, AnimationController*)> goFall = [](Physics* py, Player* pl, AnimationController* ctrl) {
			float ySpeed = py->GetVelocity().y;

			// if should start falling

			if (pl->IsAir() == false && ySpeed < -150.0f) {
				pl->SetJumping(true);

				ctrl->ChangeAnimationTo(ANIMATION_STATE::JUMP_ING);
			}


			//// landing
			//if (pl->IsAir() == true && ySpeed >= -15.0f) {
			//	pl->SetJumping(false);

			//	ctrl->ChangeAnimationTo(ANIMATION_STATE::JUMP_LAND);
			//}


			};


		manager->Execute(syncStatic);
		manager->Execute(syncDynamic);
		
		manager->Execute(dynamicWithStatic);
		manager->ExecuteSquare<component::DynamicCollider, component::SelfEntity>(dynamicWithDynamic);

		manager->Execute(handleEventStatic);
		manager->Execute(handleEventDynamic);

		manager->Execute(collideHandle);

		manager->Execute(goFall);
	}

	void SimulatePhysics::Update(ECSManager* manager, float deltaTime)
	{
		using namespace component;

		// friction on xz
		std::function<void(Physics*)> friction = [deltaTime](Physics* sp) {
			if (sp->IsToCalculate() == false || sp->IsFrictionActive() == false) return;

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

		manager->Execute(friction);
		manager->Execute(gravity);


		// play walk sound here
		std::function<void(Transform*, Physics*, Name*, SelfEntity*)> playWalkSound = [manager](Transform* tr, Physics* py, Name* name, SelfEntity* self) {
			Player* p = manager->GetComponent<Player>(self->GetEntity());
			AI* a = manager->GetComponent<AI>(self->GetEntity());

			// physics를 가진 엔티티 중, ai거나 player일 때만 사운드 재생
			if (a != nullptr || p != nullptr) {
				SOUND_TYPE type = SOUND_TYPE::FOOTPRINT;

				// todo if speed is fast
				// play run sound
				float vel = py->GetCurrentVelocityLenOnXZ();
				if (vel > 10.0f) {
					// just walk
					// set play speed
					// walk: 200, speed: 1.0f
					// run : 600, speed : 1.5f
					float soundPitch = 1.0f + (vel - 200.0f) * (600.0f - 200.0f) / (600.0f - 200.0f);
					soundPitch = std::clamp(soundPitch, 1.0f, 1.5f);
					//DebugPrint(std::format("pitch: {}", soundPitch));
					FMOD_INFO::GetInstance().play_loop_sound(tr->GetWorldPosition(), type, std::format("{}_footprint", name->getName()), soundPitch);

					// if dashing
					if (p != nullptr && py->IsDashing()) {
						cs_packet_sound_start packet(tr->GetWorldPosition(), SOUND_TYPE::FOOTPRINT);
						Client::GetInstance().send_packet(&packet);
					}

				}
				else
					FMOD_INFO::GetInstance().stop_sound(std::format("{}_footprint", name->getName()));
			}

			};

		manager->Execute(playWalkSound);

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

		POINT mousePos = InputManager::GetInstance().GetMouseCurrentPosition();
		ScreenToClient(Application::GetInstance().GethWnd(), &mousePos);

		std::function<void(Button*, UITransform*, SelfEntity*)> checkButtonPos = [controlledPawn, &mousePos](Button* but, UITransform* trans, SelfEntity* self) {
			SIZE center = trans->GetCenter();
			SIZE size = trans->GetSize();

			RECT rect = {
				center.cx - size.cx / 2,
				center.cy - size.cy / 2,
				center.cx + size.cx / 2,
				center.cy + size.cy / 2,
			};

			if (PtInRect(&rect, mousePos)) {
				KEY_STATE leftMouseState = controlledPawn->GetInputState(GAME_INPUT::MOUSE_LEFT);

				if (but->IsContain(leftMouseState))
					but->GetButtonEvent(leftMouseState)(self->GetEntity());
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
				if (sp->GetCurrentVelocityLen() > 0 || pawn->IsPressing(GAME_INPUT::MOUSE_LEFT)) {
					XMFLOAT3 pos = tr->GetWorldPosition();
					XMFLOAT3 rot = tr->GetRotation();
					XMFLOAT3 vel = sp->GetVelocity();

					Client::GetInstance().Send_Pos(pos, rot, vel, deltaTime);
				}
			}
			};

		manager->Execute(send);


		std::function<void(Server*, Transform*, Physics*)> sendNotPawn = [deltaTime](Server* serv, Transform* tr, Physics* py) {
			if (serv->IsSendMode() == false) return;

			if (py->GetCurrentVelocityLen() > 0) {
				XMFLOAT3 pos = tr->GetPosition();
				XMFLOAT3 rot = tr->GetRotation();
				XMFLOAT3 vel = py->GetVelocity();

				auto id = serv->getID();
				cs_packet_position packet(id, pos, rot, vel);
				Client::GetInstance().send_packet(&packet);
			}
			};

		manager->Execute(sendNotPawn);

	}

	void TimeLineManaging::Update(ECSManager* manager, float deltaTime)
	{
		for (auto& timeline : m_TimeLines) {
			timeline->Update(deltaTime);
			timeline->SyncData();
		}

		for (auto iter = m_TimeLines.begin(); iter != m_TimeLines.end();) {
			if ((*iter)->IsPlaying() == false) {
				(*iter)->RunEndEvent();
				iter = m_TimeLines.erase(iter);
			}
			else
				++iter;
		}
	}

	void ParticleManaging::Update(ECSManager* manager, float deltaTime)
	{
		for (auto particle : m_Particles)
			particle.second->Tick(deltaTime);
	}

	void ParticleManaging::InsertParticle(PARTICLE_TYPES type, XMFLOAT3 pos, XMFLOAT3 vel, int toInsert, float randRange)
	{
		m_Particles[type]->InsertParticle(pos, vel, toInsert, randRange);
	}

	void ParticleManaging::SyncParticle()
	{
		for (auto particle : m_Particles)
			particle.second->SyncParticle();
	}

	void ParticleEmitting::Update(ECSManager* manager, float deltaTime)
	{
		std::function<void(component::ParticleEmitter*, component::Transform*, component::Physics*)> func = [manager, deltaTime]
		(component::ParticleEmitter* pe, component::Transform* tr, component::Physics* py) {
			if (pe->IsActive() == false) return;

			if (pe->Tick(deltaTime)) 
				manager->AddParticle(pe->GetType(), tr->GetWorldPosition(), py->GetVelocity(), pe->GetEmitteMount(), pe->GetRandom());
			};

		manager->Execute(func);
	}
}
