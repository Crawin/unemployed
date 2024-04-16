#pragma once

#include "Light.h"
namespace Json { class Value; }
namespace ECSsystem { 
	class AnimationPlayTimeAdd;
}
class ResourceManager;
class Entity;
class AnimationPlayer;




namespace component {

	/////////////////////////////////////////////////////////
	// bass component class
	//
	class Component 
	{
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr) = 0;
		virtual std::bitset<COMPONENT_COUNT> GetBitset() = 0;

		virtual void ShowYourself() const = 0;
	};


	/////////////////////////////////////////////////////////
	// base component class for children
	//
	template<class Div>
	class ComponentBase : public Component
	{
		friend class ComponentFactory;

		inline static std::bitset<COMPONENT_COUNT> m_Bitset;

	public:
		ComponentBase() = default;
		virtual ~ComponentBase() {}

		virtual std::bitset<COMPONENT_COUNT> GetBitset() override { 
			return m_Bitset;
		}

		static std::bitset<COMPONENT_COUNT> GetBit() { return m_Bitset; }
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
			// Set bitset to each component
			std::bitset<COMPONENT_COUNT> bitset(1);
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

		virtual void ShowYourself() const;

		const std::string getName() { return m_Name; }
	};

	/////////////////////////////////////////////////////////
	// transform
	// 이동 관련
	//
	class Transform : public ComponentBase<Transform>
	{
		XMFLOAT3 m_Position;
		XMFLOAT3 m_Rotate;
		XMFLOAT3 m_Scale;

		XMFLOAT4X4 m_ParentTransform = Matrix4x4::Identity();

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		const XMFLOAT3& GetPosition() const { return m_Position; }
		const XMFLOAT3& GetRotation() const { return m_Rotate; }
		const XMFLOAT3& GetWorldPosition() const;
		const XMFLOAT3& GetWorldRotation() const;

		const XMFLOAT3& GetScale() const { return m_Scale; }
		const XMFLOAT4X4& GetParentTransfrom() const { return m_ParentTransform; }
		
		// 되도록이면 position끼리만을 쓰는것이 아니라 행렬을 원하면 이 함수를 쓰자
		XMFLOAT4X4& GetWorldTransform();
		XMFLOAT4X4& GetLocalTransform();

		void SetPosition(const XMFLOAT3& pos) { m_Position = pos; }
		void SetRotation(const XMFLOAT3& rot) { m_Rotate = rot; }
		void SetScale(const XMFLOAT3& sca) { m_Scale = sca; }
		void SetParentTransform(const XMFLOAT4X4& mat) { m_ParentTransform = mat; }


	};

	/////////////////////////////////////////////////////////
	// render component
	// 렌더 관련, mesh, material
	//
	class Renderer : public ComponentBase<Renderer>
	{
		XMFLOAT4X4 m_WorldMatrix = Matrix4x4::Identity();

		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

		int m_MeshID;
		int m_MaterialID;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		int GetMesh() const { return m_MeshID; }
		int GetMaterial() const { return m_MaterialID; }
		XMFLOAT4X4& GetWorldMatrix() { return m_WorldMatrix; }
		const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return m_VertexBufferView; }

		void SetMesh(int idx) { m_MeshID = idx; }
		void SetMaterial(int idx) { m_MaterialID = idx; }
		void SetWorldMatrix(const XMFLOAT4X4& mat) { m_WorldMatrix = mat; }
		void SetVertexBufferView(const D3D12_VERTEX_BUFFER_VIEW& view) { m_VertexBufferView = view; }
	};

	/////////////////////////////////////////////////////////
	// AnimationController Component
	// AnimationPlayer 를 가짐, AnimationPlayer의 수정은 여기서 이뤄짐
	//
	class AnimationController : public ComponentBase<AnimationController> {
		AnimationPlayer* m_AnimationPlayer = nullptr;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void UpdateTime(float deltaTime);

		void ChangeAnimationTo(const std::string& animDef);

		void SetPlayer(AnimationPlayer* player) { m_AnimationPlayer = player; }
	};

	/////////////////////////////////////////////////////////
	// AnimationExecutor component
	// 애니메이션플레이어를 참조, SOBV를 가짐
	//
	class AnimationExecutor : public ComponentBase<AnimationExecutor> {
		friend class ECSsystem::AnimationPlayTimeAdd;
		// blahblah 
		// animation data
		// blahblah
		int m_StreamOutBuffer = -1;
		D3D12_VERTEX_BUFFER_VIEW m_OriginalBufferView = {};
		D3D12_STREAM_OUTPUT_BUFFER_VIEW m_ToAnimateBufferView = {};

		//D3D12_SHADER_VIEW;

		// no data change
		const AnimationPlayer* m_AnimationPlayer = nullptr;

		//int m_CurAnimationIdx = -1;
		//int m_BefAnimationIdx = -1;

		//float m_BefAniWeight = 0;

		//float m_CurAniPlayTime = 0.0f;
		//float m_BefAniPlayTime = 0.0f;

		//// 하드코딩하였다...
		//float m_CurAniMaxPlayTime = 0.0f;
		//float m_BefAniMaxPlayTime = 0.0f;

		// self.idle, self.walk, self.run, self.blabal ...
		// 전부 만들어 둘까?
		// 아니면 데이터만 가지고 있고
		// 다른애가 set 해주게 만들까
		// ex) GuardAnimationPlayer라는 컴포넌트가 있음, system에서 GuardAnimPlayer & Speed, AnimationController을 해서 speed에 따라서 currentAnim을 바꿈
		// 구조를 보면 이게 맞을지도?
		// 생각해보자

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		int GetStreamOutBuffer() const { return m_StreamOutBuffer; }
		const D3D12_VERTEX_BUFFER_VIEW& GetOriginalVertexBufferView() const { return m_OriginalBufferView; }
		const D3D12_STREAM_OUTPUT_BUFFER_VIEW& GetStreamOutBufferView() const { return m_ToAnimateBufferView; }

		void SetStreamOutBuffer(int idx) { m_StreamOutBuffer = idx; }
		void SetOriginalVertexBufferView(const D3D12_VERTEX_BUFFER_VIEW& view) { m_OriginalBufferView = view; }
		void SetStreamOutBufferView(const D3D12_STREAM_OUTPUT_BUFFER_VIEW& view) { m_ToAnimateBufferView = view; }
		void SetPlayer(AnimationPlayer* player) { m_AnimationPlayer = player; }

		void SetData(ComPtr<ID3D12GraphicsCommandList> commandList, ResourceManager* manager);

		//int GetCurrentAnimation() const { return m_CurAnimationIdx; }
		//int GetBeforeAnimation() const { return m_BefAnimationIdx; }
		//float GetBeforeAnimationWeight() const { return m_BefAniWeight; }
		//float GetCurrentAnimationPlayTime() const { return m_CurAniPlayTime; }
		//float GetBeforeAnimationPlayTime() const { return m_BefAniPlayTime; }
		//float GetCurrentAnimationMaxTime() const { return m_CurAniMaxPlayTime; }
		//float GetBeforeAnimationMaxTime() const { return m_BefAniMaxPlayTime; }

		//void SetCurrentAnimation(int idx) { m_CurAnimationIdx = idx; }
		//void SetBeforeAnimation(int idx) { m_BefAnimationIdx = idx; }
		//void SetBeforeAnimationWeight(float weight) { m_BefAniWeight = weight; }
		//void SetCurrentAnimationPlayTime(float time) { m_CurAniPlayTime = time; }
		//void SetBeforeAnimationPlayTime(float time) { m_BefAniPlayTime = time; }
		//void SetCurrentAnimationMaxTime(float time) { m_CurAniMaxPlayTime = time; }
		//void SetBeforeAnimationMaxTime(float time) { m_BefAniMaxPlayTime = time; }

		//void ChangeAnimation(int toAnimIdx) { m_BefAnimationIdx = m_CurAnimationIdx; m_CurAnimationIdx = toAnimIdx; }		// todo 

		// 하드코딩 되어있다,
		UINT64* m_StreamSize = 0;

	};

	/////////////////////////////////////////////////////////
	// root
	// 루트 엔티티라면 가지고 있음
	//
	class Root : public ComponentBase<Root>
	{
		Entity* m_SelfEntity = nullptr;
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void SetEntity(Entity* ent) { m_SelfEntity = ent; }
		Entity* GetEntity() const { return m_SelfEntity; }
	};

	/////////////////////////////////////////////////////////
	// root
	// 루트 엔티티라면 가지고 있음
	//
	class Children : public ComponentBase<Children>
	{
		Entity* m_SelfEntity = nullptr;
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void SetEntity(Entity* ent) { m_SelfEntity = ent; }
		Entity* GetEntity() const { return m_SelfEntity; }
	};

	/////////////////////////////////////////////////////////
	// camera component
	// 카메라 정보
	//
	class Camera : public ComponentBase<Camera>
	{
	public:
		// Json으로 set 가능
		XMFLOAT3 m_Right = { 1.0f, 0.0f, 0.0f };
		XMFLOAT3 m_Up = { 0.0f, 1.0f, 0.0f };
		XMFLOAT3 m_Look = { 0.0f, 0.0f, 1.0f };
		XMFLOAT3 m_Position = { 0.0f, 30.0f, -150.0f };			// todo 이거 지우고 system에서 transform과 결합해 build view matrix 함수 수정

		float m_Fov = 90.0f;
		float m_Aspect = 1.7777f;
		float m_Near = 0.1f;
		float m_Far = 50000.0f;

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

		virtual void ShowYourself() const;

		void SetLook(const XMFLOAT3& look) { m_Look = look; }
		void SetRight(const XMFLOAT3& right) { m_Right = right; }
		void SetUp(const XMFLOAT3& up) { m_Up = up; }
		void SetPosition(const XMFLOAT3& pos) { m_Position = pos; }

		const XMFLOAT3& GetLook() const { return m_Look; }
		const XMFLOAT3& GetRight() const { return m_Right; }
		const XMFLOAT3& GetUp() const { return m_Up; }
		const XMFLOAT3& GetPosition() const { return m_Position; }

		const XMFLOAT4X4& GetViewMat() const { return m_ViewMatrix; }
		const XMFLOAT4X4& GetProjMat() const { return m_ProjMatrix; }

		void SetCameraData(ComPtr<ID3D12GraphicsCommandList> commandList);

	private:
		// 행렬 재생성
		// 이하 임시들임
		void BuildViewMatrix();
		void BuildProjectionMatrix();

		void UpdateShaderData();

		bool m_ProjChanged = false;

	};

	/////////////////////////////////////////////////////////
	// input component
	// 단순히 얘가 인풋을 받는 컴포넌트다 라고 알려주는 컴포넌트, 냉무, 임시, todo
	//
	class Input : public ComponentBase<Input> {

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};

	/////////////////////////////////////////////////////////
	// speed component
	// 최고속도, 가속도, 등등
	//
	class Speed : public ComponentBase<Speed> {
		float m_MaxVelocity = 300.0f;

		XMFLOAT3 m_Velocity = { 0,0,0 };
		XMFLOAT3 m_Acceleration = { 0,0,0 };

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		float GetMaxVelocity() const { return m_MaxVelocity; }

		float GetCurrentVelocityLen() const { return Vector3::Length(m_Velocity); }
		const XMFLOAT3& GetVelocity() const { return m_Velocity; }
		const XMFLOAT3 GetAcceleration() const { return m_Acceleration; }

		void SetMaxSpeed(float maxSpeed) { m_MaxVelocity = maxSpeed; }
		void SetVelocity(const XMFLOAT3& vel) { m_Velocity = vel; }
		void SetAcceleration(const XMFLOAT3& acc) { m_Acceleration = acc; }

		void AddVelocity(const XMFLOAT3& direction, float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// light component
	// 조명 정보
	//
	class Light : public ComponentBase<Light> {
		// 주의할 점
		// 카메라와 달리 얘는 배열로 관리 해야함
		LightData m_LightData;

		bool m_IsMainLight = false;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		LightData& GetLightData() { return m_LightData; }
	};

	/////////////////////////////////////////////////////////
	// TestInput component
	// 인풋 테스트용 
	//
	class TestInput : public ComponentBase<TestInput> {
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};

	/////////////////////////////////////////////////////////
	// Dia Animation Controll component
	// Dia를 쓰는 애를 위한 애니메이션 컨트롤 컴포넌트
	//
	class DiaAnimationControl : public ComponentBase<DiaAnimationControl> {
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		float m_BefSpeed = 0;

		bool m_BefKeyDown = false;
	};

	/////////////////////////////////////////////////////////
	// Day Light component
	// 시간에 따라 Directional Light가 회전되기 위한 컴포넌트
	//
	class DayLight : public ComponentBase<DayLight> {
		// 초단위
		float m_DayCycle = 30.0f;

		XMFLOAT4 m_NoonLight = {};
		XMFLOAT4 m_SunSetLight = {};
		XMFLOAT4 m_MoonLight = {};

		float m_LightAngle = 0.0f;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void SetLightAngle(float angle) { m_LightAngle = angle; }

		float GetDayCycle() const { return m_DayCycle; }
		const XMFLOAT4& GetNoonLight() const { return m_NoonLight; }
		const XMFLOAT4& GetSunSetLight() const { return m_SunSetLight; }
		const XMFLOAT4& GetMoonLight() const { return m_MoonLight; }
		float GetLightAngle() const { return m_LightAngle; }
	};

	/////////////////////////////////////////////////////////
	// Cloud component
	// 
	//
	class CloudComponent : public ComponentBase<CloudComponent> {
		float m_Width = 1000.0f;
		float m_Height = 1000.0f;

		int m_Clouds = 1000;
	};

	class Server : public ComponentBase<Server> {
		unsigned int m_id;
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		const unsigned int getID() { return m_id; }
		void setID(const unsigned int&);
	};
}


#define REGISTER_COMPONENT(TYPE, NAME) component::ComponentFactory::RegisterComponent<TYPE>(NAME)
#define GET_COMPONENT(NAME) component::ComponentFactory::GetComponent(NAME)
#define GET_COMP_COUNT component::ComponentFactory::GetComponentCount();
#define GET_COMP_SIZE(BITPOS) component::ComponentFactory::GetComponentSize(BITPOS);
#define PRINT_ALL_BITSET component::ComponentFactory::PrintBitsets();
