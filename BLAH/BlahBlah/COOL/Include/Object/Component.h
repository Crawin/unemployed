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
			static_assert(std::is_base_of<component::Component, T>::value, "T is not base of Component!!");
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

		int GetMesh() const { return m_MeshID; }
		int GetMaterial() const { return m_MaterialID; }

		void SetMesh(int idx) { m_MeshID = idx; }
		void SetMaterial(int idx) { m_MaterialID = idx; }
	};

	/////////////////////////////////////////////////////////
	// camera component
	// 카메라 정보
	//
	class Camera : public Component
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
