#pragma once
#include "ManagementComponents.h"

namespace component { class Component; }


// entity
class Entity
{
	friend class ECSSystem<COMPONENT_COUNT>;

public:
	Entity(int id) : m_Id{ id } {}
	~Entity() {
		//for (auto& comp : m_Components)
		//	delete comp;
		//for (auto& child : m_Children)
		//	delete child;
	}

	void AddComponent(component::Component* component) { m_Components.push_back(component); }
	void AddChild(Entity* entity) { m_Children.push_back(entity); }

private:
	// 컴포넌트 소유 bitset
	std::bitset<COMPONENT_COUNT> m_Bitset;

	// bitset 내부 id
	int m_Id = -1;

	// 소유주는 리소스매니저
	std::vector<component::Component*> m_Components;

	// 소유주는 리소스매니저
	std::vector<Entity*> m_Children;
};


