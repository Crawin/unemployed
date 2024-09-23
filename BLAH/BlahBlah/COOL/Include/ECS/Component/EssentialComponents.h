#pragma once

#include "Light.h"

#include "Component.h"

namespace ECSsystem {
	class AnimationPlayTimeAdd;
	class CollideHandle;
}

class AnimationPlayer;

enum COLLIDE_EVENT_TYPE {
	BEGIN,
	ING,
	END
};

using InteractionFuncion = std::function<void(Entity*, Entity*)>;
using EventFunction = std::function<void(Entity*, Entity*)>;
using EventFunctionMap = std::unordered_map<COMP_BITSET, EventFunction>;

struct CollideEvents {
	EventFunctionMap m_OnBeginOverlap;
	EventFunctionMap m_OnOverlapping;
	EventFunctionMap m_OnEndOverlap;
};

struct CollidedEntity {
	COLLIDE_EVENT_TYPE m_Type;
	Entity* m_Entity;
};

class Material;

namespace component {
	class UICanvas;

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

		const std::string& getName() { return m_Name; }
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
		const XMFLOAT3& GetScale() const { return m_Scale; }

		XMFLOAT3* GetPositionPtr() { return &m_Position; }
		XMFLOAT3* GetRotationPtr() { return &m_Rotate; }
		XMFLOAT3* GetScalePtr() { return &m_Scale; }

		XMFLOAT3 GetWorldPosition() const;
		XMFLOAT3 GetWorldRotation() const;

		const XMFLOAT4X4& GetParentTransfrom() const { return m_ParentTransform; }

		// 되도록이면 position끼리만을 쓰는것이 아니라 행렬을 원하면 이 함수를 쓰자
		XMFLOAT4X4 GetWorldTransform() const;
		XMFLOAT4X4 GetLocalTransform() const;

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
		bool m_Active = true;

		XMFLOAT4X4 m_WorldMatrix = Matrix4x4::Identity();

		XMFLOAT4 m_ExtraShaderData[2] = { {0,0,0,0}, {0,0,0,0} };

		D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

		int m_MeshID;
		int m_MaterialID;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		int GetMesh() const { return m_MeshID; }
		int GetMaterial() const { return m_MaterialID; }
		XMFLOAT4X4& GetWorldMatrix() { return m_WorldMatrix; }
		XMFLOAT4* GetExtraShaderData() { return m_ExtraShaderData; }
		const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return m_VertexBufferView; }

		void SetMesh(int idx) { m_MeshID = idx; }
		void SetMaterial(int idx) { m_MaterialID = idx; }
		void SetWorldMatrix(const XMFLOAT4X4& mat) { m_WorldMatrix = mat; }
		void SetExtraShaderData(const XMFLOAT4& data, int idx) { m_ExtraShaderData[idx] = data; }
		void SetVertexBufferView(const D3D12_VERTEX_BUFFER_VIEW& view) { m_VertexBufferView = view; }

		void SetActive(bool state) { m_Active = state; }
		bool IsActive() const { return m_Active; }
	};

	/////////////////////////////////////////////////////////
	// AnimationController Component
	// AnimationPlayer 를 가짐, AnimationPlayer의 수정은 여기서 이뤄짐
	//
	class AnimationController : public ComponentBase<AnimationController> {
		AnimationPlayer* m_AnimationPlayer = nullptr;

		ANIMATION_STATE m_CurrentState = ANIMATION_STATE::IDLE;

		// for fsm
		std::map<ANIMATION_STATE, std::function<void(void)>> m_OnEnter;
		std::map<std::pair<ANIMATION_STATE, ANIMATION_STATE>, std::function<bool(void*)>> m_ChangeCondition;
		std::map<ANIMATION_STATE, std::vector<ANIMATION_STATE>> m_TransitionGraph;

		void InsertTransition(ANIMATION_STATE from, ANIMATION_STATE to) { m_TransitionGraph[from].push_back(to); }

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void UpdateTime(float deltaTime);

		void CheckTransition(void* data);

		// change animation, if call from outside, the animation will be changed
		void ChangeAnimationTo(ANIMATION_STATE animSet);

		float GetCurrentPlayTime() const;
		float GetCurrentPlayEndTime() const;

		void SetPlayer(AnimationPlayer* player) { m_AnimationPlayer = player; }

		void InsertOnEnter(ANIMATION_STATE st, std::function<void(void)> cond) { m_OnEnter[st] = cond; }
		void InsertCondition(ANIMATION_STATE from, ANIMATION_STATE to, std::function<bool(void*)> cond) { InsertTransition(from, to); m_ChangeCondition[std::pair(from, to)] = cond; }

		AnimationPlayer* GetPlayer() const { return m_AnimationPlayer; }
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

		// 하드코딩 되어있다,
		UINT64* m_StreamSize = 0;

	};

	/////////////////////////////////////////////////////////
	// SelfEntity component
	// 본인을 가리키는거 사용할진 모름)
	//
	class SelfEntity : public ComponentBase<SelfEntity>
	{
		Entity* m_Parent = nullptr;
		Entity* m_SelfEntity = nullptr;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void SetParent(Entity* par) { m_Parent = par; }
		void SetEntity(Entity* ent) { m_SelfEntity = ent; }

		Entity* GetParent() const { return m_Parent; }
		Entity* GetEntity() const { return m_SelfEntity; }

		// if parent == null, the entity is root entity
		bool IsRootEntity() const { return m_Parent == nullptr; }
	};


	/////////////////////////////////////////////////////////
	// Attach component
	// 뼈를 따라간다
	// 
	class Attach : public ComponentBase<Attach>
	{
		const AnimationPlayer* m_AnimationPlayer = nullptr;

		XMFLOAT3 m_OriginalPosition = { 0.0f, 0.0f, 0.0f };
		XMFLOAT3 m_OriginalRotate = { 0.0f, 0.0f, 0.0f };
		XMFLOAT3 m_OriginalScale = { 0.0f, 0.0f, 0.0f };

		int m_BoneIndex = -1;
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void SetPlayer(AnimationPlayer* animPlayer) { m_AnimationPlayer = animPlayer; }

		const AnimationPlayer* GetPlayer() const { return m_AnimationPlayer; }

		XMMATRIX& GetAnimatedBone();

		void SetOriginalPosition(const XMFLOAT3& pos) { m_OriginalPosition = pos; }
		void SetOriginalRotation(const XMFLOAT3& rot) { m_OriginalRotate = rot; }
		void SetOriginalScale(const XMFLOAT3& sca) { m_OriginalScale = sca; }

		const XMFLOAT3& GetOriginalPosition() const { return m_OriginalPosition; }
		const XMFLOAT3& GetOriginalRotation() const { return m_OriginalRotate; }
		const XMFLOAT3& GetOriginalScale() const { return m_OriginalScale; }
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

		float m_Width = 1280.0f;
		float m_Height = 720.0f;

		bool m_IsMainCamera = false;
		bool m_Active = false;

		// on off상태를 이분화
		bool m_ActiveStateOnRender = false;
		bool m_IsMainCameraOnRender = false;

		// camera matrix
		XMFLOAT4X4 m_ViewMatrix = Matrix4x4::Identity();
		XMFLOAT4X4 m_ProjMatrix = Matrix4x4::Identity();

		// for culling
		BoundingFrustum m_BoundingFrustum{};
		BoundingFrustum m_BoundingOriginFrustum{};

		// root signature
		CameraDataShader* m_ShaderData = nullptr;
		int m_MappedShaderData = -1;
		D3D12_GPU_VIRTUAL_ADDRESS m_ShaderDataGPUAddr = 0;

		// resource manager's camera rtv data
		int m_RenderTargetDataIndex = -1;

		int m_NoiseType = 0;
		bool m_Vignetting = false;

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

		BoundingFrustum& GetBoundingFrustum() { return m_BoundingFrustum; }

		XMFLOAT3 GetWorldPosition() const { 
			XMMATRIX inv = XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_ViewMatrix)); 
			XMFLOAT3 t;
			XMStoreFloat3(&t, inv.r[3]);
			return t;
		}
		XMFLOAT3 GetWorldDirection() const { return { m_ViewMatrix._31, m_ViewMatrix._32, m_ViewMatrix._33 }; }

		void SetCameraData(ComPtr<ID3D12GraphicsCommandList> commandList);

		int GetCameraIndex() const { return m_RenderTargetDataIndex; }

		void SetActive(bool state) { m_Active = state; }
		void SetMainCamera(bool state) { m_IsMainCamera = state; }

		void SyncActiveState() { m_ActiveStateOnRender = m_Active; m_IsMainCameraOnRender = m_IsMainCamera; }

		bool IsMainCamera() const { return m_IsMainCamera; }
		bool IsMainCameraOnRender() const { return m_IsMainCameraOnRender; }
		bool IsActive() const { return m_Active; }
		bool IsActiveOnRender() const { return m_ActiveStateOnRender; }

		int GetNoiseType() const { return m_NoiseType; }
		bool GetVignettingState() const { return m_Vignetting; }

	private:
		// 행렬 재생성
		void BuildProjectionMatrix();

		void UpdateShaderData();

		bool m_ProjChanged = false;

	};

	/////////////////////////////////////////////////////////
	// input component
	// 단순히 얘가 인풋을 받는 컴포넌트다 라고 알려주는 컴포넌트, 냉무, 임시, todo
	//
	//class Input : public ComponentBase<Input> {
	//	Entity* m_InteractionEntity = nullptr;

	//public:
	//	virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

	//	virtual void ShowYourself() const;

	//	void SetInteractionEntity(Entity* ent) { m_InteractionEntity = ent; }
	//	Entity* GetInteractionEntity() { return m_InteractionEntity; }
	//};

	/////////////////////////////////////////////////////////
	// Physics component
	// 최고속도, 가속도, 등등
	//
	class Physics : public ComponentBase<Physics> {
		float m_OriginalMaxVelocity = 300.0f;
		float m_CurrentMaxVelocity = 300.0f;
		float m_DashSpeed = 400.0f;

		float m_Elasticity = 1.1f;

		bool m_CalculatePhysics = false;
		bool m_Friction = true;
		bool m_Dashing = false;

		XMFLOAT3 m_Velocity = { 0,0,0 };
		XMFLOAT3 m_Acceleration = { 0,0,0 };

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		float GetOriginalMaxVelocity() const { return m_OriginalMaxVelocity; }
		float GetMaxVelocity() const { return m_CurrentMaxVelocity; }
		float* GetMaxVelocityPtr() { return &m_CurrentMaxVelocity; }

		float GetCurrentVelocityLen() const { return Vector3::Length(m_Velocity); }
		float GetCurrentVelocityLenOnXZ() const { XMFLOAT3 temp = { m_Velocity.x, 0.0f, m_Velocity.z }; return Vector3::Length(temp); }
		const XMFLOAT3& GetVelocity() const { return m_Velocity; }
		XMFLOAT3 GetVelocityOnXZ() const { XMFLOAT3 temp = { m_Velocity.x, 0.0f, m_Velocity.z }; return temp; }
		const XMFLOAT3& GetAcceleration() const { return m_Acceleration; }

		void SetOriginalMaxVelocity(float maxVel) { m_OriginalMaxVelocity = maxVel; }
		void SetMaxVelocity(float maxVel) { m_CurrentMaxVelocity = maxVel; }
		void SetVelocity(const XMFLOAT3& vel) { m_Velocity = vel; }
		void SetVelocityOnXZ(const XMFLOAT3& vel) { m_Velocity.x = vel.x; m_Velocity.z = vel.z; }
		void SetAcceleration(const XMFLOAT3& acc) { m_Acceleration = acc; }

		void AddVelocity(const XMFLOAT3& direction, float deltaTime);

		float GetElasticity() const { return m_Elasticity; }

		void SetCalculateState(bool state) { m_CalculatePhysics = state; }
		bool IsToCalculate() const { return m_CalculatePhysics; }

		void SetActiveFriction(bool state) { m_Friction = state; }
		bool IsFrictionActive() const { return m_Friction; }

		void SetDash(bool state) { m_Dashing = state; }
		bool IsDashing() const { return m_Dashing; }

		void SetDashSpeed(float speed) { m_DashSpeed = speed; }
		float GetDashSpeed() const { return m_DashSpeed; }
	};

	/////////////////////////////////////////////////////////
	// light component
	// 조명 정보
	//
	class Light : public ComponentBase<Light> {
		// 주의할 점
		// 카메라와 달리 얘는 배열로 관리 해야함
		LightData m_LightData;

		int m_LightIdx = -1;

		bool m_IsMainLight = false;
		bool m_CastShadow = false;

		float m_Score = -100;
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		bool IsMainLight() const { return m_IsMainLight; }
		bool IsCastShadow() const { return m_CastShadow; }

		LightData& GetLightData() { return m_LightData; }

		void CalculateScore(const XMFLOAT3& camPos, const XMFLOAT3& camDir);

		void RefreshScore() { m_Score = -100; }
		float GetScore() const { return m_Score; }

		void SetIndex(int idx) { m_LightIdx = idx; }
		int GetIndex() const { return m_LightIdx; }
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
	// Day Light component
	// 시간에 따라 Directional Light가 회전되기 위한 컴포넌트
	//
	class DayLight : public ComponentBase<DayLight> {
		// 초단위, m_DayCycle초 만큼 한바퀴를 돈다
		//
		XMFLOAT4 m_NoonLight = {};
		XMFLOAT4 m_SunSetLight = {};
		XMFLOAT4 m_MoonLight = {};
		XMFLOAT4 m_MinAmbient = {};
		XMFLOAT4 m_MaxAmbient = {};

		XMFLOAT3 m_OriginRotate{ 0,0,0 };

		Light* m_LightComponent = nullptr;

		float m_LightAngle = 0.0f;
		bool m_RenderShader = false;
		bool m_Day = true;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void SetLightAngle(float angle) { m_LightAngle = angle; }

		const XMFLOAT4& GetNoonLight() const { return m_NoonLight; }
		const XMFLOAT4& GetSunSetLight() const { return m_SunSetLight; }
		const XMFLOAT4& GetMoonLight() const { return m_MoonLight; }
		float GetLightAngle() const { return m_LightAngle; }

		const XMFLOAT4& GetMinAmbient() const { return m_MinAmbient; }
		const XMFLOAT4& GetMaxAmbient() const { return m_MaxAmbient; }
		
		const XMFLOAT3& GetOriginRotate() const { return m_OriginRotate; }

		const Light* GetLightComp() const { return m_LightComponent; }

		bool IsRender() const { return m_RenderShader; }
	};

	class DayLightManager : public ComponentBase<DayLightManager> {
		// 초단위, m_DayCycle초 만큼 한바퀴를 돈다
		float m_DayCycle = 30.0f;
		float m_CurTime = 0.0f;

		static int m_ManagerCount;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void ShowYourself() const {}

		void SetDayCycle(float cycle) { m_DayCycle = cycle; }
		float GetDayCycle() const { return m_DayCycle; }

		void SetCurTime(float time) { m_CurTime = time; }
		float GetCurTime() const { return m_CurTime; }

		float* GetCurTimePtr() { return &m_CurTime; }

		void TimeAdd(float deltaTime);
	};

	/////////////////////////////////////////////////////////
	// server component
	// 서버와 위치 동기화가 필요한 엔티티를 위한 컴포넌트
	//
	class Server : public ComponentBase<Server> {
		unsigned int m_id;
		bool m_SendMode = false;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		const unsigned int getID() { return m_id; }
		void setID(const unsigned int&);

		void SetSendMode(bool state) { m_SendMode = state; }
		bool IsSendMode() const { return m_SendMode; }
	};

	/////////////////////////////////////////////////////////
	// Collider component
	// 충돌체크
	//
	class Collider : public ComponentBase<Collider> {
		BoundingOrientedBox m_BoundingBoxOriginal;
		BoundingOrientedBox m_CurrentBox;
		bool m_StaticObject = false;
		bool m_Trigger = false;
		bool m_IsCapsule = false;
		bool m_Collided = false;

		CollideEvents m_EventFunctions;

		std::list<CollidedEntity> m_CollidedEntities;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void SetCollided(bool col) { m_Collided = col; }
		void SetOriginBox(const BoundingOrientedBox& box) { m_BoundingBoxOriginal = box; }
		void SetOriginBoxCenter(const BoundingOrientedBox& box) { m_BoundingBoxOriginal.Center = box.Center; }
		void SetBoundingBox(const BoundingOrientedBox& box) { m_CurrentBox = box; }
		void SetCapsule(bool b) { m_IsCapsule = b; }

		const BoundingOrientedBox& GetOriginalBox() const { return m_BoundingBoxOriginal; }
		const BoundingOrientedBox& GetBoundingBox() const { return m_CurrentBox; }
		bool GetCollided() const { return m_Collided; }
		bool IsStaticObject() const { return m_StaticObject; }
		bool IsCapsule() const { return m_IsCapsule; }
		bool IsTrigger() const { return m_Trigger; }

		void UpdateBoundingBox(const XMMATRIX& transMat);

		const std::list<CollidedEntity>& GetCollidedEntitiesList() const { return m_CollidedEntities; }
		void InsertCollidedEntity(Entity* ent);
		void UpdateCollidedList();
		void ResetList() { m_CollidedEntities = std::list<CollidedEntity>(); }
		const EventFunctionMap* GetEventMap(COLLIDE_EVENT_TYPE type) const;

		template<class COMP>
		void InsertEvent(EventFunction& eventFunc, COLLIDE_EVENT_TYPE type)
		{
			COMP_BITSET bit = COMP::GetBit();

			switch (type) {
			case COLLIDE_EVENT_TYPE::BEGIN:	m_EventFunctions.m_OnBeginOverlap[bit] = eventFunc;		break;
			case COLLIDE_EVENT_TYPE::ING:	m_EventFunctions.m_OnOverlapping[bit] = eventFunc;		break;
			case COLLIDE_EVENT_TYPE::END:	m_EventFunctions.m_OnEndOverlap[bit] = eventFunc;		break;
			default:
				DebugPrint("ERROR!! no event type");
	};
		}

	};

	/////////////////////////////////////////////////////////
	// Dynamic Collider component
	// 일반 콜라이더의 기준으로 루프를 돌지 않기 위해
	//
	class DynamicCollider : public ComponentBase<DynamicCollider> {
		BoundingOrientedBox m_BoundingBoxOriginal;
		BoundingOrientedBox m_CurrentBox;
		bool m_StaticObject = false;
		bool m_Trigger = false;
		bool m_IsCapsule = false;
		bool m_Collided = false;
		bool m_Active = true;
		bool m_ColideWithDynamic = true;

		float m_PossibleClimbHeight = 100.0f;

		CollideEvents m_EventFunctions;

		std::list<CollidedEntity> m_CollidedEntities;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		void SetCollided(bool col) { m_Collided = col; }
		void SetOriginBox(const BoundingOrientedBox& box) { m_BoundingBoxOriginal = box; }
		void SetBoundingBox(const BoundingOrientedBox& box) { m_CurrentBox = box; }
		void SetActive(bool active) { m_Active = active; }

		const BoundingOrientedBox& GetOriginalBox() const { return m_BoundingBoxOriginal; }
		const BoundingOrientedBox& GetBoundingBox() const { return m_CurrentBox; }
		bool GetCollided() const { return m_Collided; }
		bool IsStaticObject() const { return m_StaticObject; }
		bool IsCapsule() const { return m_IsCapsule; }
		bool IsTrigger() const { return m_Trigger; }
		bool IsActive() const { return m_Active; }

		void UpdateBoundingBox(const XMMATRIX& transMat);

		const std::list<CollidedEntity>& GetCollidedEntitiesList() const { return m_CollidedEntities; }
		void InsertCollidedEntity(Entity* ent);
		void UpdateCollidedList();
		void ResetList() { m_CollidedEntities = std::list<CollidedEntity>(); }

		const EventFunctionMap* GetEventMap(COLLIDE_EVENT_TYPE type) const;

		float GetPossibleClimb() const { return m_PossibleClimbHeight; }

		template<class COMP>
		void InsertEvent(EventFunction& eventFunc, COLLIDE_EVENT_TYPE type)
		{
			COMP_BITSET bit = COMP::GetBit();

			switch (type) {
			case COLLIDE_EVENT_TYPE::BEGIN:	m_EventFunctions.m_OnBeginOverlap[bit] = eventFunc;		break;
			case COLLIDE_EVENT_TYPE::ING:	m_EventFunctions.m_OnOverlapping[bit] = eventFunc;		break;
			case COLLIDE_EVENT_TYPE::END:	m_EventFunctions.m_OnEndOverlap[bit] = eventFunc;		break;
			default:
				DebugPrint("ERROR!! no event type");
			};
		}
	};
	
	/////////////////////////////////////////////////////////
	// Interaction Component
	// 상호작용이 있는 무언가를 위해
	//
	class Interaction : public ComponentBase<Interaction> {
		InteractionFuncion m_InteractionFunction = nullptr;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);
		virtual void ShowYourself() const;
		
		void SetInteractionFunction(InteractionFuncion& interaction) { m_InteractionFunction = interaction; }
		const InteractionFuncion& GetInteractionFunction() const { return m_InteractionFunction; }
	};

	/////////////////////////////////////////////////////////
	// AttachInputcomponent
	// 로컬 x축을 회전시키기 위해
	//
	class AttachInput : public ComponentBase<AttachInput> {

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};

	/////////////////////////////////////////////////////////
	// Player Component
	// 플레이어 컴포넌트
	//
	class Player : public ComponentBase<Player> {
		bool m_Jumping = false;
		bool m_OnGround = false;
		bool m_Sitting = false;

		XMFLOAT3 m_OriginalPosition{};
		XMFLOAT3 m_OriginalRotate{};

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);
		virtual void ShowYourself() const;

		void SetJumping(bool jumpst) { m_Jumping = jumpst; }
		void SetOnGround(bool state) { m_OnGround = state; }
		void SetSit(bool state) { m_Sitting = state; }

		bool IsAir() const { return m_Jumping && !m_OnGround; }
		bool IsSitting() const { return m_Sitting; }

		const XMFLOAT3& GetOriginalPosition() const { return m_OriginalPosition; }
		const XMFLOAT3& GetOriginalRotate() const { return m_OriginalRotate; }
	};

	/////////////////////////////////////////////////////////
	// Pawn Component
	// PlayerController에 의해 입력을 받는다, 입력 상태를 저장한다
	//
	class Inventory;
	class Pawn : public ComponentBase<Pawn> {
		KEY_STATE m_KeyStates[static_cast<long long int>(GAME_INPUT::GAME_INPUT_END)];
		Entity* m_InteractionEntity = nullptr;
		bool m_Active = false;
		POINT m_MouseDif = { 0, 0 };

		// todo string을 안써도 되는 방법을 찾아보자
		std::string m_CameraSocketName;
		Camera* m_Camera = nullptr;
		Physics* m_Physics = nullptr;
		Entity* m_CameraEntity = nullptr;
		Inventory* m_Inventory = nullptr;

		Entity* m_SelfEntity = nullptr;

		bool m_ControledByServer = false;

		std::string m_HudName = "";
		UICanvas* m_HudCanvas = nullptr;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);
		virtual void ShowYourself() const;

		void ResetInput();
		void TickInput();

		void PressInput(GAME_INPUT key);

		void SetInput(GAME_INPUT key, KEY_STATE state) { m_KeyStates[static_cast<int>(key)] = state; }

		KEY_STATE GetInputState(GAME_INPUT key) const { return m_KeyStates[static_cast<long long int>(key)]; }
		bool IsPressing(GAME_INPUT key) const;

		void SetInteractionEntity(Entity* ent) { m_InteractionEntity = ent; }
		Entity* GetInteractionEntity() { return m_InteractionEntity; }

		void SetActive(bool active, ECSManager* manager);
		bool IsActive() const { return m_Active; }

		void SetMouseMove(const POINT& pt) { m_MouseDif = pt; }
		const POINT& GetMouseMove() const { return m_MouseDif; }

		bool GetControlServer() const { return m_ControledByServer; }
		void SetControlServer(bool state) { m_ControledByServer = state; }

		Entity* GetCameraEntity() const { return m_CameraEntity; }
		Entity* GetSelfEntity() const { return m_SelfEntity; }
	};

	/////////////////////////////////////////////////////////
	// PlayerController Component
	// 플레이어 컨트롤러 컴포넌트, Player에 빙의
	//
	class PlayerController : public ComponentBase<PlayerController> {
		Pawn* m_CurrentPossess = nullptr;
		std::string m_TargetEntityName;			// todo 안써도 되는 방법을 찾아보자

		Pawn* m_MasterPlayerPawn = nullptr;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);
		virtual void ShowYourself() const;

		bool Possess(ECSManager* manager, const std::string& targetName);
		bool Possess(Pawn* target, ECSManager* manager);
		void PossessToMasterPlayer(ECSManager* manager);

		const std::string& GetTargetInit() const { return m_TargetEntityName; }
		Pawn* GetControllingPawn() const { return m_CurrentPossess; }

		void SetMasterPlayerPawn() { m_MasterPlayerPawn = m_CurrentPossess; }

	};

	/////////////////////////////////////////////////////////
	// AI Component
	// AI들이 가지는 컴포넌트
	//
	class AI : public ComponentBase<AI> {
		int m_Type = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		//virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);
		virtual void ShowYourself() const {}

		int GetType() const { return m_Type; }
	};

	/////////////////////////////////////////////////////////
	// Particle Component
	// Particle 컴포넌트, 각자 다른 파티클이다
	//
	struct ParticleData {
		XMFLOAT3 m_Position;
		XMFLOAT3 m_Velocity;
		float m_LifeTime;
	};
	class Particle : public ComponentBase<Particle> {
		Material* m_ParticleMaterial;

		PARTICLE_TYPES m_ParticleType;
		int m_MaxParticle = 500;
		float m_DefaultLifeTime = 5.0f;
		XMFLOAT2 m_ParticleSize{ 20.0f, 20.0f };

		std::list<ParticleData> m_ParticleDatas;

		XMFLOAT3* m_ShaderData = nullptr;
		int m_MappedShaderData = -1;
		D3D12_GPU_VIRTUAL_ADDRESS m_ShaderDataGPUAddr = 0;

		D3D12_VERTEX_BUFFER_VIEW m_ParticleBufferView = {};

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);
		virtual void ShowYourself() const {}

		void Tick(float deltaTime);

		void InsertParticle(XMFLOAT3 pos, XMFLOAT3 vel, int toInsert, float randRange);

		void SyncParticle();

		void OnRender(ComPtr<ID3D12GraphicsCommandList> commandList);

		PARTICLE_TYPES GetType() const { return m_ParticleType; }
	};

	class ParticleEmitter : public ComponentBase<ParticleEmitter> {
		bool m_Active = false;
		PARTICLE_TYPES m_ParticleType;
		
		int m_EmitteMount = 1;
		float m_EmittePerSec = 10.0f;

		float m_TimeElapsed = 0.0f;
		float m_RandomMount = 10.0f;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);
		virtual void ShowYourself() const {}

		void SetActive(bool state) { m_Active = state; }
		bool IsActive() const { return m_Active; }

		// return true when should emitte
		bool Tick(float deltaTime);

		PARTICLE_TYPES GetType() const { return m_ParticleType; }
		int GetEmitteMount() const { return m_EmitteMount; }
		float GetRandom() const { return m_RandomMount; }
	};

}
