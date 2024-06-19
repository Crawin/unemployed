#pragma once
//#include "ManagementComponents.h"

namespace component { class Component; }


// entity
class Entity
{
	friend class ResourceManager;
	friend class ECSManager;
	friend class ComponentSet;

public:
	Entity() {}
	~Entity() {
		//for (auto& comp : m_Components)
		//	delete comp;
		//for (auto& child : m_Children)
		//	delete child;
	}

private:
	void AddComponent(component::Component* component) { m_Components.push_back(component); }
	void AddChild(Entity* entity) { m_Children.push_back(entity); }
	void AddBit(const COMP_BITSET& bit) { 
		m_Bitset |= bit;
	}

public:
	const std::vector<Entity*>& GetChildren() const { return m_Children; }

	COMP_BITSET GetBitset() const { return m_Bitset; }

	int GetInnerID() const { return m_Id; }

private:
	// 컴포넌트 소유 bitset
	COMP_BITSET m_Bitset;

	// bitset 내부 id
	int m_Id = -1;

	// 소유주는 리소스매니저
	std::vector<component::Component*> m_Components;

	// 소유주는 리소스매니저
	std::vector<Entity*> m_Children;
};


