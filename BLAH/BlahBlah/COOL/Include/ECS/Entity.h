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
	void AddBit(const COMP_BITSET& bit) { 
		m_Bitset |= bit;
	}

public:
	const std::list<Entity*>& GetChildren() const { return m_Children; }

	void AddChild(Entity* entity);
	void EraseChild(Entity* ent);

	void SetParent(Entity* ent) { m_ParentEntity = ent; }

	COMP_BITSET GetBitset() const { return m_Bitset; }
	int GetInnerID() const { return m_Id; }
	Entity* GetParent() const { return m_ParentEntity; }

private:
	// 컴포넌트 소유 bitset
	COMP_BITSET m_Bitset;

	// bitset 내부 id
	int m_Id = -1;

	// 소유주는 리소스매니저
	std::vector<component::Component*> m_Components;

	// 소유주는 리소스매니저
	std::list<Entity*> m_Children;

	Entity* m_ParentEntity = nullptr;
};


