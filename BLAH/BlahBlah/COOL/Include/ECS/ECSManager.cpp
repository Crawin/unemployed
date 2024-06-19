﻿#include "framework.h"
#include "ECSManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Container
/////////////////////////////////////////////////////////////////////////////////////////////////////

ComponentContainer::ComponentContainer(size_t stride)
	: m_Stride{ stride }
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Set
/////////////////////////////////////////////////////////////////////////////////////////////////////

inline ComponentSet::ComponentSet(COMP_BITSET bit)
{
	// 미리 해당 bitset에 맞춰 componentcontainer를 만들어둔다.
	for (int i = 0; i < COMPONENT_COUNT; ++i) {
		if (bit.test(i)) {
			// add
			size_t componentSize = GET_COMP_SIZE(i);
			
			COMP_BITSET tempBit(1);
			tempBit <<= i;

			m_Set.emplace(tempBit, componentSize);
		}
	}
}

void ComponentSet::InsertComponent(component::Component* comp)
{
	COMP_BITSET bit = comp->GetBitset();

	// check the component is exist in the set
	if (m_Set.contains(bit) == false) {
		ERROR_QUIT("ERROR!! not this component");
		//exit(1);
	}

	// insert component;
	m_Set[bit].push_back(comp);

}

void ComponentSet::InsertComponentByEntity(Entity* entity)
{
	for (auto& comp : entity->m_Components) 
	{
		comp->ShowYourself();
		InsertComponent(comp);

		// release before comp here
		delete comp;
	}

	entity->m_Components.clear();

	entity->m_Id = m_EntitySize++;
}

void ComponentSet::OnStart(Entity* ent, ECSManager* manager, ResourceManager* rm)
{
	int idx = ent->GetInnerID();

	for (auto& [key, val] : m_Set) {
		component::Component* comp = val.GetData<component::Component>(idx);
		comp->OnStart(ent, manager, rm);
	}

}


/////////////////////////////////////////////////////////////////////////////////////////////////////
// ECS System
/////////////////////////////////////////////////////////////////////////////////////////////////////

ECSManager::ECSManager()
{
}

ECSManager::~ECSManager()
{
	// todo delete 모두 잘 하는지 얘가 확인 해보자
	for (auto& ent : m_Entities)
		delete ent;

	for (auto& sys : m_Systems)
		delete sys;

	// component엔 동적할당 하지 않았음
}

void ECSManager::AddEntity(Entity* entity)
{
	// 1. 일단 entity가 어떤 component를 가지고 있는지 확인해야함
	COMP_BITSET bitset(0);
	for (auto& comp : entity->m_Components) 
		bitset |= comp->GetBitset();

	// 여기에 컴포넌트set 맞춰서 넣으면 된다.
	if (m_ComponentSets.contains(bitset) == false) 
	{
		// 없다면 만들어둔다
		m_ComponentSets.emplace(bitset, bitset);
	}

	DebugPrint(std::format("//////////////////////////entity bitset: {}", bitset.to_string()));
	m_ComponentSets[bitset].InsertComponentByEntity(entity);
	entity->m_Bitset = bitset;

	m_Entities.push_back(entity);
}

void ECSManager::AddToRoot(Entity* entity)
{
	m_RootEntities.push_back(entity);
}

void ECSManager::InitSystem()
{
	for (auto& system : m_Systems) {
		system->OnInit(this);
	}
}

void ECSManager::UpdateSystem(float deltaTime)
{
	for (auto& system : m_Systems) {
		system->Update(this, deltaTime);
	}
}

void ECSManager::OnStart(ResourceManager* rm)
{
	if (m_Started) return;
	m_Started = true;
	
	// for every entities
	for (Entity* ent : m_Entities) {
		// On Start

		COMP_BITSET entBitset = ent->GetBitset();

		COMP_BITSET bit(1);

		m_ComponentSets[entBitset].OnStart(ent, this, rm);
	}
}
