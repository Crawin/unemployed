#pragma once

// unordered_map <- 80 bytes
// map <- 24 bytes

class ResourceManager;
class Entity;
class ECSManager;
namespace Json { class Value; }

namespace component {

	/////////////////////////////////////////////////////////
	// bass component class
	//
	class Component 
	{
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr) = 0;
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr) {};
		virtual COMP_BITSET GetBitset() = 0;

		virtual void ShowYourself() const = 0;
	};


	/////////////////////////////////////////////////////////
	// base component class for children
	//
	template<class Div>
	class ComponentBase : public Component
	{
		friend class ComponentFactory;

		inline static COMP_BITSET m_Bitset;

	public:
		ComponentBase() = default;
		virtual ~ComponentBase() {}

		virtual COMP_BITSET GetBitset() override { 
			return m_Bitset;
		}

		static COMP_BITSET GetBit() { return m_Bitset; }
	};

	/////////////////////////////////////////////////////////
	// ComponentFactory
	// 한번 생성되면 변화는 없음
	//
	class ComponentFactory
	{
		// 싱글톤
		ComponentFactory() {}
		ComponentFactory(const ComponentFactory& other) = delete;
		ComponentFactory& operator=(const ComponentFactory& other) = delete;
		~ComponentFactory() {}

		// component size
		int m_ComponentCount = 0;

		// component maker map
		std::map<std::string, std::function<Component* ()>> m_ComponentFactoryMap;

		// component size
		std::vector<size_t> m_ComponentSizeOnID;

		//public:
			// 굳이 얘를 밖으로 뺄 이유가 없는듯?
		static ComponentFactory& GetInstance() {
			static ComponentFactory inst;
			return inst;
		}

	public:

		// register component 
		template<class T>
		static void RegisterComponent(const std::string& componentName)
		{
#ifdef _DEBUG
			static_assert(std::is_base_of<component::Component, T>::value, "T is not base of Component!!");
#endif
			// Set bitset to each component
			COMP_BITSET bitset(1);
			bitset <<= GetInstance().m_ComponentCount++;
			T::m_Bitset = bitset;

			DebugPrint(std::format("Component Register!!\t bitset: {}, name: {}", T::m_Bitset.to_string(), componentName));

			// Register Component size, 
			GetInstance().m_ComponentSizeOnID.push_back(sizeof(T));

			// Register Component to Factory Map
			GetInstance().m_ComponentFactoryMap[componentName] = []() { return new T; };
		}

		// get component by string
		static Component* GetComponent(const std::string& componentName)
		{
			auto& func = GetInstance().m_ComponentFactoryMap[componentName];
			return func ? func() : nullptr;
		}

		static int GetComponentCount()  { return GetInstance().m_ComponentCount; }

		static size_t GetComponentSize(const size_t bitPos) { return GetInstance().m_ComponentSizeOnID[bitPos]; }

		static void PrintBitsets() {
			auto& map = GetInstance().m_ComponentFactoryMap;

			for (auto& [name, val] : map) {
				Component* data = val != nullptr ? val() : nullptr;
				if (data == nullptr) continue;
				DebugPrint(std::format("bitset: {}, name: {}", data->GetBitset().to_string(), name));
				delete data;
			}

		}
	};
}


#define REGISTER_COMPONENT(TYPE, NAME) component::ComponentFactory::RegisterComponent<TYPE>(NAME)
#define GET_COMPONENT(NAME) component::ComponentFactory::GetComponent(NAME)
#define GET_COMP_COUNT component::ComponentFactory::GetComponentCount();
#define GET_COMP_SIZE(BITPOS) component::ComponentFactory::GetComponentSize(BITPOS);
#define PRINT_ALL_BITSET component::ComponentFactory::PrintBitsets();

// include files here
// not include comp.h there

#include "EssentialComponents.h"
#include "UIComponents.h"
#include "ContentComponents.h"
