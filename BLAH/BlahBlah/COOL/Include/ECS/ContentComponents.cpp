#include "framework.h"
#include "ContentComponents.h"
#include "Scene/ResourceManager.h"
#include <json/json.h>



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
	}

	void DoorControl::ShowYourself() const
	{
	}


	void UIKeypad::Create(Json::Value& v, ResourceManager* rm)
	{
		// nothing to do
	}

	void UIKeypad::OnStart(ECSManager* manager, ResourceManager* rm)
	{
		// 여기서 자신의 자식들을 돌며 해당 버튼이 눌렸을 때 본인의 answer가 올라가게 만들어 두어야 함
	}

	void UIKeypad::ShowYourself() const
	{
	}

}
