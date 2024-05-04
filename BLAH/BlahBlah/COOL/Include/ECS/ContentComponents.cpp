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

	void DoorControl::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value door = v["Door"];
	}

	void DoorControl::ShowYourself() const
	{
	}


}
