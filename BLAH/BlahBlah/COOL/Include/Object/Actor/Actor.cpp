#include "framework.h"
#include "Actor.h"
#include "Object/ObjectManager.h"
#include <json/json.h>

Actor::Actor(Json::Value& root, ObjectManager* manager)
	: ObjectBase(root, manager)
{
	// material과 mesh만 set 해주면 됨

	//manager->FindMesh
}

Actor::~Actor()
{
}
