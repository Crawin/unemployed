#pragma once
#include "Object/ObjectBase.h"

class Json::Value;
class ObjectManager;

class Actor :
    public ObjectBase
{
public:
	Actor(Json::Value& root, ObjectManager* manager);
	virtual ~Actor();

private:

	// 없으면 -1
	int m_MeshID = -1;
	// 없으면 -1
	int m_MaterialID = -1;

public:
	void SetMesh(int mesh) { m_MeshID = mesh; }
	void SetMaterial(int material) { m_MaterialID = material; }
};

