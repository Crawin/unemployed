﻿#pragma once

#include "Entity.h"
#include "Component.h"
#include "ECS_System.h"

// 컴포넌트들을 가지는 클래스
// std::vector<char>를 랩핑 하였다.
class ComponentContainer {
	size_t m_Stride = 0;
	size_t m_Elements = 0;
	std::vector<char> m_Data;

public:
	ComponentContainer() {
		DebugPrint("NONO");
	}
	ComponentContainer(size_t stride);

	template <class T>
	void push_back(const T* data)
	{
		m_Data.resize(m_Data.size() + m_Stride);

		memcpy((&m_Data.front() + (m_Stride * m_Elements)), data, m_Stride);

		++m_Elements;
	}

	template <class T>
	T* GetData(size_t index)
	{
		// 포인터로 넘겨줘야 할듯하다
		return reinterpret_cast<T*>(&m_Data[index * m_Stride]);
	}

};

using ComponentContainerMap = std::unordered_map<std::bitset<COMPONENT_COUNT>, ComponentContainer>;

// ComponentContainer들을 가짐
class ComponentSet {

	//friend class ECSManager;
	// Component를 각각 저장하는 container
	ComponentContainerMap m_Set;

	int m_EntitySize = 0;

public:
	ComponentSet() {
		DebugPrint("NO");
	}
	ComponentSet(std::bitset<COMPONENT_COUNT> bit);

	void InsertComponentByEntity(Entity* entity);

	template<class ...COMPONENTS>
	void Execute(std::function<void(COMPONENTS*...)>& func)
	{
		for (int i = 0; i < m_EntitySize; ++i)
		{
			// https://en.cppreference.com/w/cpp/language/foldx
			// fold expression, C++ 17
			func(GetComponent<COMPONENTS>(i)...);

		}
	}

	template<class ...COMPONENTS>
	void Execute(int innerIdx, std::function<void(COMPONENTS*...)>& func)
	{
		func(GetComponent<COMPONENTS>(innerIdx)...);
	}

	template<class COMP>
	COMP* GetComponent(int idx)
	{
		if (m_Set.contains(COMP::GetBit()))
			return m_Set[COMP::GetBit()].GetData<COMP>(idx);

		return nullptr;
	}

	int GetEntitySize() const { return m_EntitySize; }

private:
	void InsertComponent(component::Component* comp);

};

using ComponentSetMap = std::unordered_map<std::bitset<COMPONENT_COUNT>, ComponentSet>;

class ECSManager
{
	// entity
	std::vector<Entity*> m_Entities;
	std::vector<Entity*> m_RootEntities;

	// ComponentSet을 저장
	std::unordered_map<std::bitset<COMPONENT_COUNT>, ComponentSet> m_ComponentSets;

	// system
	std::vector<ECSsystem::System*> m_Systems;

public:
	ECSManager();
	~ECSManager();

	void AddEntity(Entity* entity);
	void AddToRoot(Entity* entity);

	void InsertSystem(ECSsystem::System* system) { m_Systems.push_back(system); }

	void InitSystem();
	void UpdateSystem(float deltaTime);

	// execute normal O(n)
	template<class ...COMPONENTS>
	void Execute(std::function<void(COMPONENTS*...)>& func)
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

	// execute from root (for sync transform)
	template<class ...COMPONENTS>
	void ExecuteRoot(std::function<void(COMPONENTS*...)>& func) 
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

	// execute exact entity(bit, innerID)
	template<class ...COMPONENTS>
	void ExecuteFromEntity(std::bitset<COMPONENT_COUNT> bit, int innerID, std::function<void(COMPONENTS*...)>& func)
	{
		std::bitset<COMPONENT_COUNT> funcBitset = GetBitset<COMPONENTS...>();

		if ((funcBitset & bit) == funcBitset)
			m_ComponentSets[bit].Execute(innerID, func);
	}

	// execute O(n^2)
	template<class ...COMPONENTS>
	void ExecuteSquare(std::function<void(COMPONENTS*..., COMPONENTS*...)>& func) 
	{
		// find bitset first
		std::bitset<COMPONENT_COUNT> bitset = GetBitset<COMPONENTS...>();

		for (auto outIter = m_ComponentSets.begin(); outIter != m_ComponentSets.end(); ++outIter) {
			if ((bitset & outIter->first) != bitset) continue;

			ComponentSet& out = outIter->second;
			int outEntitySize = out.GetEntitySize();

			for (int i = 0; i < outEntitySize; ++i) {
				// inner loop from here
				for (auto inIter = outIter; inIter != m_ComponentSets.end(); ++inIter) {
					if ((bitset & inIter->first) != bitset) continue;

					ComponentSet& in = inIter->second;
					int inEntitySize = in.GetEntitySize();

					for (int j = (inIter == outIter) ? (i + 1) : 0; j < inEntitySize; ++j) {
						// do something
						//out.ExecuteWithOtherComps(func, in.GetComponent<COMPONENTS>(j)...);
						func(out.GetComponent<COMPONENTS>(i)..., in.GetComponent<COMPONENTS>(j)...);
					}
				}
			}
		}
	}

	// 사용을 자제하자
	template<class T>
	T* GetComponent(std::bitset<COMPONENT_COUNT> entBit, int innerId)
	{
		if (m_ComponentSets.contains(entBit)) {
			return m_ComponentSets[entBit].GetComponent<T>(innerId);
		}

		return nullptr;
	}

	template<class T>
	T* GetComponent(Entity* entity)
	{
		std::bitset<COMPONENT_COUNT> entBit = entity->GetBitset();

		int innerId = entity->GetInnerID();

		if (m_ComponentSets.contains(entBit)) {
			return m_ComponentSets[entBit].GetComponent<T>(innerId);
		}

		return nullptr;
	}

private:
	template<class ...COMPONENTS>
	std::bitset<COMPONENT_COUNT> GetBitset()
	{
		return (COMPONENTS::GetBit() | ...);
	}

};