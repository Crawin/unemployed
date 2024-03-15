#include "framework.h"
#include "ECSManager.h"
#include "Entity.h"
#include "Component.h"
#include "ECS_System.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Container
/////////////////////////////////////////////////////////////////////////////////////////////////////

ComponentContainer::ComponentContainer(size_t stride)
	: m_Stride{ stride }
{
}

template<class T>
inline void ComponentContainer::push_back(const T* data)
{
	m_Data.resize(m_Data.size() + m_Stride);

	memcpy((&m_Data.front() + (m_Stride * m_Elements)), data, m_Stride);

	++m_Elements;
}

template<class T>
T* ComponentContainer::GetData(size_t index)
{
	// 포인터로 넘겨줘야 할듯하다
	return reinterpret_cast<T*>(&m_Data[index * m_Stride]);
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

template<class ...COMPONENTS>
inline void ComponentSet::Execute(std::function<void(COMPONENTS*...)>& func)
{
	for (int i = 0; i < m_EntitySize; ++i) 
	{
		// https://en.cppreference.com/w/cpp/language/foldx
		// fold expression, C++ 17
		func(GetComponent<COMPONENTS>(i)...);
	 
	}
}

template<class ...COMPONENTS>
void ComponentSet::Execute(int innerIdx, std::function<void(COMPONENTS*...)>& func)
{
	func(GetComponent<COMPONENTS>(innerIdx)...);
}

template<class COMP>
COMP* ComponentSet::GetComponent(int idx)
{
	//ComponentContainer& container = m_Set[bit];
	if (m_Set.contains(COMP::GetBit())) 
		return m_Set[COMP::GetBit()].GetData<COMP>(idx);

	return nullptr;
	//container[];
	//COMP* temp = container[idx];
	//return temp;
	//return nullptr;
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

inline void ECSManager::AddEntity(Entity* entity)
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

	//entity->m_Bitset = bitset;


}

void ECSManager::AddToRoot(Entity* entity)
{
	m_RootEntities.push_back(entity);
}

void ECSManager::UpdateSystem(float deltaTime)
{
	for (auto& system : m_Systems) {
		system->Update(this, deltaTime);
	}
}

template<class ...COMPONENTS>
inline void ECSManager::Execute(std::function<void(COMPONENTS*...)>& func)
{
	// find bitset first
	std::bitset<COMPONENT_COUNT> bitset = GetBitset<COMPONENTS...>();

	// structured binding
	for (auto& [key, compSet] : m_ComponentSets) {
		// not match => dont
		if ((bitset & key) != bitset) continue;

		compSet.Execute(func);
	}
	
}

template<class ...COMPONENTS>
void ECSManager::ExecuteRoot(std::function<void(COMPONENTS*...)>& func)//, Entity* ent)
{
	auto& compSet = m_ComponentSets;
	std::bitset<COMPONENT_COUNT> funcBitset = GetBitset<COMPONENTS...>();
	// for every root and Template Entities

	std::function<void(component::Root*)> forRoot = [&compSet, &func, funcBitset](component::Root* root) {
		// find bitset first
		Entity* ent = root->GetEntity();
		std::bitset<COMPONENT_COUNT> entityBitset = ent->GetBitset();
		int entityID = ent->GetInnerID();

		// do sth with Root & component
		if ((funcBitset & entityBitset) == funcBitset)
			compSet[entityBitset].Execute(entityID, func);
		};
	
	Execute(forRoot);
}

template<class ...COMPONENTS>
void ECSManager::ExecuteFromEntity(std::bitset<COMPONENT_COUNT> bit, int innerID, std::function<void(COMPONENTS*...)>& func)
{
	std::bitset<COMPONENT_COUNT> funcBitset = GetBitset<COMPONENTS...>();

	if ((funcBitset & bit) == funcBitset)
		m_ComponentSets[bit].Execute(innerID, func);
}

template<class T>
T* ECSManager::GetComponent(std::bitset<COMPONENT_COUNT> entBit, int innerId)
{
	if (m_ComponentSets.contains(entBit)) {
		return m_ComponentSets[entBit].GetComponent<T>(innerId);
	}

	return nullptr;
}

template<class ...COMPONENTS>
std::bitset<COMPONENT_COUNT> ECSManager::GetBitset()
{
	// fold expression, C++ 17
	return (COMPONENTS::GetBit() | ...);
}

