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
	std::bitset<COMPONENT_COUNT> bit(1);
	bit <<= comp->GetGID();

	// check the component is exist in the set
	if (m_Set.contains(bit) == false) {
		DebugPrint("ERROR!! not this component");
		exit(1);
	}

	// insert component;
	m_Set[bit].push_back(comp);

}

void ComponentSet::InsertComponentByEntity(Entity* entity)
{
	for (auto& comp : entity->m_Components) 
	{
		InsertComponent(comp);
	}

	++m_EntitySize;
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

template<class COMP>
COMP* ComponentSet::GetComponent(int idx)
{
	std::bitset<COMPONENT_COUNT> bit(1);
	bit <<= COMP::m_GID;

	//ComponentContainer& container = m_Set[bit];
	return m_Set[bit].GetData<COMP>(idx);
	//container[];
	//COMP* temp = container[idx];
	//return temp;
	//return nullptr;
}



/////////////////////////////////////////////////////////////////////////////////////////////////////
// ECS System
/////////////////////////////////////////////////////////////////////////////////////////////////////

inline void ECSManager::AddEntity(Entity* entity)
{
	// 1. 일단 entity가 어떤 component를 가지고 있는지 확인해야함
	std::bitset<COMPONENT_COUNT> bitset(0);
	for (auto& comp : entity->m_Components) {
		std::bitset<COMPONENT_COUNT> compBit(1);
		compBit <<= comp->GetGID();

		bitset |= compBit;
	}

	// 여기에 컴포넌트set 맞춰서 넣으면 된다.
	if (m_ComponentSets.contains(bitset) == false) 
	{
		// 없다면 만들어둔다
		m_ComponentSets.emplace(bitset, bitset);
	}

	m_ComponentSets[bitset].InsertComponentByEntity(entity);

	m_Entities.push_back(entity);

	DebugPrint(std::format("entity bitset: {}", bitset.to_string()));

}

void ECSManager::UpdateSystem(float deltaTime)
{
	for (auto& system : m_System) {
		system->Update(deltaTime);
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
std::bitset<COMPONENT_COUNT> ECSManager::GetBitset()
{
	// fold expression, C++ 17
	return (GetBit<COMPONENTS>() | ...);
}

template<class COMP>
std::bitset<COMPONENT_COUNT> ECSManager::GetBit()
{
	std::bitset<COMPONENT_COUNT> bit(1);
	bit <<= COMP::m_GID;

	return bit;
}


