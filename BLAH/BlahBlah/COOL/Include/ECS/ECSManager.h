#pragma once



class Entity;
namespace component { class Component; }
namespace ECSsystem { class System; }

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
	T* GetData(size_t index);

};


// ComponentContainer들을 가짐
class ComponentSet {

	// Component를 각각 저장하는 container
	std::unordered_map<std::bitset<COMPONENT_COUNT>, ComponentContainer> m_Set;

	int m_EntitySize = 0;

public:
	ComponentSet() {
		DebugPrint("NO");
	}
	ComponentSet(std::bitset<COMPONENT_COUNT> bit);

	void InsertComponentByEntity(Entity* entity);

	template<class ...COMPONENTS>
	void Execute(std::function<void(COMPONENTS*...)>& func);

	template<class COMP>
	COMP* GetComponent(int idx);

private:
	void InsertComponent(component::Component* comp);

};


class ECSManager
{
	// entity
	std::vector<Entity*> m_Entities;

	// ComponentSet을 저장
	std::unordered_map<std::bitset<COMPONENT_COUNT>, ComponentSet> m_ComponentSets;

	// system
	std::vector<ECSsystem::System*> m_Systems;

public:
	ECSManager();
	~ECSManager();

	void AddEntity(Entity* entity);

	void InsertSystem(ECSsystem::System* system) { m_Systems.push_back(system); }

	void UpdateSystem(float deltaTime);

	template<class ...COMPONENTS>
	void Execute(std::function<void(COMPONENTS*...)>& func);

private:

	template<class ...COMPONENTS>
	std::bitset<COMPONENT_COUNT> GetBitset();

};
