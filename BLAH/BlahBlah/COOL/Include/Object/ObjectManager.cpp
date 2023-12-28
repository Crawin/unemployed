#include "framework.h"
#include "ObjectManager.h"
#include "ObjectBase.h"

//void ObjectManager::InsertObject(ObjectBase* obj)
//{
//	obj->SetIndex(m_NextID++);
//	m_Objects.push_back(obj);
//}

ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
	for (int i = 0; i < m_Objects.size(); ++i) {
		if (m_Objects[i] != nullptr) {
			delete m_Objects[i];
		}
	}
}

bool ObjectManager::LoadFolder(const std::string& pathName)
{
	if (std::filesystem::exists(pathName) == false) {
		DebugPrint(std::format("ERROR!! no such path!!, path: {}", pathName));
	}


	return true;
}

ObjectBase* ObjectManager::GetObjectFromName(const std::string& name)
{
	for (auto obj : m_Objects) {
		if (obj->GetName() == name) {
			return obj;
		}
	}

	return nullptr;
}

void ObjectManager::Update(float deltaTime)
{
	for (int i = 0; i < m_NextID; ++i) {
		m_Objects[i]->Update(deltaTime);

		// obb 업데이트
		//XMMATRIX matrix = XMLoadFloat4x4(&m_ObjectWorldMatrices[i]);
		//m_ObjectOBBs[i].Transform(m_ObjectOBBs[i], matrix);
	}

}
