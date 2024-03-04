#pragma once



class Entity;
namespace component { class Component; }
class System;


template<std::size_t N, typename... Components>
class ComponentSet {

	std::unordered_map<std::bitset<N>, std::vector<component::Component>> m_Datas;


public:
	ComponentSet(std::bitset<N> bit) {
		// 미리 해당 bitset에 맞춰 vector를 만들어둔다.
		for (int i = 0; i < N; ++i) {
			int t = 1 << i;
			if (bit & t) {
				// add
				std::bitset bitset = Components::m_BIT;

			}
		}
	}

};


template<std::size_t N>
class ECSSystem
{
	// entity
	std::vector<Entity*> m_Entities;

	// component를 저장
	std::unordered_map<std::bitset<N>, std::vector<ComponentSet<N>*>> m_Components;

	// system
	std::vector<System*> m_System;

//private:
//	template <class Component, class Other ...>
//	void BuildComponentSet(ComponentSet<N>* set)
//	{
//		// do something
//
//
//		BuildComponentSet<Other>(set);
//	}
//
//
//public:
//	
//	template<class Components ...>
//	void CreateArchetype()
//	{
//		ComponentSet<N>* set = new ComponentSet<N>;
//
//		BuildComponentSet<Components>(set);
//
//
//	}


};

