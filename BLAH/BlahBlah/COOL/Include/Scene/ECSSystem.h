#pragma once



class Entity;
namespace component { class Component; }
class System;

// 컴포넌트들을 가지는 클래스
// std::vector<char>를 랩핑 하였다.
class ComponentContainer {
	size_t m_Stride = 0;
	size_t m_Elements = 0;
	std::vector<char> m_Data;

public:
	ComponentContainer() { DebugPrint("NONO"); };
	ComponentContainer(size_t stride);

	template <class T>
	void push_back(const T* data);

	template <class T>
	T* operator[](size_t index);

};


// ComponentContainer들을 가짐
template<std::size_t N>
class ComponentSet {

	// Component를 각각 저장하는 container
	std::unordered_map<std::bitset<N>, ComponentContainer> m_Set;

public:
	ComponentSet() {
		DebugPrint("NO");
	}
	ComponentSet(std::bitset<N> bit);

	void InsertComponent(component::Component* comp);

	template<class ...COMPONENTS>
	void Execute(std::function<void(COMPONENTS*...)>& func);

};


template<std::size_t N>
class ECSSystem
{
	// entity
	std::vector<Entity*> m_Entities;

	// ComponentSet을 저장
	std::unordered_map<std::bitset<N>, ComponentSet<N>> m_ComponentSets;

	// system
	std::vector<System*> m_System;

public:
	void AddEntity(Entity* entity);

	template<class ...COMPONENTS>
	void Execute(std::function<void(COMPONENTS*...)>& func);

private:

	//std::bitset<N> GetBitset() { return std::bitset<N>(0); }

	template<class ...COMPONENTS>
	std::bitset<N> GetBitset();

	template<class COMP>
	std::bitset<N> GetBit();
};
