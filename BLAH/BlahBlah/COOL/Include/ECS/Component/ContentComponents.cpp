#include "framework.h"
#include "ContentComponents.h"
#include "Scene/ResourceManager.h"
#include "ECS/ECSManager.h"
#include "Animation/AnimationTrack.h"
#include "json/json.h"
#include "ECS/TimeLine/TimeLine.h"
#include "Network/Client.h"
#include "FMODsound/FmodSound.h"
#include "App/Application.h"
#include "Scene/SceneManager.h"

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
				XMFLOAT4X4 worldtr = tr->GetWorldTransform();
				XMMATRIX world = XMLoadFloat4x4(&worldtr);
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

			// jump
			ctrl->InsertCondition(ANIMATION_STATE::JUMP_START, ANIMATION_STATE::JUMP_ING, endPlaying);
			ctrl->InsertCondition(ANIMATION_STATE::JUMP_LAND, ANIMATION_STATE::BLENDED_MOVING_STATE, endPlaying);

			// sit
			ctrl->InsertCondition(ANIMATION_STATE::SIT_START, ANIMATION_STATE::SIT_LOOP, endPlaying);
			ctrl->InsertCondition(ANIMATION_STATE::SIT_END, ANIMATION_STATE::BLENDED_MOVING_STATE, endPlaying);

			// attack
			ctrl->InsertCondition(ANIMATION_STATE::ATTACK, ANIMATION_STATE::BLENDED_MOVING_STATE, endPlaying);

			// throw
			ctrl->InsertCondition(ANIMATION_STATE::THROW_END, ANIMATION_STATE::BLENDED_MOVING_STATE, endPlaying);

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

		COND end_play = [ctrl](void* data) {
			return ctrl->GetCurrentPlayTime() - ctrl->GetCurrentPlayEndTime() > 0.0f;
			};

		// insert transition (graph)
		ctrl->InsertCondition(ANIMATION_STATE::IDLE, ANIMATION_STATE::WALK, walk);
		ctrl->InsertCondition(ANIMATION_STATE::WALK, ANIMATION_STATE::IDLE, idle);
		ctrl->InsertCondition(ANIMATION_STATE::ATTACK, ANIMATION_STATE::IDLE, end_play);

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

		m_MaxAngle = door["MaxAngle"].asFloat();
		m_Answer = door["Answer"].asInt();
		m_KeyID = door["KeyID"].asInt();
		m_Locked = door["Locked"].asInt();
		m_Gamemode = door["Game"].asInt();
		m_FailCount = door["FailCount"].asInt();
		m_RotateAxis = door["RotateAxis"].asInt();
		m_DoorId = door["ID"].asInt();
		m_Hp = door["HP"].asInt();

	}

	void DoorControl::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Interaction* interaction = manager->GetComponent<Interaction>(selfEntity);

		if (interaction == nullptr)
			ERROR_QUIT("ERROR!! no interaction component on current DoorControl entity");

		InteractionFuncion withDoor = [manager](Entity* player, Entity* door) {
			DebugPrint("Interaction, DOOR!!");

			component::DoorControl* doorCtrl = manager->GetComponent<component::DoorControl>(door);
			component::Inventory* inven = manager->GetComponent<component::Inventory>(player);
			auto trans = manager->GetComponent<component::Transform>(door);

			// put key into player's keytool
			if (doorCtrl != nullptr && (1 <= doorCtrl->GetKeyID() && 9 >= doorCtrl->GetKeyID())) {
				Entity* curhold = inven->GetCurrentHoldingItem();
				if (curhold != nullptr) {
					KeyTool* keytool = manager->GetComponent<KeyTool>(curhold);
					if (keytool != nullptr) {
						keytool->InsertKey(doorCtrl->GetKeyID());
						trans->SetPosition({ 300,0,0 });
					}
				}
			}
			else if (doorCtrl->GetKeyDoorOpen() == true) {
				Entity* curhold = inven->GetCurrentHoldingItem();
				if (curhold != nullptr) {
					KeyTool* keytool = manager->GetComponent<KeyTool>(curhold);
					if (keytool != nullptr) {
						keytool->DeleteKey(doorCtrl->GetAnswer());
						doorCtrl->SetKeyDoorOpen(false);
					}
				}
			}
			
			// if opened
			if (doorCtrl != nullptr && doorCtrl->IsLocked() == false) {

				char toState = 0;
				if (doorCtrl->IsOpen() == false)
					toState = 1;

				cs_packet_open_door packet(doorCtrl->GetDoorID(), toState);
				Client::GetInstance().send_packet(&packet);

				cs_packet_sound_start packet(trans->GetWorldPosition(), SOUND_TYPE::DOOR_OPEN);
				Client::GetInstance().send_packet(&packet);

				doorCtrl->SetOpen(manager, trans, door, !doorCtrl->IsOpen(), true);


			}
			else {
				// todo
				// door에 연결된 미니게임이 무엇인지 정보를 찾고
				// 키패드인지 패턴인지 등등을 확인한 후
				// 정답에 대한 정보를 넣어준다. set answer
				// 정답에 대한 정보는 door가 가지고 있다. set door
				// KeyPadUI는 확인 버튼이 눌렸을 시 전달받은 정보를 토대로 통과 여부를 판단
				
				if (doorCtrl->GetGamemode() == 1) { //1 == keypad
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
				else if (doorCtrl->GetGamemode() == 2) { //2 == key
					Entity* curhold = inven->GetCurrentHoldingItem();
					if (curhold != nullptr) {
						KeyTool* keytool = manager->GetComponent<KeyTool>(curhold);
						if (keytool != nullptr) {
							std::function<void(component::UICanvas*, component::UIDoorKey*)> openKey = [door, doorCtrl, keytool](component::UICanvas* can, component::UIDoorKey* key) {
								DebugPrint("KEY SHOW");

								key->SetAnswer(doorCtrl->GetAnswer());
								for (int i = 0; i < MAX_BUTTON; ++i) {
									key->SetAnswerButton(i, keytool->GetKeyHold(i+1));
									key->SetAnswerButtonMaterial(i, keytool->GetKeyHold(i+1));
								}
								key->SetAnswerMaterial(doorCtrl->GetAnswer());
								key->SetFailCount(doorCtrl->GetFailCount());
								key->SetDoor(door);

								can->ShowUI();
								};

							manager->Execute(openKey);
						}
					}
				}
				else if (doorCtrl->GetGamemode() == 3) { //3 == doorlock Cutline
					std::function<void(component::UICanvas*, component::UICutLine*)> openCutline = [door, doorCtrl](component::UICanvas* can, component::UICutLine* line) {
						DebugPrint("CUTLINE SHOW");

						line->SetAnswer(doorCtrl->GetAnswer());
						line->SetDoor(door);

						line->SetCurrent(0);

						if (doorCtrl->IsUioff() == false) can->ShowUI();
						};

					manager->Execute(openCutline);
				}
				else if (doorCtrl->GetGamemode() == 5) { //5 == final create Cutline
					Entity* curhold = inven->GetCurrentHoldingItem();
					if (curhold != nullptr) {
						KeyTool* keytool = manager->GetComponent<KeyTool>(curhold);
						if (keytool != nullptr) {
							std::function<void(component::UICanvas*, component::UITreasureChest*)> openKey = [door, doorCtrl, keytool](component::UICanvas* can, component::UITreasureChest* key) {
								DebugPrint("KEY SHOW");

								key->SetAnswer(doorCtrl->GetAnswer());
								for (int i = 0; i < MAX_BUTTON; ++i) {
									key->SetAnswerButton(i, keytool->GetKeyHold(i + 1));
									key->SetAnswerButtonMaterial(i, keytool->GetKeyHold(i + 1));
								}

								//key->SetAnswerMaterial(,doorCtrl->GetAnswer());
								key->SetDoor(door);

								can->ShowUI();
								};

							manager->Execute(openKey);
						}
					}
				}
			}
			};

		interaction->SetInteractionFunction(withDoor);
	}

	void DoorControl::ShowYourself() const
	{
	}

	void DoorControl::SetLock(bool lock, bool sendServer)
	{
		m_Locked = lock;

		// send door open packet
		if (sendServer) {
			cs_packet_unlock_door packet(m_DoorId, m_Open ? 1 : 0);
			Client::GetInstance().send_packet(&packet);
		}
	}

	void DoorControl::SetOpen(ECSManager* manager, Transform* tr, Entity* self, bool state, bool sendServer)
	{
		auto rot = tr->GetRotation();
		XMFLOAT3 rotAfter = rot;

		float angle = GetMaxAngle();
		if (m_Open) angle *= -1;

		DebugPrint(std::format("angle: {}", angle));

		switch (m_RotateAxis) {
		case 0:		rotAfter.x -= angle;	break;
		case 1:		rotAfter.y -= angle;	break;
		case 2:		rotAfter.z -= angle;	break;
			break;
		}

		TimeLine<XMFLOAT3>* openDoor = new TimeLine<XMFLOAT3>(tr->GetRotationPtr());
		openDoor->AddKeyFrame(rot, 0);
		openDoor->AddKeyFrame(rotAfter, 1);

		manager->AddTimeLine(openDoor);

		// send door open packet
		if (sendServer) {
			DebugPrint("SEND DOOR OPEN");
			cs_packet_open_door packet(m_DoorId, state ? 1 : 0);
			Client::GetInstance().send_packet(&packet);
		}

		FMOD_INFO::GetInstance().play_unloop_sound(tr->GetWorldPosition(), SOUND_TYPE::DOOR_OPEN, "DOOR_OPEN");
		if (sendServer) {
			cs_packet_sound_start packet(tr->GetWorldPosition(), SOUND_TYPE::DOOR_OPEN);
			Client::GetInstance().send_packet(&packet);
		}

		m_Open = state;
	}

	void DoorControl::SetCrushPosition(XMFLOAT3 pos, float power, float distance)
	{
		// crush point는 z는 사용하지 않는다
		m_CrushPositionAndPower[m_NextCrushIndex].x = pos.x;
		m_CrushPositionAndPower[m_NextCrushIndex].y = pos.y;
		m_CrushPositionAndPower[m_NextCrushIndex].z = power;
		m_CrushPositionAndPower[m_NextCrushIndex].w = distance;

		m_NextCrushIndex = (m_NextCrushIndex + 1) % _countof(m_CrushPositionAndPower);
	}

	void UIDoorKey::Create(Json::Value& v, ResourceManager* rm)
	{
		rm->AddLateLoadUI("UI/UITempMaterial", nullptr);
		rm->AddLateLoadUI("UI/UIKey1", nullptr);
		rm->AddLateLoadUI("UI/UIKey2", nullptr);
		rm->AddLateLoadUI("UI/UIOld_Key", nullptr);
		rm->AddLateLoadUI("UI/UIRing_Key", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Black", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Blue", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Green", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Pink", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Red", nullptr);
		rm->AddLateLoadUI("UI/UIX", nullptr);

	}

	void UIDoorKey::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		rm->GetMaterial("Key_No1");

		auto& children = selfEntity->GetChildren();

		UICanvas* canvas = manager->GetComponent<UICanvas>(selfEntity);
		UIDoorKey* doorkey = manager->GetComponent<UIDoorKey>(selfEntity);

		m_KeyMaterialMap = std::map<int, int>();
		m_KeyMaterialMap[0] = rm->GetMaterial("UITempMaterial");
		m_KeyMaterialMap[1] = rm->GetMaterial("UIKey1");
		m_KeyMaterialMap[2] = rm->GetMaterial("UIKey2");
		m_KeyMaterialMap[3] = rm->GetMaterial("UIOld_Key");
		m_KeyMaterialMap[4] = rm->GetMaterial("UIRing_Key");
		m_KeyMaterialMap[5] = rm->GetMaterial("UIFinalChestKey_Black");
		m_KeyMaterialMap[6] = rm->GetMaterial("UIFinalChestKey_Blue");
		m_KeyMaterialMap[7] = rm->GetMaterial("UIFinalChestKey_Green");
		m_KeyMaterialMap[8] = rm->GetMaterial("UIFinalChestKey_Pink");
		m_KeyMaterialMap[9] = rm->GetMaterial("UIFinalChestKey_Red");

		m_KeyMaterialMap[-1] = rm->GetMaterial("UIX");


		for (Entity* child : children) {
			Name* childName = manager->GetComponent<Name>(child);

			// 확인 버튼
			if (childName != nullptr) {
				std::string name = childName->getName();

				if (name == "Exit") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction exit = [canvas](Entity* ent) {
						// hide ui;
						canvas->HideUI();

						};
					button->SetButtonEvent(KEY_STATE::END_PRESS, exit);
				}
				if (name == "AnswerKey") {
					m_AnswerUIrender = manager->GetComponent<UIRenderer>(child);
				}
				if (name == "Check") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction check = [canvas, doorkey, manager](Entity* ent) {
						// hide ui;

						if (doorkey->GetAnswer() == doorkey->GetCurrent()) {
							// open door
							Entity* door = doorkey->GetDoor();
							DoorControl* doorCtrl = manager->GetComponent<DoorControl>(door);
							doorCtrl->SetLock(false);
							doorCtrl->SetKeyDoorOpen(true);
							canvas->HideUI();
						}
						else {
							int failcount = doorkey->GetFailCount();
							failcount--;
							if (doorkey->GetFailCount() == 0) {
								canvas->HideUI();
							}
							doorkey->SetFailCount(failcount);
							DebugPrint(std::format("failcount: {}", failcount));
						}

						};
					button->SetButtonEvent(KEY_STATE::END_PRESS, check);
				}
				else {
					for (int i = 0; i <= MAX_BUTTON; ++i) {
						std::string buttonName = std::format("KeyButton{}", i);
						if (name == buttonName) {
							Button* button = manager->GetComponent<Button>(child);
							UIRenderer* render = manager->GetComponent<UIRenderer>(child);
							m_AnswerbuttonUIrender[i] = render;
							Key* key = manager->GetComponent<Key>(child);
							m_keyAnswer[i] = key;
							ButtonEventFunction password = [key, doorkey, manager, i](Entity* ent) { // 버튼에 대한 콜백함수 등록
								int current = key->GetKeyID();
								doorkey->SetCurrent(current);
								DebugPrint(std::format("length: {}", current));
								};
							button->SetButtonEvent(KEY_STATE::END_PRESS, password);
						}
					}
				}
			}
		}
	}

	void UIDoorKey::ShowYourself() const
	{
	}

	void UIDoorKey::SetAnswerMaterial(int idx)
	{
		m_AnswerUIrender->SetMaterial(m_KeyMaterialMap[idx]);
	}

	void UIDoorKey::SetAnswerButtonMaterial(int target, int answer)
	{
		m_AnswerbuttonUIrender[target]->SetMaterial(m_KeyMaterialMap[answer]);
	}

	void UIDoorKey::SetAnswerButton(int target, int answer)
	{
		m_keyAnswer[target]->SetKeyID(answer);
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
				if (name == "Exit") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction exit = [canvas](Entity* ent) {
						canvas->HideUI();
						};
					button->SetButtonEvent(KEY_STATE::END_PRESS, exit);
				}

				if (name == "Check") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction check = [canvas, keypad, manager](Entity* ent) {
						// hide ui;
						DebugPrint("check");

						DebugPrint(std::format("pw: {}", keypad->GetCurrent()));
						if (keypad->GetAnswer() == keypad->GetCurrent()) {
							// open door
							Entity* door = keypad->GetDoor();
							DoorControl* doorCtrl = manager->GetComponent<DoorControl>(door);
							doorCtrl->SetLock(false);
							canvas->HideUI();
						}

						};
					button->SetButtonEvent(KEY_STATE::END_PRESS, check);
				}
				else {
					for (int i = 0; i <= 9; ++i) {
						std::string buttonName = std::format("{}Button", i);
						if (name == buttonName) {
							Button* button = manager->GetComponent<Button>(child);
							ButtonEventFunction password = [keypad, manager, i](Entity* ent) { // 버튼에 대한 콜백함수 등록
								int current = keypad->GetCurrent();
								current = current * 10 + i;
								keypad->SetCurrent(current);

								};
							button->SetButtonEvent(KEY_STATE::END_PRESS, password);
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
		m_SubInventorySocketName = inven["SubSocketName"].asString();

		for(int i = 0; i < MAX_INVENTORY; ++i)
			m_TargetEntityNames[i] = inven[std::format("Slot_{}", i)].asString().c_str();
	}

	void Inventory::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		m_HoldingSocket = manager->GetEntityFromRoute(m_InventorySocketName, selfEntity);
		m_SubHoldingSocket = manager->GetEntityFromRoute(m_SubInventorySocketName, selfEntity);

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

		// default main mode
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

	const Entity* Inventory::GetHoldingSocket() const
	{
		if (m_MainMode)
			return m_HoldingSocket;

		return m_SubHoldingSocket;
	}

	void Inventory::SetMainMode(bool state, ECSManager* manager)
	{
		if (m_MainMode == state) return;
		m_MainMode = state;

		Entity* curItem = GetCurrentHoldingItem();
		if (curItem != nullptr) {
			if (state == true)
				manager->AttachChild(m_HoldingSocket, curItem);
			else
				manager->AttachChild(m_SubHoldingSocket, curItem);
		}
	}

	void Holdable::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value hold = v["Holdable"];
		m_HoldableItemID = hold["HoldableID"].asInt();
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
			masterAnimCtrl->ChangeAnimationTo(ANIMATION_STATE::ATTACK);
			auto& client = Client::GetInstance();
			std::string playername;

			auto masterTr = manager->GetComponent<Transform>(master);
			FMOD_INFO::GetInstance().play_unloop_sound(masterTr->GetWorldPosition(), SOUND_TYPE::CROWBAR_SWING, "crobwar_swing");

			switch (client.getCharType())
			{
			case 1:
				playername = client.GetHostPlayerName();
				break;
			case 2:
				playername = client.GetGuestPlayerName();
				break;
			}
			if (masterAnimCtrl->GetPlayer()->GetName() == playername)			// 자신의 캐릭터라면 animation 변경 패킷 전송 및 소리패킷 전송
			{
				cs_packet_anim_type anim(ANIMATION_STATE::ATTACK);
				Client::GetInstance().send_packet(&anim);

				cs_packet_sound_start packet(masterTr->GetWorldPosition(), SOUND_TYPE::CROWBAR_SWING);
				Client::GetInstance().send_packet(&packet);
			}

			// play sound


			// set collider on
			auto selfCollider = manager->GetComponent<DynamicCollider>(selfEntity);
			selfCollider->SetActive(true);

			DebugPrint("Todo: PlayAttackAnimation and Weapon Collider On");
		});

		// set collider event
		DynamicCollider* collider = manager->GetComponent<DynamicCollider>(selfEntity);
		EventFunction attackHit = [manager](Entity* self, Entity* other) {
			DebugPrint("doorCrush");

			DynamicCollider* collider = manager->GetComponent<DynamicCollider>(self);
			Transform* doorTrans = manager->GetComponent<Transform>(other);

			// get collider center position on other transform
			XMFLOAT3 attackCenter = collider->GetBoundingBox().Center;

			XMFLOAT4X4 doorWorld = doorTrans->GetWorldTransform();
			XMVECTOR pos = XMLoadFloat3(&attackCenter);
			XMMATRIX doorWorldInv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&doorWorld));

			pos = XMVector3Transform(pos, doorWorldInv);

			XMFLOAT3 attackPosOnDoorLocal;
			XMStoreFloat3(&attackPosOnDoorLocal, pos);

			// set collided position on renderer
			DoorControl* doorComp = manager->GetComponent<DoorControl>(other);
			doorComp->SetCrushPosition(attackPosOnDoorLocal, 50.0f, 15.0f);
			
			// todo 문 체력 줄여서 0되면 열리게 
			int doorhp = doorComp->GetDoorHp();
			doorComp->SetDoorHp(--doorhp);
			if (doorhp == 0) doorComp->SetLock(false);
			// todo 소리 재생
			};

		collider->InsertEvent<DoorControl>(attackHit, COLLIDE_EVENT_TYPE::BEGIN);

		EventFunction setFirePoo = [manager](Entity* self, Entity* other) {
			DynamicCollider* collider = manager->GetComponent<DynamicCollider>(self);
			Collider* otherColl = manager->GetComponent<Collider>(other);

			// if trigger, no fire
			if (otherColl->IsTrigger()) return;

			Transform* tr = manager->GetComponent<Transform>(self);
			XMFLOAT3 pos =  tr->GetWorldPosition();
			XMFLOAT3 otherPos = otherColl->GetBoundingBox().Center;

			XMFLOAT3 vel = {
				pos.x - otherPos.x,
				pos.y - otherPos.y,
				pos.z - otherPos.z,
			};
			XMVECTOR velT = XMVector3Normalize(XMLoadFloat3(&vel)) * 200.0f;
			XMStoreFloat3(&vel, velT);

			DebugPrint("POO");
			manager->AddParticle(PARTICLE_TYPES::SPARK, pos, vel, 100, 10.0f);
			collider->SetActive(false);

			// play sound
			FMOD_INFO::GetInstance().play_unloop_sound(tr->GetWorldPosition(), SOUND_TYPE::CROWBAR_HIT, "crobwar_hit");

			cs_packet_sound_start packet(tr->GetWorldPosition(), SOUND_TYPE::CROWBAR_HIT);
			Client::GetInstance().send_packet(&packet);

			};
		collider->InsertEvent<Collider>(setFirePoo, COLLIDE_EVENT_TYPE::BEGIN);
		collider->SetActive(false);
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
			
			Physics* py = manager->GetComponent<Physics>(self);
			DebugPrintVector(py->GetVelocity(), "velocity: ");

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
			masterAnimCtrl->ChangeAnimationTo(ANIMATION_STATE::THROW_START);

			//auto selfCollider = manager->GetComponent<DynamicCollider>(selfEntity);
			//selfCollider->SetActive(true);

			DebugPrint("Todo: PlayAttackAnimation and Weapon Collider On");
		});
	
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::MOUSE_LEFT, KEY_STATE::PRESSING), [&directionOrigin, &directionResult, &bakeTime, manager, holdable](float deltaTime) {
			Inventory* masterInv = manager->GetComponent<Inventory>(holdable->GetMaster());
			Transform* masterTr = manager->GetComponent<Transform>(masterInv->GetHoldingSocket()->GetParent());

			XMVECTOR dir{ directionOrigin.x, directionOrigin.y, directionOrigin.z, 0 };
			XMFLOAT4X4 worldTr = masterTr->GetWorldTransform();
			XMMATRIX world = XMLoadFloat4x4(&worldTr);

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

			// set anim to throw end
			Entity* master = holdable->GetMaster();
			auto masterAnimCtrl = manager->GetComponent<AnimationController>(master);
			masterAnimCtrl->ChangeAnimationTo(ANIMATION_STATE::THROW_END);

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

			// if particle emitter have
			ParticleEmitter* pe = manager->GetComponent<ParticleEmitter>(selfEntity);
			if (pe != nullptr) {
				pe->SetActive(true);
			}

			// set collide event
			EventFunction cctvHitEvent = [manager](Entity* self, Entity* other) {
				DynamicCollider* dc = manager->GetComponent<DynamicCollider>(self);
				Physics* py = manager->GetComponent<Physics>(self);

				// if particle emitter have
				ParticleEmitter* pe = manager->GetComponent<ParticleEmitter>(self);
				if (pe != nullptr) {
					pe->SetActive(false);
					DebugPrint("ON");
				}

				// set velocity calculate false
				py->SetCalculateState(false);
				py->SetVelocity({ 0,0,0 });

				// collider off
				dc->SetActive(false);

				Server* serv = manager->GetComponent<Server>(self);
				Transform* tr = manager->GetComponent<Transform>(self);
				if (serv != nullptr) {
					XMFLOAT3 pos = tr->GetPosition();
					XMFLOAT3 rot = tr->GetRotation();
					XMFLOAT3 vel = py->GetVelocity();

					auto id = serv->getID();
					cs_packet_position packet(id, pos, rot, vel);
					Client::GetInstance().send_packet(&packet);

				}
				Name* name = manager->GetComponent<Name>(other);
				Transform* otherTrans = manager->GetComponent<Transform>(other);
				Collider* otherCol = manager->GetComponent<Collider>(other);

				DebugPrint(std::format("Hit Collider, name: {}", name->getName()));

				Drink* drink = manager->GetComponent<Drink>(self);
				if (drink != nullptr) {
					// play sound
					FMOD_INFO::GetInstance().play_unloop_sound(tr->GetWorldPosition(), SOUND_TYPE::DRINK_THROW_HIT, "DRINK_THROW");

					cs_packet_sound_start packet(tr->GetWorldPosition(), SOUND_TYPE::DRINK_THROW_HIT);
					Client::GetInstance().send_packet(&packet);
				}
				};


			dc->InsertEvent<Collider>(cctvHitEvent, COLLIDE_EVENT_TYPE::BEGIN);

			DebugPrint("Throw");
		});

	}

	void Screen::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value sc = v["Screen"];

		for (int i = 0; i < MAX_CCTV; ++i)
			m_TargetNames[i] = sc[std::format("Slot_{}", i)].asString().c_str();
	}

	void Screen::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		// todo 임시이다
		int cameraIndex[MAX_CCTV] = { -1 };
		memset(cameraIndex, -1, sizeof(cameraIndex));

		for (int i = 0; i < MAX_CCTV; ++i) {
			Entity* camEnt = manager->GetEntity(m_TargetNames[i]);
			if (camEnt == nullptr) continue;

			Camera* cam = manager->GetComponent<Camera>(camEnt);
			if (cam == nullptr) continue;

			cameraIndex[i] = cam->GetCameraIndex();
		}

		for (int i = 0; i < MAX_CCTV; ++i) {
			if (cameraIndex[i] == -1) continue;
			const auto& data = rm->GetCameraRenderTargetData(cameraIndex[i]);

			component::Renderer* ren = manager->GetComponent<component::Renderer>(selfEntity);
			Material* mat = rm->GetMaterial(ren->GetMaterial());
			mat->SetDataIndex(0, data.m_ResultRenderTargetIndex);
		}

		component::Renderer* rend = manager->GetComponent<component::Renderer>(selfEntity);
		XMFLOAT4 extra = { 1.0f, 0.0f, 0.0f, 0.0f };
		Material* mat = rm->GetMaterial(rend->GetMaterial());
		mat->SetExtraDataIndex(0, 1.0f);
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

		m_SittableSocketName = sit["SocketName"].asString();
	}

	void Sittable::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		m_SittableSocket = manager->GetEntityFromRoute(m_SittableSocketName, selfEntity);
		Entity* sittableSocket = m_SittableSocket;

		Interaction* interaction = manager->GetComponent<Interaction>(selfEntity);

		if (interaction == nullptr)
			ERROR_QUIT("ERROR!! no interaction component on current Sittable entity");

		InteractionFuncion withSittable = [manager, sittableSocket](Entity* player, Entity* sittableEntity) {
			AnimationController* animCtrl = manager->GetComponent<AnimationController>(player);
			Player* playerComp = manager->GetComponent<Player>(player);

			
			DebugPrint("Interaction");

			Transform* tr = manager->GetComponent<Transform>(player);

			// play sit start
			if (playerComp->IsSitting() == false) {
				// attach first
				tr->SetPosition({ 0,0,0 });
				tr->SetRotation({ 0,0,0 });
				manager->AttachChild(sittableSocket, player);

				// set animation
				animCtrl->ChangeAnimationTo(ANIMATION_STATE::SIT_START);
				cs_packet_anim_type anim(ANIMATION_STATE::SIT_START);
				Client::GetInstance().send_packet(&anim);
				float endTime = animCtrl->GetCurrentPlayEndTime();

				// set player position
				TimeLine<XMFLOAT3>* trMove = new TimeLine<XMFLOAT3>(tr->GetPositionPtr());
				trMove->AddKeyFrame({ 0, 0, 37.9f }, 0.0f);
				trMove->AddKeyFrame({ 0, 0, 37.9f }, endTime);
				trMove->AddKeyFrame({ 0, 0, 0 }, endTime);
				manager->AddTimeLine(trMove);

				// set player camera position
				Entity* camera = manager->GetEntityInChildren("PlayerCamera", player);
				Transform* camTr = manager->GetComponent<Transform>(camera);
				TimeLine<XMFLOAT3>* camMove = new TimeLine<XMFLOAT3>(camTr->GetPositionPtr());
				XMFLOAT3 curPos = camTr->GetPosition();
				XMFLOAT3 aftPos = curPos;
				aftPos.y -= 50.0f;
				camMove->AddKeyFrame(curPos, 0.0f);
				camMove->AddKeyFrame(aftPos, endTime);
				manager->AddTimeLine(camMove);

				// set inventory also
				Entity* inven = manager->GetEntityInChildren("InvenParent", camera);
				Transform* invTr = manager->GetComponent<Transform>(inven);
				TimeLine<XMFLOAT3>* invMove = new TimeLine<XMFLOAT3>(invTr->GetPositionPtr());
				curPos = invTr->GetPosition();
				aftPos = curPos;
				aftPos.y += 50.0f;
				invMove->AddKeyFrame(curPos, 0.0f);
				invMove->AddKeyFrame(aftPos, endTime);
				manager->AddTimeLine(invMove);

				// sit
				playerComp->SetSit(true);
				DebugPrint("goSit");
			}
			else {
				// detach
				manager->DetachChild(sittableSocket, player);

				// set animation
				animCtrl->ChangeAnimationTo(ANIMATION_STATE::SIT_END);
				cs_packet_anim_type anim(ANIMATION_STATE::SIT_END);
				Client::GetInstance().send_packet(&anim);
				float endTime = animCtrl->GetCurrentPlayEndTime();

				// set player camera position
				Entity* camera = manager->GetEntityInChildren("PlayerCamera", player);
				Transform* camTr = manager->GetComponent<Transform>(camera);
				TimeLine<XMFLOAT3>* camMove = new TimeLine<XMFLOAT3>(camTr->GetPositionPtr());
				XMFLOAT3 curPos = camTr->GetPosition();
				XMFLOAT3 aftPos = curPos;
				aftPos.y += 50.0f;
				camMove->AddKeyFrame(curPos, 0.0f);
				camMove->AddKeyFrame(aftPos, endTime);
				manager->AddTimeLine(camMove);

				// set inventory also
				Entity* inven = manager->GetEntityInChildren("InvenParent", camera);
				Transform* invTr = manager->GetComponent<Transform>(inven);
				TimeLine<XMFLOAT3>* invMove = new TimeLine<XMFLOAT3>(invTr->GetPositionPtr());
				curPos = invTr->GetPosition();
				aftPos = curPos;
				aftPos.y -= 50.0f;
				invMove->AddKeyFrame(curPos, 0.0f);
				invMove->AddKeyFrame(aftPos, endTime);
				manager->AddTimeLine(invMove);

				playerComp->SetSit(false);
				DebugPrint("standUp");
			}


			};
		interaction->SetInteractionFunction(withSittable);

	}

	void CCTVController::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value cctvCtrl = v["CCTVController"];

		for (int i = 0; i < MAX_CAMERA; ++i)
			m_TargetCCTVNames[i] = cctvCtrl[std::format("Slot_{}", i)].asString().c_str();
	}

	void CCTVController::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		// get camera render target indices
		for (int i = 0; i < MAX_CAMERA; ++i) 
			m_CCTVEntities[i] = manager->GetEntity(m_TargetCCTVNames[i]);


		// set holdable action maps
		Holdable* holdable = manager->GetComponent<Holdable>(selfEntity);

		const float rootSpeed = 10.0f;

		// rotate by arrow keys
		ActionFunction arrowUp = [this, manager, rootSpeed](float deltaTime) {
			Transform* tr = manager->GetComponent<Transform>(m_CCTVEntities[m_CurrentCCTV]);
			XMFLOAT3 rot = tr->GetRotation();
			rot.x -= (deltaTime * rootSpeed);
			tr->SetRotation(rot);
			};
		ActionFunction arrowDown = [this, manager, rootSpeed](float deltaTime) {
			Transform* tr = manager->GetComponent<Transform>(m_CCTVEntities[m_CurrentCCTV]);
			XMFLOAT3 rot = tr->GetRotation();
			rot.x += (deltaTime * rootSpeed);
			tr->SetRotation(rot);
			};
		ActionFunction arrowLeft = [this, manager, rootSpeed](float deltaTime) {
			Transform* tr = manager->GetComponent<Transform>(m_CCTVEntities[m_CurrentCCTV]);
			XMFLOAT3 rot = tr->GetRotation();
			rot.y -= (deltaTime * rootSpeed);
			tr->SetRotation(rot);
			};
		ActionFunction arrowRight = [this, manager, rootSpeed](float deltaTime) {
			Transform* tr = manager->GetComponent<Transform>(m_CCTVEntities[m_CurrentCCTV]);
			XMFLOAT3 rot = tr->GetRotation();
			rot.y += (deltaTime * rootSpeed);
			tr->SetRotation(rot);
			};
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_UP, KEY_STATE::PRESSING), arrowUp);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_DOWN, KEY_STATE::PRESSING), arrowDown);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_LEFT, KEY_STATE::PRESSING), arrowLeft);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_RIGHT, KEY_STATE::PRESSING), arrowRight);

		// with [, ], change cctvs
		ActionFunction prev = [this](float deltaTime) {};
		ActionFunction next = [this](float deltaTime) {};
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::PREVIOUS, KEY_STATE::END_PRESS), prev);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::NEXT, KEY_STATE::END_PRESS), next);
	}

	void VandingMachine::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void VandingMachine::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		// set can objects
		std::vector<Entity*> cans;
		std::function<void(Drink*, SelfEntity*)> findDrink = [&cans](Drink*, SelfEntity* ent) {cans.push_back(ent->GetEntity()); };
		manager->Execute(findDrink);

		// set interaction
		Interaction* interaction = manager->GetComponent<Interaction>(selfEntity);

		if (interaction == nullptr)
			ERROR_QUIT("ERROR!! no interaction component on current VandingMachine entity");

		InteractionFuncion withVanding = [manager](Entity* player, Entity* vandingMachine) {
			// todo
			// ui show here
			// in ui
			//		select can
			//		put can into player's inventory
			DebugPrint("Hit");
			// 테스트용 임시 코드
			//std::vector<Entity*> cans;
			//std::function<void(Drink*, SelfEntity*)> findDrink = [&cans](Drink* dr, SelfEntity* ent) { if (dr->isOccupied() == false) cans.push_back(ent->GetEntity()); };
			//manager->Execute(findDrink);
			
			// todo 임시
			//Inventory* playerInv = manager->GetComponent<Inventory>(player);
			//Holdable* holdable = manager->GetComponent<Holdable>(cans.front());
			//holdable->SetMaster(player);
			//playerInv->AddItem(cans.front(), 3);

			//Entity* curhold = inven->GetCurrentHoldingItem();
			//if (curhold != nullptr) {
			//	KeyTool* keytool = manager->GetComponent<KeyTool>(curhold);
			//	if (keytool != nullptr) {

			std::function<void(component::UICanvas*, component::UIVandingMachine*)> openVandingUI = [player](component::UICanvas* can, component::UIVandingMachine* vand) {
				vand->SetPlayer(player);
				can->ShowUI();
				};
			manager->Execute(openVandingUI);

			};

		interaction->SetInteractionFunction(withVanding);
	}

	void Drink::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value dr = v["Drink"];
		m_Type = dr["Type"].asInt();
		//m_InventorySocket += m_Type;
	}

	void Drink::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
	}

	void CreditCard::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void CreditCard::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
	}

	void Key::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value key = v["Key"];
	}

	void Key::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
	}

	void Key::ShowYourself() const
	{
	}

	void UICutLine::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void UICutLine::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		auto& children = selfEntity->GetChildren();

		UICanvas* canvas = manager->GetComponent<UICanvas>(selfEntity);
		UICutLine* cutline = manager->GetComponent<UICutLine>(selfEntity);

		for (Entity* child : children) {
			Name* childName = manager->GetComponent<Name>(child);

			// 확인 버튼
			if (childName != nullptr) {
				std::string name = childName->getName();

				if (name == "Exit") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction exit = [canvas](Entity* ent) {
						// hide ui;
						canvas->HideUI();

						};
					button->SetButtonEvent(KEY_STATE::END_PRESS, exit);
				}
				if (name == "Check") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction check = [canvas, cutline, manager](Entity* ent) {
						// hide ui;
						DebugPrint(std::format("pw: {}", cutline->GetCurrent()));
						Entity* door = cutline->GetDoor();
						DoorControl* doorCtrl = manager->GetComponent<DoorControl>(door);
						if (cutline->GetAnswer() == cutline->GetCurrent()) {
							// open door
							doorCtrl->SetLock(false);
							canvas->HideUI();
						}
						else {
							doorCtrl->SetUioff(true);
							canvas->HideUI();
						}

						};
					button->SetButtonEvent(KEY_STATE::END_PRESS, check);
				}
				else {
					for (int i = 1; i <= 4; ++i) { //1,2,3,4 == red, blue, green, yellow
						std::string buttonName = std::format("Line{}", i);
						if (name == buttonName) {
							Button* button = manager->GetComponent<Button>(child);
							ButtonEventFunction password = [canvas, cutline, manager, i](Entity* ent) { // 버튼에 대한 콜백함수 등록
								int current = cutline->GetCurrent();
								current = current * 10 + i;
								cutline->SetCurrent(current);

								};
							button->SetButtonEvent(KEY_STATE::END_PRESS, password);
						}
					}
				}
			}
		}
	}

	void UICutLine::ShowYourself() const
	{
	}

	void KeyTool::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value kt = v["KeyTool"];

		memset(m_Keys, -1, sizeof(m_Keys));
		m_SoundMakingMinimum = kt["SoundMakingMinimum"].asInt();

	}

	void KeyTool::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Holdable* holdable = manager->GetComponent<Holdable>(selfEntity);

		if (holdable == nullptr) {
			Name* name = manager->GetComponent<Name>(selfEntity);
			ERROR_QUIT(std::format("ERROR!!! no Holdable component in this entity, entity name: {}\n태양이 두개면 시원한 이유는?\nsun sun 해지니까", name->getName()));
		}

		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::MOUSE_LEFT, KEY_STATE::START_PRESS), [manager, holdable, selfEntity, this](float deltaTime) {
			Entity* master = holdable->GetMaster();
			Pawn* masterPawn = manager->GetComponent<Pawn>(master);
			Entity* interactionEntity = masterPawn->GetInteractionEntity();

			if (interactionEntity != nullptr) {
				DoorControl* door = manager->GetComponent<DoorControl>(interactionEntity);
				if (door != nullptr) {
					// check answer here
					int doorAnswer = door->GetAnswer();
					if (IsKeyInKeyTool(doorAnswer)) {
						// 잠금 해제
						door->SetLock(false);
						DeleteKey(doorAnswer);
					}
				}
			}

		});
	}

	bool KeyTool::InsertKey(int keyAnswer)
	{
		for (int i = 0; i < _countof(m_Keys); ++i) {
			if (i == keyAnswer) {
				m_Keys[i] = keyAnswer;
				++m_CurrentHolding;
				return true;
			}
		}

		return false;
	}

	bool KeyTool::DeleteKey(int keyAnswer)
	{
		for (int i = 0; i < _countof(m_Keys); ++i) {
			if (m_Keys[i] == keyAnswer) {
				m_Keys[i] = -1;
				--m_CurrentHolding;
				return true;
			}
		}

		return false;
	}

	bool KeyTool::IsKeyInKeyTool(int keyAnswer) const
	{
		for (int i = 0; i < _countof(m_Keys); ++i)
			if (m_Keys[i] == keyAnswer)
				return true;

		return false;
	}

	void RCController::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value rc = v["RCController"];

		m_TargetRC = rc["Target"].asString().c_str();
	}

	void RCController::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		// get camera render target indices
		m_RCEntity = manager->GetEntity(m_TargetRC);

		// set holdable action maps
		Holdable* holdable = manager->GetComponent<Holdable>(selfEntity);

		float& angle = m_GoRotateAngle;
		const float c_RcMoveSpeed = 1.0f;
		const float c_RotateSpeed = 30.0f;

		// rotate by arrow keys
		ActionFunction arrowUp = [this, manager, c_RcMoveSpeed](float deltaTime) {
			//DebugPrint(std::format("angle: {}", m_GoRotateAngle));
			Transform* tr = manager->GetComponent<Transform>(m_RCEntity);
			Physics* py = manager->GetComponent<Physics>(m_RCEntity);

			XMVECTOR d = { 0.0f, 0.0f, 1.0f, 0.0f };
			XMFLOAT4X4 temp = tr->GetWorldTransform();
			XMMATRIX mat = XMLoadFloat4x4(&temp);

			d = XMVector4Transform(d, mat);

			XMFLOAT3 move;
			XMStoreFloat3(&move, d);
			move.y = 0.0f;

			XMStoreFloat3(&move, XMVector3Normalize(XMLoadFloat3(&move)));
			py->AddVelocity(move, deltaTime * c_RcMoveSpeed);

			XMFLOAT3 velocity = py->GetVelocity();
			XMFLOAT3 velocityOnXZ = py->GetVelocityOnXZ();
			XMVECTOR vel = XMLoadFloat3(&velocityOnXZ);
			float curSpeed = XMVectorGetX(XMVector3Length(vel));
			float maxSpeed = py->GetMaxVelocity();

			// limit max speed
			if (curSpeed >= maxSpeed)
			{
				vel = vel / curSpeed * maxSpeed;
				velocity.x = XMVectorGetX(vel);
				velocity.z = XMVectorGetZ(vel);
				py->SetVelocity(velocity);
			}

			};
		ActionFunction arrowDown = [this, manager, c_RcMoveSpeed](float deltaTime) {
			//DebugPrint(std::format("angle: {}", m_GoRotateAngle));
			Transform* tr = manager->GetComponent<Transform>(m_RCEntity);
			Physics* py = manager->GetComponent<Physics>(m_RCEntity);

			XMVECTOR d = { 0.0f, 0.0f, 1.0f, 0.0f };
			XMFLOAT4X4 temp = tr->GetWorldTransform();
			XMMATRIX mat = XMLoadFloat4x4(&temp);

			d = XMVector4Transform(-d, mat);

			XMFLOAT3 move;
			XMStoreFloat3(&move, d);
			move.y = 0.0f;
			
			XMStoreFloat3(&move, XMVector3Normalize(XMLoadFloat3(&move)));
			py->AddVelocity(move, deltaTime * c_RcMoveSpeed);

			XMFLOAT3 velocity = py->GetVelocity();
			XMFLOAT3 velocityOnXZ = py->GetVelocityOnXZ();
			XMVECTOR vel = XMLoadFloat3(&velocityOnXZ);
			float curSpeed = XMVectorGetX(XMVector3Length(vel));
			float maxSpeed = py->GetMaxVelocity();

			// limit max speed
			if (curSpeed >= maxSpeed)
			{
				vel = vel / curSpeed * maxSpeed;
				velocity.x = XMVectorGetX(vel);
				velocity.z = XMVectorGetZ(vel);
				py->SetVelocity(velocity);
			}
			};
		ActionFunction dirDrift = [this, manager, c_RotateSpeed](float deltaTime) {
			Transform* tr = manager->GetComponent<Transform>(m_RCEntity);
			Physics* py = manager->GetComponent<Physics>(m_RCEntity);
			
			tr->GetRotationPtr()->y += py->GetCurrentVelocityLenOnXZ() * m_GoRotateAngle / c_RotateSpeed * deltaTime;
			};


		ActionFunction dirLeft = [&angle](float deltaTime) { angle -= 45.0f; };
		ActionFunction dirRight = [&angle](float deltaTime) { angle += 45.0f; };



		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_UP, KEY_STATE::PRESSING), arrowUp);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_DOWN, KEY_STATE::PRESSING), arrowDown);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_LEFT, KEY_STATE::START_PRESS), dirLeft);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_LEFT, KEY_STATE::END_PRESS), dirRight);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_LEFT, KEY_STATE::PRESSING), dirDrift);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_RIGHT, KEY_STATE::START_PRESS), dirRight);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_RIGHT, KEY_STATE::END_PRESS), dirLeft);
		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::ARROW_RIGHT, KEY_STATE::PRESSING), dirDrift);
	}

	void UITreasureChest::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value rc = v["UITreasureChest"];

		rm->AddLateLoadUI("UI/UITempMaterial", nullptr);
		rm->AddLateLoadUI("UI/UIKey1", nullptr);
		rm->AddLateLoadUI("UI/UIKey2", nullptr);
		rm->AddLateLoadUI("UI/UIOld_Key", nullptr);
		rm->AddLateLoadUI("UI/UIRing_Key", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Black", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Blue", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Green", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Pink", nullptr);
		rm->AddLateLoadUI("UI/UIFinalChestKey_Red", nullptr);
		rm->AddLateLoadUI("UI/UIX", nullptr);
	}

	void UITreasureChest::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		// todo
		// 남은 열쇠 갯수 1
		m_KeyMaterialMap = std::map<int, int>();
		m_KeyMaterialMap[0] = rm->GetMaterial("UITempMaterial");
		m_KeyMaterialMap[1] = rm->GetMaterial("UIKey1");
		m_KeyMaterialMap[2] = rm->GetMaterial("UIKey2");
		m_KeyMaterialMap[3] = rm->GetMaterial("UIOld_Key");
		m_KeyMaterialMap[4] = rm->GetMaterial("UIRing_Key");
		m_KeyMaterialMap[5] = rm->GetMaterial("UIFinalChestKey_Black");
		m_KeyMaterialMap[6] = rm->GetMaterial("UIFinalChestKey_Blue");
		m_KeyMaterialMap[7] = rm->GetMaterial("UIFinalChestKey_Green");
		m_KeyMaterialMap[8] = rm->GetMaterial("UIFinalChestKey_Pink");
		m_KeyMaterialMap[9] = rm->GetMaterial("UIFinalChestKey_Red");

		m_KeyMaterialMap[-1] = rm->GetMaterial("UIX");

		UICanvas* canvas = manager->GetComponent<UICanvas>(selfEntity);
		UITreasureChest* doorkey = manager->GetComponent<UITreasureChest>(selfEntity);
		auto& children = selfEntity->GetChildren();

		for (Entity* child : children) {
			Name* childName = manager->GetComponent<Name>(child);

			// 확인 버튼
			if (childName != nullptr) {
				std::string name = childName->getName();

				if (name == "Exit") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction exit = [canvas, manager, this](Entity* ent) {
						DebugPrint("exit");
						// set result answer
						//int newAnswer = 0;
						//for (int idx = 0; idx < _countof(m_Answer); ++idx) {
						//	if (m_Answer[idx] != -1) {
						//		newAnswer += m_Answer[idx];

						//		if (idx < _countof(m_Answer) - 1)
						//			newAnswer *= 10;
						//	}
						//}

						//DebugPrint(std::format("NewAnswer: {}", newAnswer));
						//DoorControl* doorCtrl = manager->GetComponent<DoorControl>(m_DoorEntity);
						//doorCtrl->SetAnswer(newAnswer);

						// hide ui;
						canvas->HideUI();

						};
					button->SetButtonEvent(KEY_STATE::END_PRESS, exit);
				}
				else if (name == "Check") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction check = [canvas, doorkey, manager, this](Entity* ent) {
						DebugPrint("Check");
						
						// check current in answer
						for (int i = 0; i < _countof(m_Answer); ++i) {
							// if find answer
							if (m_Answer[i] == m_Current) {
								// set answer [-1]
								m_Answer[i] = -1;

								// disable answer img
								SetAnswerButtonMaterial(i, -1);
							}
						}

						// set answer
						int newAnswer = 0;
						for (int idx = 0; idx < _countof(m_Answer); ++idx) {
							if (m_Answer[idx] != -1) {
								newAnswer += m_Answer[idx];

								if (idx < _countof(m_Answer) - 1)
									newAnswer *= 10;
							}
						}

						DoorControl* doorCtrl = manager->GetComponent<DoorControl>(m_DoorEntity);
						doorCtrl->SetAnswer(newAnswer);

						// success
						// open
						if (newAnswer == 0) {
							// open door
							Entity* door = doorkey->GetDoor();
							DoorControl* doorCtrl = manager->GetComponent<DoorControl>(door);
							doorCtrl->SetLock(false);
							doorCtrl->SetKeyDoorOpen(true);
							canvas->HideUI();

							// open final chest
							sc_packet_ending packet(2);
							Client::GetInstance().send_packet(&packet);
						}
						};
					button->SetButtonEvent(KEY_STATE::END_PRESS, check);
				}
				else {

					// answers
					for (int i = 0; i < FINAL_LOCK; ++i) {
						std::string renderName = std::format("AnswerKey{}", i);
						if (name == renderName) {
							m_AnswerUIrender[i] = manager->GetComponent<UIRenderer>(child);
						}
					}

					// buttons
					for (int i = 0; i < MAX_BUTTON; ++i) {
						std::string buttonName = std::format("KeyButton{}", i);
						if (name == buttonName) {
							Button* button = manager->GetComponent<Button>(child);
							UIRenderer* render = manager->GetComponent<UIRenderer>(child);
							m_AnswerbuttonUIrender[i] = render;
							Key* key = manager->GetComponent<Key>(child);
							m_keyAnswer[i] = key;
							ButtonEventFunction password = [key, doorkey, manager, this](Entity* ent) { m_Current = key->GetKeyID(); };
							button->SetButtonEvent(KEY_STATE::END_PRESS, password);
						}
					}
				}
			}
		}
	}

	void UITreasureChest::SetAnswer(int ans)
	{
		// reset to -1
		for (int i = 0; i < FINAL_LOCK; ++i) {
			m_Answer[i] = -1;
			m_AnswerUIrender[i]->SetMaterial(m_KeyMaterialMap[m_Answer[i]]);
		}

		DebugPrint("Answer start");
		DebugPrint(std::format("AnswerStart: {}", ans));
		// set answer
		for (int i = 0; i < FINAL_LOCK; ++i) {
			int answer = ans % 10;
			if (answer != 0)
				m_Answer[i] = answer;
			ans /= 10;
			DebugPrint(std::format("Answer: {}", m_Answer[i]));

			m_AnswerUIrender[i]->SetMaterial(m_KeyMaterialMap[m_Answer[i]]);

			if (ans == 0) break;
		}
		DebugPrint("Answer end");
		// set answer material auto
	}

	void UITreasureChest::SetAnswerButtonMaterial(int target, int answer)
	{
		m_AnswerbuttonUIrender[target]->SetMaterial(m_KeyMaterialMap[answer]);
	}

	void UITreasureChest::SetAnswerButton(int target, int answer)
	{
		m_keyAnswer[target]->SetKeyID(answer);
	}

	void UIVandingMachine::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void UIVandingMachine::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		std::function<bool(Entity*)> insertCan = [manager, this](Entity* can) {
			Inventory* playerInv = manager->GetComponent<Inventory>(m_PlayerEntity);
			Holdable* holdable = manager->GetComponent<Holdable>(can);
			Drink* drink = manager->GetComponent<Drink>(can);

			// if not occupied, insert
			int targetInvNum = drink->GetInvenSocket();
			if (playerInv->IsOccupied(targetInvNum)) {
				holdable->SetMaster(m_PlayerEntity);
				playerInv->AddItem(can, targetInvNum);
				return true;
			}
			return false;
		};

		auto& children = selfEntity->GetChildren();
		UICanvas* canvas = manager->GetComponent<UICanvas>(selfEntity);

		for (Entity* child : children) {
			Name* childName = manager->GetComponent<Name>(child);

			if (childName != nullptr) {
				std::string name = childName->getName();

				// Exit
				if (name == "Exit") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction exit = [canvas](Entity* ent) { 
						canvas->HideUI();
						};
					button->SetButtonEvent(KEY_STATE::END_PRESS, exit);
				}
				else {
					for (int i = 0; i < MAX_DRINK_TYPE; ++i) {
						std::string buttonName = std::format("Drink_0{}", i);

						if (name == buttonName) {
							Button* button = manager->GetComponent<Button>(child);
							int type = i;
							ButtonEventFunction getDrink = [manager, type, this, canvas](Entity* ent) {
								// find cans
								std::vector<Entity*> cans;
								std::function<void(Drink*, SelfEntity*)> findDrink = [&cans, type](Drink* dr, SelfEntity* ent) { 
									if (dr->isOccupied() == false && dr->GetType() == type)
										cans.push_back(ent->GetEntity()); 
									};
								manager->Execute(findDrink);

								// if cans over 1, insert can
								if (cans.empty() == false) {
									Entity* can = cans.front();
									Inventory* playerInv = manager->GetComponent<Inventory>(m_PlayerEntity);
									Transform* playerTr = manager->GetComponent<Transform>(m_PlayerEntity);
									Holdable* holdable = manager->GetComponent<Holdable>(can);
									Drink* drink = manager->GetComponent<Drink>(can);

									int targetInvNum = drink->GetInvenSocket();
									if (playerInv->IsOccupied(targetInvNum) == false) {
										holdable->SetMaster(m_PlayerEntity);
										playerInv->AddItem(can, targetInvNum);
										drink->SetOccupied(true);
										// success
										// send inventory in

										// if self
										Server* servComp = manager->GetComponent<Server>(m_PlayerEntity);
										if (servComp->getID() == Client::GetInstance().getPSock()[0]) {
											cs_packet_get_item packet(holdable->GetHoldableID(), targetInvNum);
											Client::GetInstance().send_packet(&packet);
										}

										// play sound
										FMOD_INFO::GetInstance().play_unloop_sound(playerTr->GetWorldPosition(), SOUND_TYPE::DRINK_BUY, "DRINK_BUY");

										cs_packet_sound_start packet(playerTr->GetWorldPosition(), SOUND_TYPE::DRINK_BUY);
										Client::GetInstance().send_packet(&packet);

										// hide ui
										canvas->HideUI();
									}
								}

								};
							button->SetButtonEvent(KEY_STATE::END_PRESS, getDrink);
						}

					}
				}
			}

		}



	}

	void Consumable::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value con = v["Consumable"];

		m_ConsumType = con["Type"].asInt();
	}

	void Consumable::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Holdable* holdable = manager->GetComponent<Holdable>(selfEntity);
		ActionFunction action;
		switch (m_ConsumType) {
			// speed up
		case 1:
			action = [selfEntity, holdable, manager](float deltaTime) {
				Entity* master = holdable->GetMaster();
				Physics* masterPhy = manager->GetComponent<Physics>(master);
				Inventory* masterInv = manager->GetComponent<Inventory>(master);
				Transform* masterTr = manager->GetComponent<Transform>(master);

				// set speed up
				float* masterMaxSpeed = masterPhy->GetMaxVelocityPtr();

				float speed = masterPhy->GetOriginalMaxVelocity();
				TimeLine<float>* speedUp = new TimeLine<float>(masterMaxSpeed);
				speedUp->AddKeyFrame(speed * 3.0f, 0.0f);
				speedUp->AddKeyFrame(speed * 3.0f, 35.0f);
				speedUp->AddKeyFrame(speed + speed, 35.0f);
				speedUp->AddKeyFrame(speed, 40.0f);
				manager->AddTimeLine(speedUp);

				// detach self
				masterInv->EraseCurrentHolding();
				manager->AttachChild(holdable->GetOriginParent(), selfEntity);
				//manager->AttachChild(holdable->GetOriginParent(), selfEntity);

				FMOD_INFO::GetInstance().play_unloop_sound(masterTr->GetWorldPosition(), SOUND_TYPE::DRINK_CONSUME, "ConsumeDr");

				cs_packet_sound_start packet(masterTr->GetWorldPosition(), SOUND_TYPE::DRINK_CONSUME);
				Client::GetInstance().send_packet(&packet);
				};
			break;

			// no sound
		case 2:
			action = [selfEntity, holdable, manager](float deltaTime) {
				Entity* master = holdable->GetMaster();
				Inventory* masterInv = manager->GetComponent<Inventory>(master);
				
				// todo player component
				// play sound to zero

				// todo sound
				// play consume drink

				// detach self
				manager->AttachChild(holdable->GetOriginParent(), selfEntity);
				masterInv->EraseCurrentHolding();
				};
			break;

			// default error but keep going
		default:
			ERROR_QUIT("error!! no consumable type");
			action = [](float deltaTime) {};
			break;
		}

		holdable->SetAction(Input_State_In_LongLong(GAME_INPUT::MOUSE_LEFT, KEY_STATE::END_PRESS), action);
	}

	void UIEnding::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value end = v["UIEnding"];
		// 0: 체포 , 1: 타임오버, 2: 게임성공, 3: 도주


		for (int i = 0; i < MAX_ENDINGS; ++i) {
			m_EndingImageStrings[i] = end[std::format("{}", i)].asString();
			std::string loadUI = std::format("UI/{}", m_EndingImageStrings[i]);
			rm->AddLateLoadUI(loadUI, nullptr);
		}
	}

	void UIEnding::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		m_EndingImageRenderer = manager->GetComponent<UIRenderer>(manager->GetEntityInChildren("EndingImage", selfEntity));

		for (int i = 0; i < MAX_ENDINGS; ++i)
			m_EndingImageMaterials[i] = rm->GetMaterial(m_EndingImageStrings[i]);

		// set button event
		Entity* buttonEnt = manager->GetEntityInChildren("ReturnToLobby", selfEntity);
		Button* returnLobby = manager->GetComponent<Button>(buttonEnt);
		UICanvas* canv = manager->GetComponent<UICanvas>(selfEntity);

		ButtonEventFunction goLobby = [canv](Entity* self) {
			canv->HideUI();
			Application::GetInstance().GetSceneManager()->SetToChangeScene("Test", 0);
			};

		returnLobby->SetButtonEvent(KEY_STATE::END_PRESS, goLobby);
	}

	void UIEnding::SetEndingImage(int endingType)
	{
		m_EndingImageRenderer->SetMaterial(m_EndingImageMaterials[endingType]);
	}

	void StartInteraction::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		Interaction* interaction = manager->GetComponent<Interaction>(selfEntity);

		bool& ready = m_Ready;

		InteractionFuncion inter = [manager, ready](Entity* player, Entity* self) {
			// todo
			// show Conversation UI

			std::function<void(UICanvas*, UIConversation*)> showConvers = [manager, ready](UICanvas* canv, UIConversation* conver) {

				if (ready == false) 
				{
					// Object/UIConversation.json 참고
					conver->SetConversation(1);
					conver->SetTalker(1);

					// if not ready, 
					std::function<void()> openLoginUI = [manager, canv]() {
						canv->HideUI();
						std::function<void(UICanvas*, UILogin*)> showLogin = [manager](UICanvas* canv, UILogin* login) {
							login->SetRoomNum(0);
							canv->ShowUI();
							};
						manager->Execute(showLogin);
						};

					TimeLine<float>* toChangeOtherUI = new TimeLine<float>(nullptr);
					toChangeOtherUI->AddKeyFrame(0, 0.0f);
					toChangeOtherUI->AddKeyFrame(1, 3.0f);
					toChangeOtherUI->SetEndEvent(openLoginUI);
					manager->AddTimeLine(toChangeOtherUI);
				}
				else {
					// todo "기다려" 재생 해야함
					conver->SetConversation(0);
					conver->SetTalker(1);

					// hide ui
					std::function<void()> hideUI = [canv] () { canv->HideUI(); };

					TimeLine<float>* toChangeOtherUI = new TimeLine<float>(nullptr);
					toChangeOtherUI->AddKeyFrame(0, 0.0f);
					toChangeOtherUI->AddKeyFrame(1, 3.0f);
					toChangeOtherUI->SetEndEvent(hideUI);
					manager->AddTimeLine(toChangeOtherUI);
				}

				canv->ShowUI();

				};
			manager->Execute(showConvers);


			// wait for timeline end
			// show MakeOrJoinRoom UI
			//ready = true;

			};

		interaction->SetInteractionFunction(inter);
	}

	void UIConversation::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value conv = v["UIConversation"];

		// conversations
		m_NumOfConversations = conv["MaxConversation"].asInt();
		m_ConversationsString = new std::string[m_NumOfConversations];

		for (int i = 0; i < m_NumOfConversations; ++i) {
			m_ConversationsString[i] = conv["Conversation"][i].asString();
			rm->AddLateLoadUI(std::format("UI/Conversation/{}", m_ConversationsString[i]), nullptr);
		}
		
		// talkers
		m_NumOfTalkers = conv["MaxTalker"].asInt();
		m_TalkersString = new std::string[m_NumOfTalkers];

		for (int i = 0; i < m_NumOfTalkers; ++i) {
			m_TalkersString[i] = conv["Talker"][i].asString();
			rm->AddLateLoadUI(std::format("UI/Conversation/{}", m_TalkersString[i]), nullptr);
		}
	}

	void UIConversation::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{
		m_ConversationsMaterialIdx = std::vector<int>();
		m_TalkersMaterialIdx = std::vector<int>();
		m_ConversationsMaterialIdx.reserve(m_NumOfConversations);
		m_TalkersMaterialIdx.reserve(m_NumOfTalkers);

		// conversations
		for (int i = 0; i < m_NumOfConversations; ++i) 
			m_ConversationsMaterialIdx.push_back(rm->GetMaterial(m_ConversationsString[i]));

		// talkers
		for (int i = 0; i < m_NumOfTalkers; ++i)
			m_TalkersMaterialIdx.push_back(rm->GetMaterial(m_TalkersString[i]));

		m_ConversationUI = manager->GetComponent<UIRenderer>(manager->GetEntityFromRoute("Conversation", selfEntity));
		m_TalkerUI = manager->GetComponent<UIRenderer>(manager->GetEntityFromRoute("Talker", selfEntity));

		delete[] m_ConversationsString;
		delete[] m_TalkersString;
	}

	void UIConversation::SetConversation(int convIdx)
	{
		m_ConversationUI->SetMaterial(m_ConversationsMaterialIdx[convIdx]);
	}

	void UIConversation::SetTalker(int talkerIdx)
	{
		m_TalkerUI->SetMaterial(m_TalkersMaterialIdx[talkerIdx]);
	}

	void UILogin::OnStart(Entity* selfEntity, ECSManager* manager, ResourceManager* rm)
	{

		auto& children = selfEntity->GetChildren();

		UICanvas* canvas = manager->GetComponent<UICanvas>(selfEntity);

		for (Entity* child : children) {
			Name* childName = manager->GetComponent<Name>(child);
			auto& name = childName->getName();

			if (name == "Exit") {
				Button* button = manager->GetComponent<Button>(child);
				ButtonEventFunction exit = [canvas](Entity* ent) {canvas->HideUI(); };
				button->SetButtonEvent(KEY_STATE::END_PRESS, exit);
			}
			else if (name == "Host") {
				Button* button = manager->GetComponent<Button>(child);
				ButtonEventFunction makeroomButton = [canvas, manager](Entity* ent) {
					canvas->HideUI();
					//if (Client::GetInstance().getPSock()[0])
					{
						Application::GetInstance().GetSceneManager()->PossessPlayer(true);
						Client::GetInstance().Send_Room(pMAKEROOM, NULL);

						// set start inventory to ready
						std::function<void(StartInteraction*)> ready = [](StartInteraction* si) { si->SetReady(true); };
						manager->Execute(ready);

					}
					};
				button->SetButtonEvent(KEY_STATE::END_PRESS, makeroomButton);
			}
			else if (name == "JoinRoom") {
				Button* button = manager->GetComponent<Button>(child);
				ButtonEventFunction joinRoom = [canvas, this, manager](Entity* ent) {
					canvas->HideUI();
					//if (Client::GetInstance().getPSock()[0])
					{
						Application::GetInstance().GetSceneManager()->PossessPlayer(false);
						Client::GetInstance().Send_Room(pENTERROOM, m_ToJoinRoom);

						// set start inventory to ready
						std::function<void(StartInteraction*)> ready = [](StartInteraction* si) { si->SetReady(true); };
						manager->Execute(ready);
					}
					};
				button->SetButtonEvent(KEY_STATE::END_PRESS, joinRoom);
			}
			else {
				// number buttons
				for (int i = 0; i <= 9; ++i) {
					std::string buttonName = std::format("{}Button", i);
					if (name == buttonName) {
						Button* button = manager->GetComponent<Button>(child);
						ButtonEventFunction roomNumBut = [this, manager, i](Entity* ent) { m_ToJoinRoom = m_ToJoinRoom * 10 + i; };
						button->SetButtonEvent(KEY_STATE::END_PRESS, roomNumBut);
					}
				}
			}
		}

	}

}
