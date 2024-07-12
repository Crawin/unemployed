#include "framework.h"
#include "ContentComponents.h"
#include "Scene/ResourceManager.h"
#include "ECS/ECSManager.h"
#include "Animation/AnimationTrack.h"
#include "json/json.h"
#include "ECS/TimeLine/TimeLine.h"


namespace component {
	void PlayerAnimControll::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void PlayerAnimControll::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		AnimationController* ctrl = manager->GetComponent<AnimationController>(selfEntity);

		if (ctrl == nullptr) {
			const std::string& name = manager->GetComponent<Name>(selfEntity)->getName();
			ERROR_QUIT(std::format("ERROR!! no anim controller on this entity, name: {}", name));
		}

		// set update function
		AnimationPlayer* player = ctrl->GetPlayer();

		using COND = std::function<bool(void*)>;

		// make conditions
		COND idle = [](void* data) {
			float* sleed = reinterpret_cast<float*>(data);
			return *sleed < 30.0f;
			};

		COND idleToWalk = [](void* data) {
			float* sleed = reinterpret_cast<float*>(data);
			return *sleed >= 30.0f;
			};

		COND walkToRun = [](void* data) {
			float* sleed = reinterpret_cast<float*>(data);
			return *sleed >= 300.0f;
			};

		COND runToWalk = [](void* data) {
			float* sleed = reinterpret_cast<float*>(data);
			return *sleed < 300.0f;
			};

		COND hit = [](void* data) {
			return GetAsyncKeyState(VK_END) & 0x8000;
			};

		COND falldown = [ctrl](void* data) {
			return ctrl->GetCurrentPlayTime() - ctrl->GetCurrentPlayEndTime() > 2.0f;
			};

		COND endPlaying = [ctrl](void* data) {
			return ctrl->GetCurrentPlayTime() - ctrl->GetCurrentPlayEndTime() > 0.0f;
			};


		if (player->IsStateOwn(ANIMATION_STATE::BLENDED_MOVING_STATE)) {
			player->SetUpdateFunctionTo(ANIMATION_STATE::BLENDED_MOVING_STATE, [manager, selfEntity](float deltaTime, void* data) {
				Point2D* pt = reinterpret_cast<Point2D*>(data);

				Physics* py = manager->GetComponent<Physics>(selfEntity);
				Transform* tr = manager->GetComponent<Transform>(selfEntity);
				XMVECTOR lookDir{ 0, 0, 1, 0 };
				XMVECTOR up{ 0, 1, 0 };
				XMMATRIX world = XMLoadFloat4x4(&tr->GetWorldTransform());
				lookDir = XMVector3Normalize(XMVector4Transform(lookDir, world));

				XMVECTOR movingDir = XMVector3Normalize(XMLoadFloat3(&py->GetVelocity()));

				// 스칼라 삼중적
				float dot = std::clamp(XMVectorGetX(XMVector3Dot(lookDir, movingDir)), -1.0f, 1.0f);
				float angle = XMConvertToDegrees(acos(dot));
				float scalarTriPro = XMVectorGetX(XMVector3Dot(XMVector3Cross(lookDir, movingDir), up));
				if (scalarTriPro < 0)
					angle = 360.0f - angle;

				pt->m_Speed = py->GetCurrentVelocityLenOnXZ();
				pt->m_Angle = angle;

				if (IsZero(pt->m_Speed)) {
					pt->m_Speed = 0;
					pt->m_Angle = 0;
				}

				});

			ctrl->InsertCondition(ANIMATION_STATE::JUMP_START, ANIMATION_STATE::JUMP_ING, endPlaying);
			ctrl->InsertCondition(ANIMATION_STATE::JUMP_LAND, ANIMATION_STATE::BLENDED_MOVING_STATE, endPlaying);

			ctrl->InsertCondition(ANIMATION_STATE::SIT_START, ANIMATION_STATE::SIT_LOOP, endPlaying);
			ctrl->InsertCondition(ANIMATION_STATE::SIT_END, ANIMATION_STATE::BLENDED_MOVING_STATE, endPlaying);

			ctrl->ChangeAnimationTo(ANIMATION_STATE::BLENDED_MOVING_STATE);
			ctrl->ChangeAnimationTo(ANIMATION_STATE::BLENDED_MOVING_STATE);
		}

		else {
			// insert transition (graph)
			ctrl->InsertCondition(ANIMATION_STATE::IDLE, ANIMATION_STATE::WALK, idleToWalk);
			ctrl->InsertCondition(ANIMATION_STATE::IDLE, ANIMATION_STATE::GETTING_HIT, hit);
			ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::IDLE, idle);
			ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::GETTING_HIT, hit);
			ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::RUN, walkToRun);
			ctrl->InsertCondition(ANIMATION_STATE::RUN, ANIMATION_STATE::WALK, runToWalk);
			ctrl->InsertCondition(ANIMATION_STATE::RUN, ANIMATION_STATE::GETTING_HIT, hit);
			ctrl->InsertCondition(ANIMATION_STATE::GETTING_HIT, ANIMATION_STATE::GETUP, falldown);
			ctrl->InsertCondition(ANIMATION_STATE::GETUP, ANIMATION_STATE::IDLE, endPlaying);
			// set start animation
			//ctrl->ChangeAnimationTo(ANIMATION_STATE::IDLE);
			//ctrl->ChangeAnimationTo(ANIMATION_STATE::IDLE);
		}

	}

	void PlayerAnimControll::ShowYourself() const
	{
		DebugPrint("PlayerAnimControll Comp");
	}

	void DiaAnimationControl::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void DiaAnimationControl::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		AnimationController* ctrl = manager->GetComponent<AnimationController>(selfEntity);

		if (ctrl == nullptr) {
			const std::string& name = manager->GetComponent<Name>(selfEntity)->getName();
			ERROR_QUIT(std::format("ERROR!! no anim controller on this entity, name: {}", name));
		}

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
		ctrl->InsertCondition(ANIMATION_STATE::IDLE, ANIMATION_STATE::GETTING_HIT, hit);
		ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::IDLE, idle);
		ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::GETTING_HIT, hit);
		ctrl->InsertCondition(ANIMATION_STATE::GETTING_HIT, ANIMATION_STATE::GETUP, falldown);
		ctrl->InsertCondition(ANIMATION_STATE::GETUP, ANIMATION_STATE::IDLE, getup);

		// set start animation
		//ctrl->ChangeAnimationTo(ANIMATION_STATE::IDLE);
	}

	void DiaAnimationControl::ShowYourself() const
	{
		DebugPrint("DiaAnimationControl Comp");
	}

	void DoorControl::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value door = v["Door"];

		m_Answer = door["Answer"].asInt();
	}

	void DoorControl::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Interaction* interaction = manager->GetComponent<Interaction>(selfEntity);

		if (interaction == nullptr)
			ERROR_QUIT("ERROR!! no interaction component on current DoorControl entity");

		InteractionFuncion withDoor = [manager](Entity* player, Entity* door) {
			DebugPrint("Interaction, DOOR!!");

			// if opened
			component::DoorControl* doorCtrl = manager->GetComponent<component::DoorControl>(door);
			auto trans = manager->GetComponent<component::Transform>(door);
			if (doorCtrl != nullptr && doorCtrl->IsLocked() == false) {
				auto rot = trans->GetRotation();
				XMFLOAT3 rotAfter = rot;
				rotAfter.y -= 90.0f;
				TimeLine<XMFLOAT3>* openDoor = new TimeLine<XMFLOAT3>(trans->GetRotationPtr());
				openDoor->AddKeyFrame(rot, 0);
				openDoor->AddKeyFrame(rotAfter, 1);

				manager->AddTimeLine(door, openDoor);
			}
			else {
				// todo
				// door에 연결된 미니게임이 무엇인지 정보를 찾고
				// 키패드인지 패턴인지 등등을 확인한 후
				// 정답에 대한 정보를 넣어준다. set answer
				// 정답에 대한 정보는 door가 가지고 있다. set door
				// KeyPadUI는 확인 버튼이 눌렸을 시 전달받은 정보를 토대로 통과 여부를 판단

				std::function<void(component::UICanvas*, component::UIKeypad*)> openUI = [door, doorCtrl](component::UICanvas* can, component::UIKeypad* kpd) {
					DebugPrint("UI SHOW!!");

					// test
					kpd->SetAnswer(doorCtrl->GetAnswer());
					kpd->SetDoor(door);

					kpd->SetCurrent(0);

					can->ShowUI();
					};

				manager->Execute(openUI);
			}
			};

		interaction->SetInteractionFunction(withDoor);
	}

	void DoorControl::ShowYourself() const
	{
	}


	void UIKeypad::Create(Json::Value& v, ResourceManager* rm)
	{
		// nothing to do
	}

	void UIKeypad::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		// 여기서 자신의 자식들을 돌며 해당 버튼이 눌렸을 때 본인의 answer가 올라가게 만들어 두어야 함
		auto& children = selfEntity->GetChildren();

		UICanvas* canvas = manager->GetComponent<UICanvas>(selfEntity);
		UIKeypad* keypad = manager->GetComponent<UIKeypad>(selfEntity);

		if (canvas == nullptr || keypad == nullptr) 
			ERROR_QUIT("ERRIR!!!! no canvas or keypad on current entity");
		

		for (Entity* child : children) {
			Name* childName = manager->GetComponent<Name>(child);
			// 확인 버튼
			if (childName != nullptr) {
				std::string name = childName->getName();

				if (name == "Check") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction check = [canvas, keypad, manager](Entity* ent) {
						// hide ui;
						canvas->HideUI();
						DebugPrint("check");

						DebugPrint(std::format("pw: {}", keypad->GetCurrent()));
						if (keypad->GetAnswer() == keypad->GetCurrent()) {
							// open door
							Entity* door = keypad->GetDoor();
							DoorControl* doorCtrl = manager->GetComponent<DoorControl>(door);
							doorCtrl->SetLock(false);
						}

						};
					button->SetButtonDownEvent(KEY_STATE::END_PRESS, check);
				}
				else {
					for (int i = 1; i <= 9; ++i) {
						std::string buttonName = std::format("{}Button", i);
						if (name == buttonName) {
							Button* button = manager->GetComponent<Button>(child);
							ButtonEventFunction password = [canvas, keypad, manager, i](Entity* ent) { // 버튼에 대한 콜백함수 등록
								int current = keypad->GetCurrent();
								current = current * 10 + i;
								keypad->SetCurrent(current);
								};
							button->SetButtonDownEvent(KEY_STATE::END_PRESS, password);
						}
					}
				}
			}
		}

	}

	void UIKeypad::ShowYourself() const
	{
	}

	void Inventory::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value inven = v["Inventory"];

		m_InventorySocketName = inven["SocketName"].asString();

		for(int i = 0; i < MAX_INVENTORY; ++i)
			m_TargetEntityNames[i] = inven[std::format("Slot_{}", i)].asString().c_str();
	}

	void Inventory::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Name* childNameComp = manager->GetComponent<Name>(selfEntity);

		// 계층마다 Inventory 소켓의 위치가 다를 수도 있다.
		const char* playerCam = "PlayerCamera";
		const char* invenSocketName = "InventoryHand";

		Entity* entityTarget = selfEntity;

		size_t start = 0;
		size_t end = m_InventorySocketName.find('/');

		while (end != std::string::npos) {
			entityTarget = manager->GetEntityInChildren(m_InventorySocketName.substr(start, end - start), entityTarget);

			if (entityTarget == nullptr)
				ERROR_QUIT("ERROR!!! hierachy error");

			start = end + 1;
			end = m_InventorySocketName.find('/', start);
		}

		m_HoldingSocket = manager->GetEntityInChildren(m_InventorySocketName.substr(start), entityTarget);
		if (m_HoldingSocket == nullptr) 
			ERROR_QUIT("ERROR!! no holding hand in current child, inventory component error");
		
		// find targets here
		for (int i = 0; i < MAX_INVENTORY; ++i) {
			if (m_TargetEntityNames[i] != "") {
				Entity* target = manager->GetEntity(m_TargetEntityNames[i]);

				if (target != nullptr) {
					Holdable* hold = manager->GetComponent<Holdable>(target);
					if (hold == nullptr) ERROR_QUIT(std::format("ERROR!! no holdable component in this entity, name: {}", m_TargetEntityNames[i]));

					hold->SetMaster(selfEntity);
					m_Items[i] = target;
				}
			}
		}

		m_CurrentHolding = 0;
		if (m_Items[m_CurrentHolding] != nullptr) 
		{
			
			manager->AttachChild(m_HoldingSocket, m_Items[m_CurrentHolding]);
		}
	}

	void Inventory::ShowYourself() const
	{

	}

	bool Inventory::ChangeHoldingItem(int idx, ECSManager* manager)
	{
		// if same item, no chagne
		if (idx == m_CurrentHolding) return true;

		// attach bef item to origin parent;
		if (m_Items[m_CurrentHolding] != nullptr) {
			Holdable* curHold = manager->GetComponent<Holdable>(m_Items[m_CurrentHolding]);
			manager->AttachChild(curHold->GetOriginParent(), m_Items[m_CurrentHolding]);
		}

		m_CurrentHolding = idx;
		// if no item, just return
		if (m_Items[idx] == nullptr) {
			return true;
		}

		// attach new item to inven socket
		// dont have to nullcheck
		manager->AttachChild(m_HoldingSocket, m_Items[idx]);
		m_CurrentHolding = idx;

		return true;
	}

	void Holdable::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value hold = v["Holdable"];

	}

	void Holdable::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		m_OriginalParent = selfEntity->GetParent();
	}

	void Attackable::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value attack = v["Attackable"];
	}

	void Attackable::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Holdable* holdable = manager->GetComponent<Holdable>(selfEntity);

		if (holdable == nullptr) {
			Name* name = manager->GetComponent<Name>(selfEntity);
			ERROR_QUIT(std::format("ERROR!!! no Holdable component in this entity, entity name: {}\n태양이 두개면 시원한 이유는?\nsun sun 해지니까", name->getName()));
		}

		// todo
		// find self's dynamic collider
		// collide event with self -> other player or Guard, Get DAMAGE


		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::MOUSE_LEFT, KEY_STATE::START_PRESS), [manager, holdable, selfEntity](float deltaTime) {
			Entity* master = holdable->GetMaster();
			
			// todo
			// change player chara animation to attack
			auto masterAnimCtrl = manager->GetComponent<AnimationController>(master);
			//masterAnimCtrl->ChangeAnimationTo(ANIMATION_STATE::ATTACK);

			//auto selfCollider = manager->GetComponent<DynamicCollider>(selfEntity);
			//selfCollider->SetActive(true);

			DebugPrint("Todo: PlayAttackAnimation and Weapon Collider On");
		});
	}

	void Attackable::ShowYourself() const
	{
	}

	void Throwable::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value thr = v["Throwable"];

		m_ThrowBakeTimeMax = thr["BakeTime"].asFloat();
		m_ThrowSpeed = thr["ThrowSpeed"].asFloat();

		XMStoreFloat3(&m_DirectionOrigin, XMVector3Normalize(XMLoadFloat3(&m_DirectionOrigin)));
	}

	void Throwable::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Holdable* holdable = manager->GetComponent<Holdable>(selfEntity);

		if (holdable == nullptr) {
			Name* name = manager->GetComponent<Name>(selfEntity);
			ERROR_QUIT(std::format("ERROR!!! no Holdable component in this entity, entity name: {}\n차가 놀라면?\n앗 차가!", name->getName()));
		}

		// km/h -> cm/s
		float speed = m_ThrowSpeed * 27.7778f;
		float maxBakeTime = m_ThrowBakeTimeMax;
		float& bakeTime = m_ThrowBakeTime;
		XMFLOAT3& directionOrigin = m_DirectionOrigin;
		XMFLOAT3& directionResult = m_DirectionResult;

		// set collide event
		EventFunction cctvHitEvent = [manager](Entity* self, Entity* other) {

			Name* name = manager->GetComponent<Name>(other);

			DebugPrint(std::format("Hit Collider, name: {}", name->getName()));
			};

		DynamicCollider* dc = manager->GetComponent<DynamicCollider>(selfEntity);
		dc->InsertEvent<Collider>(cctvHitEvent, COLLIDE_EVENT_TYPE::ING);

		// todo
		// find self's dynamic collider
		// collide event with self -> other wall, stick to wall
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::MOUSE_LEFT, KEY_STATE::START_PRESS), [manager, holdable, selfEntity, &bakeTime](float deltaTime) {
			bakeTime = 0;

			Entity* master = holdable->GetMaster();

			// todo
			// change player chara animation to throw ready
			auto masterAnimCtrl = manager->GetComponent<AnimationController>(master);
			//masterAnimCtrl->ChangeAnimationTo(ANIMATION_STATE::THROWREADY);

			//auto selfCollider = manager->GetComponent<DynamicCollider>(selfEntity);
			//selfCollider->SetActive(true);

			DebugPrint("Todo: PlayAttackAnimation and Weapon Collider On");
		});
	
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::MOUSE_LEFT, KEY_STATE::PRESSING), [&directionOrigin, &directionResult, &bakeTime, manager, holdable](float deltaTime) {
			Inventory* masterInv = manager->GetComponent<Inventory>(holdable->GetMaster());
			Transform* masterTr = manager->GetComponent<Transform>(masterInv->GetHoldingSocket()->GetParent());

			XMVECTOR dir{ directionOrigin.x, directionOrigin.y, directionOrigin.z, 0 };
			XMMATRIX world = XMLoadFloat4x4(&masterTr->GetWorldTransform());

			// vector4 transform으로 방향벡터 transform
			XMVECTOR resDir = XMVector3Normalize(XMVector4Transform(dir, world));
			XMStoreFloat3(&directionResult, resDir);

			bakeTime += deltaTime;

		});
	
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::MOUSE_LEFT, KEY_STATE::END_PRESS), [&directionResult, &bakeTime, maxBakeTime, speed, manager, selfEntity, holdable](float deltaTime) {
			XMStoreFloat3(&directionResult, XMVector3Normalize(XMLoadFloat3(&directionResult)));
			DebugPrintVector(directionResult, "realDir");
			bakeTime = std::min(maxBakeTime, bakeTime);

			DebugPrint(std::format("bakeTime: {}, speed: {}, result: {}", bakeTime, speed, ((bakeTime / maxBakeTime)*speed) / 27.7778f));


			float resultSpeed = speed * (bakeTime / maxBakeTime);
			// detach
			manager->DetachChild(selfEntity->GetParent(), selfEntity);

			// erase itself from Inventory
			Inventory* masterInv = manager->GetComponent<Inventory>(holdable->GetMaster());
			masterInv->EraseCurrentHolding();

			Physics* py = manager->GetComponent<Physics>(selfEntity);
			DynamicCollider* dc = manager->GetComponent<DynamicCollider>(selfEntity);

		
			// add force, calculate physics true
			directionResult.x *= resultSpeed;
			directionResult.y *= resultSpeed;
			directionResult.z *= resultSpeed;

			py->SetVelocity(directionResult);
			py->SetCalculateState(true);

			DebugPrintVector(py->GetVelocity(), "velocity");
			// set collider on
			dc->SetActive(true);

			// set collide event
			EventFunction cctvHitEvent = [manager](Entity* self, Entity* other) {
				DynamicCollider* dc = manager->GetComponent<DynamicCollider>(self);
				Physics* py = manager->GetComponent<Physics>(self);

				// set velocity calculate false
				py->SetCalculateState(false);
				py->SetVelocity({ 0,0,0 });

				// collider off
				dc->SetActive(false);

				Name* name = manager->GetComponent<Name>(other);
				Transform* otherTrans = manager->GetComponent<Transform>(other);
				Collider* otherCol = manager->GetComponent<Collider>(other);

				DebugPrint(std::format("Hit Collider, name: {}", name->getName()));
				};


			dc->InsertEvent<Collider>(cctvHitEvent, COLLIDE_EVENT_TYPE::BEGIN);

			DebugPrint("Throw");
		});

	}

	void Screen::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void Screen::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		// todo 임시이다
		int cameraIndex = -1;
		std::function<void(Name*, Camera*)> findCam = [&cameraIndex, manager](Name* name, Camera* cam) {
			if (name->getName() == "CCTV_01")
				cameraIndex = cam->GetCameraIndex();
			};
		manager->Execute(findCam);

		if (cameraIndex != -1) {
			const auto& data = rm->GetCameraRenderTargetData(cameraIndex);

			component::Renderer* ren = manager->GetComponent<component::Renderer>(selfEntity);
			Material* mat = rm->GetMaterial(ren->GetMaterial());
			mat->SetDataIndex(0, data.m_ResultRenderTargetIndex);
		}
	}

	void Screen::ShowYourself() const
	{
	}

	void Sittable::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value sit = v["Sittable"];

		m_AttachPosition.x = sit["AttachPosition"][0].asFloat();
		m_AttachPosition.y = sit["AttachPosition"][1].asFloat();
		m_AttachPosition.z = sit["AttachPosition"][2].asFloat();
	}

	void Sittable::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Interaction* interaction = manager->GetComponent<Interaction>(selfEntity);

		if (interaction == nullptr)
			ERROR_QUIT("ERROR!! no interaction component on current Sittable entity");

		InteractionFuncion withSittable = [manager](Entity* player, Entity* sittableEntity) {
			AnimationController* animCtrl = manager->GetComponent<AnimationController>(player);
			Player* playerComp = manager->GetComponent<Player>(player);

			
			DebugPrint("Interaction");

			// play sit start
			if (playerComp->IsSitting() == false) {

				// todo
				// attach player? 0rotation here
				animCtrl->ChangeAnimationTo(ANIMATION_STATE::SIT_START);
				playerComp->SetSit(true);
				DebugPrint("goSit");
			}
			else {
				animCtrl->ChangeAnimationTo(ANIMATION_STATE::SIT_END);
				playerComp->SetSit(false);
				DebugPrint("standUp");
			}


			};
		interaction->SetInteractionFunction(withSittable);

	}

}
