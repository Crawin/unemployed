#include "framework.h"
#include "Actor.h"
#include "Scene/ResourceManager.h"
#include <json/json.h>

Actor::Actor(Json::Value& root, ResourceManager* manager)
	: ObjectBase(root, manager)
{
	// material과 mesh만 set 해주면 됨

	//manager->FindMesh
}

Actor::~Actor()
{
}
