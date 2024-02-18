#pragma once

namespace Json { class Value; }
class ResourceManager;

namespace component {

	/////////////////////////////////////////////////////////
	// base component class
	//
	class Component
	{
	public:
		Component() = default;
		virtual ~Component() {}

		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr) = 0;
	};

	/////////////////////////////////////////////////////////
	// ComponentFactory
	// 한번 생성되면 변화는 없음
	// 
	//
	class ComponentFactory 
	{
		ComponentFactory() {}
		ComponentFactory(const ComponentFactory& other) = delete;
		ComponentFactory& operator=(const ComponentFactory& other) = delete;
		~ComponentFactory() {}

		std::map<std::string, std::function<Component* ()>> m_ComponentFactoryMap;

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
			GetInstance().m_ComponentFactoryMap[componentName] = []() { return new T; };
		}

		// get component by string
		static Component* GetComponent(const std::string& componentName)
		{
			auto& func = GetInstance().m_ComponentFactoryMap[componentName];
			return func ? func() : nullptr;
		}

		static bool IsExist(const std::string& componentName)
		{
			auto& map = GetInstance().m_ComponentFactoryMap;
			return map.find(componentName) != map.end();
		}
	};


	/////////////////////////////////////////////////////////
	// Name Component
	// 이름
	//
	class Name : public Component 
	{
		std::string m_Name;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
	};

	/////////////////////////////////////////////////////////
	// transform
	// 이동 관련
	//
	class Transform : public Component 
	{
		XMFLOAT3 m_Position;
		XMFLOAT4 m_Rotate;
		XMFLOAT3 m_Scale;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
	};

	/////////////////////////////////////////////////////////
	// render component
	// 렌더 관련, mesh, material
	//
	class Renderer : public Component 
	{
		int m_MeshID;
		int m_MaterialID;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		void SetMesh(int idx) { m_MeshID = idx; }
		void SetMaterial(int idx) { m_MaterialID = idx; }
	};

}


#define REGISTER_COMPONENT(TYPE, NAME) component::ComponentFactory::RegisterComponent<TYPE>(NAME)
#define GET_COMPONENT(NAME) component::ComponentFactory::GetComponent(NAME)
#define CHECK_COMPONENT(NAME) component::ComponentFactory::IsExist(NAME)
