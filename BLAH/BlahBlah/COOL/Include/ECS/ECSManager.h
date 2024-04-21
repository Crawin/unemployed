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
	ComponentContainer() {
		DebugPrint("NONO");
	}
	ComponentContainer(size_t stride);

	template <class T>
	void push_back(const T* data);

	template <class T>
	T* GetData(size_t index);

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
	void Execute(std::function<void(COMPONENTS*...)>& func);

	template<class ...COMPONENTS>
	void Execute(int innerIdx, std::function<void(COMPONENTS*...)>& func);

	template<class COMP>
	COMP* GetComponent(int idx);

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
	void Execute(std::function<void(COMPONENTS*...)>& func);

	// execute from root (for sync transform)
	template<class ...COMPONENTS>
	void ExecuteRoot(std::function<void(COMPONENTS*...)>& func);//, Entity* ent);

	// execute exact entity(bit, innerID)
	template<class ...COMPONENTS>
	void ExecuteFromEntity(std::bitset<COMPONENT_COUNT> bit, int innerID, std::function<void(COMPONENTS*...)>& func);

	// execute O(n^2)
	template<class ...COMPONENTS>
	void ExecuteSquare(std::function<void(COMPONENTS*..., COMPONENTS*...)>& func);

	// 사용을 자제하자
	template<class T>
	T* GetComponent(std::bitset<COMPONENT_COUNT> entBit, int innerId);

	template<class T>
	T* GetComponent(Entity* entity);

private:
	template<class ...COMPONENTS>
	std::bitset<COMPONENT_COUNT> GetBitset();

};
