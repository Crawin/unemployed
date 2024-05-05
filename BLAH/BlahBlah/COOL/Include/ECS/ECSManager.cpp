#include "framework.h"
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

inline ComponentSet::ComponentSet(std::bitset<COMPONENT_COUNT> bit)
{
	// 미리 해당 bitset에 맞춰 componentcontainer를 만들어둔다.
	for (int i = 0; i < COMPONENT_COUNT; ++i) {
		if (bit.test(i)) {
			// add
			size_t componentSize = GET_COMP_SIZE(i);
			
			std::bitset<COMPONENT_COUNT> tempBit(1);
			tempBit <<= i;

			m_Set.emplace(tempBit, componentSize);
			
			//std::bitset bitset = Components::m_BIT;

		}
	}
}

void ComponentSet::InsertComponent(component::Component* comp)
{
	std::bitset<COMPONENT_COUNT> bit = comp->GetBitset();

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
	std::bitset<COMPONENT_COUNT> bitset(0);
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
