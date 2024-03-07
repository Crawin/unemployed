#pragma once

namespace Json { class Value; }
class ResourceManager;

struct CameraDataShader {
	XMFLOAT4X4 m_ViewMatrix;
	XMFLOAT4X4 m_ProjMatrix;
	XMFLOAT3 m_CameraPosition;
};


namespace component {

	/////////////////////////////////////////////////////////
	// bass component class
	//
	class Component 
	{
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr) = 0;
		virtual int GetGID() = 0;
	};


	/////////////////////////////////////////////////////////
	// base component class for children
	//
	template<class Div>
	class ComponentBase : public Component
	{
	public:
		ComponentBase() = default;
		virtual ~ComponentBase() {}

		virtual int GetGID() override { return m_GID; }

		inline static int m_GID = -1;
	};

	/////////////////////////////////////////////////////////
	// ComponentFactory
	// 한번 생성되면 변화는 없음
	// 
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
			// Set GID to Each Component
			T::m_GID = GetInstance().m_ComponentCount++;

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

		static bool IsExist(const std::string& componentName)
		{
			auto& map = GetInstance().m_ComponentFactoryMap;
			return map.find(componentName) != map.end();
		}

		static int GetComponentCount()  { return GetInstance().m_ComponentCount; }

		static size_t GetComponentSize(const size_t bitPos) { return GetInstance().m_ComponentSizeOnID[bitPos]; }
	};


	/////////////////////////////////////////////////////////
	// Name Component
	// 이름
	//
	class Name : public ComponentBase<Name>
	{
		// todo 주의!!!! 이것은 컴포넌트set에 저장하지 말고 entity가 직접 가지게 할까?
		std::string m_Name;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
	};

	/////////////////////////////////////////////////////////
	// transform
	// 이동 관련
	//
	class Transform : public ComponentBase<Transform>
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
	class Renderer : public ComponentBase<Renderer>
	{
		int m_MeshID;
		int m_MaterialID;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		int GetMesh() const { return m_MeshID; }
		int GetMaterial() const { return m_MaterialID; }

		void SetMesh(int idx) { m_MeshID = idx; }
		void SetMaterial(int idx) { m_MaterialID = idx; }
	};

	/////////////////////////////////////////////////////////
	// camera component
	// 카메라 정보
	//
	class Camera : public ComponentBase<Camera>
	{
		// Json으로 set 가능
		XMFLOAT3 m_Right = { 1.0f, 0.0f, 0.0f };
		XMFLOAT3 m_Up = { 0.0f, 1.0f, 0.0f };
		XMFLOAT3 m_Look = { 0.0f, 0.0f, 1.0f };
		XMFLOAT3 m_Position = { 0.0f, 30.0f, -150.0f };			// todo 이거 지우고 system에서 transform과 결합해 build view matrix 함수 수정

		float m_Fov = 90.0f;
		float m_Aspect = 1.7777f;
		float m_Near = 0.1f;
		float m_Far = 1000.0f;

		bool m_IsMainCamera = false;

		// camera matrix
		XMFLOAT4X4 m_ViewMatrix = Matrix4x4::Identity();
		XMFLOAT4X4 m_ProjMatrix = Matrix4x4::Identity();

		// for culling
		BoundingFrustum m_BoundingFrustum{};

		// root signature
		CameraDataShader* m_ShaderData = nullptr;
		int m_MappedShaderData = -1;
		D3D12_GPU_VIRTUAL_ADDRESS m_ShaderDataGPUAddr = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		void SetLook(const XMFLOAT3& look) { m_Look = look; }
		void SetRight(const XMFLOAT3& right) { m_Right = right; }
		void SetUp(const XMFLOAT3& up) { m_Up = up; }

		XMFLOAT3 GetLook() const { return m_Look; }
		XMFLOAT3 GetRight() const { return m_Right; }
		XMFLOAT3 GetUp() const { return m_Up; }

		XMFLOAT4X4 GetViewMat() const { return m_ViewMatrix; }
		XMFLOAT4X4 GetProjMat() const { return m_ProjMatrix; }

		void SetCameraData(ComPtr<ID3D12GraphicsCommandList> commandList);

	private:
		// 행렬 재생성
		// 이하 임시들임
		void BuildViewMatrix();
		void BuildProjectionMatrix();

		void UpdateShaderData();
	};

}


#define REGISTER_COMPONENT(TYPE, NAME) component::ComponentFactory::RegisterComponent<TYPE>(NAME)
#define GET_COMPONENT(NAME) component::ComponentFactory::GetComponent(NAME)
#define CHECK_COMPONENT(NAME) component::ComponentFactory::IsExist(NAME)
#define GET_COMP_COUNT component::ComponentFactory::GetComponentCount();
#define GET_COMP_SIZE(BITPOS) component::ComponentFactory::GetComponentSize(BITPOS);

