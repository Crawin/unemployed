#include "../framework.h"
#include "ObjectManager.h"
#include "ObjectBase.h"

void ObjectManager::InsertObject(ObjectBase* obj)
{
	obj->SetIndex(m_NextID++);
	m_Objects.push_back(obj);
}

void ObjectManager::Update(float deltaTime)
{
	for (int i = 0; i < m_NextID; ++i) {
		m_Objects[i]->Update(deltaTime);

		// obb 업데이트
		XMMATRIX matrix = XMLoadFloat4x4(&m_ObjectWorldMatrices[i]);
		m_ObjectOBBs[i].Transform(m_ObjectOBBs[i], matrix);
	}

}
