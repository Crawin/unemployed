#pragma once
#include "Object/ObjectBase.h"

class Actor :
    public ObjectBase
{
	// 없으면 -1
	int m_MeshID = -1;
	// 없으면 -1
	int m_MaterialID = -1;

public:
	void SetMesh(int mesh) { m_MeshID = mesh; }
	void SetMaterial(int material) { m_MaterialID = material; }
};

