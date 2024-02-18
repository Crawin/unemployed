#include "framework.h"
#include "Component.h"
#include "Scene/ResourceManager.h"
#include <json/json.h>

namespace component 
{
	void Name::Create(Json::Value& v, ResourceManager* rm)
	{
		m_Name = v["Name"].asString();
	}

	void Renderer::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value rend = v["Renderer"];

		//m_MeshID = rm->GetMeshToLoad(rend["Mesh"].asString());

		// todo
		// 일단 전부 late load로 해두긴 했는데
		// 추후에 이걸 놔두어도 될지 확인
		rm->AddLateLoad(rend["Mesh"].asString(), rend["Material"].asString(), this);

		// get failed
		//if (m_MeshID == -1) 
		//{
		//	// add to late load
		//	
		//}

		//m_MaterialID = rm->GetMaterialToLoad(rend["Material"].asString());

	}

	void Transform::Create(Json::Value& v, ResourceManager* rm)
	{
		Json::Value trans = v["Transform"];

		m_Position.x = trans["Position"][0].asFloat();
		m_Position.y = trans["Position"][1].asFloat();
		m_Position.z = trans["Position"][2].asFloat();

		m_Rotate.x = trans["Rotate"][0].asFloat();
		m_Rotate.y = trans["Rotate"][1].asFloat();
		m_Rotate.z = trans["Rotate"][3].asFloat();
		m_Rotate.w = trans["Rotate"][4].asFloat();

		m_Position.x = trans["Position"][0].asFloat();
		m_Position.y = trans["Position"][1].asFloat();
		m_Position.z = trans["Position"][2].asFloat();
	}

}
