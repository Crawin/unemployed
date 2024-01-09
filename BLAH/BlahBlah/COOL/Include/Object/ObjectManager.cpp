#include "framework.h"
#include "ObjectManager.h"
#include "ObjectBase.h"
#include <json/json.h>

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

bool ObjectManager::LoadFile(const std::string& fileName)
{
	DebugPrint(std::format("name: {}", fileName));

	std::ifstream file(fileName);

	if (file.is_open() == false) {
		DebugPrint(std::format("file open failed!! file name: {}", ExtractFileName(fileName)));
		return false;
	}

	Json::Reader reader;
	Json::Value root;

	reader.parse(file, root);
	
	ObjectBase* obj = nullptr;

	// 여기에 파일 로드
	switch (root["type"].asInt()) {
	case 0:
	default:
	{
		// default, ObjectBase
		//obj = new ObjectBase(root, );
		break;
	}
	case 1:
	{
		// actor

		break;
	}
	}
	


	return true;
}

bool ObjectManager::LoadFolder(const std::string& pathName)
{
	if (m_MaterialManager == nullptr || m_MeshManager == nullptr) {
		DebugPrint("ERROR!! no manager registered!!");
		return false;
	}

	if (std::filesystem::exists(pathName) == false) {
		DebugPrint(std::format("ERROR!! no such path!!, path: {}", pathName));
	}

	DebugPrint(std::format("Object Directory ::::::: {}", pathName));
	for (const auto& file : std::filesystem::directory_iterator(pathName)) {
		if (file.is_directory()) continue;

		std::string fileName = pathName + file.path().filename().string();
		CHECK_CREATE_FAILED(LoadFile(fileName), "File Load Failed!!");
		//if (LoadFile(fileName) == false) return false;
	}

	DebugPrint("\n");

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
