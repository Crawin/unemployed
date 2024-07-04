#include "framework.h"
#include "ContentComponents.h"
#include "Scene/ResourceManager.h"
#include "ECS/ECSManager.h"
#include "json/json.h"



namespace component {
	void PlayerAnimControll::Create(Json::Value& v, ResourceManager* rm)
	{
	}

	void PlayerAnimControll::ShowYourself() const
	{
		DebugPrint("PlayerAnimControll Comp");
	}

	void DiaAnimationControl::Create(Json::Value& v, ResourceManager* rm)
	{
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
				if (name == "Exit") {
					Button* button = manager->GetComponent<Button>(child);
					ButtonEventFunction exit = [canvas](Entity* ent) {
						canvas->HideUI();
						};
					button->SetButtonReleaseEvent(exit);
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
					button->SetButtonReleaseEvent(check);
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
							button->SetButtonReleaseEvent(password);
						}
					}
				}
			}
		}

	}

	void UIKeypad::ShowYourself() const
	{
	}

}
