#pragma once

#include "Component.h"

struct Input_State_In_LongLong
{
	long long int m_Data;

	Input_State_In_LongLong(GAME_INPUT input, KEY_STATE keyState) {
		long long int forward = static_cast<long long int>(input);
		long long int backward = static_cast<long long int>(keyState);
		forward <<= 32;

		m_Data = forward | backward;
	}

	GAME_INPUT GetInput() const { return static_cast<GAME_INPUT>(m_Data >> 32); }
	KEY_STATE GetState() const { return static_cast<KEY_STATE>(m_Data << 32 >> 32); }

	auto operator<=>(const Input_State_In_LongLong& other) const = default;
};

namespace std {
	template<>
	struct hash<Input_State_In_LongLong> {
		std::size_t operator()(const Input_State_In_LongLong& k) const {
			return std::hash<long long int>()(k.m_Data);
		}
	};
}

// void func(float deltaTime);
using ActionFunction = std::function<void(float)>;
using ActionFunctionMap = std::unordered_map<Input_State_In_LongLong, ActionFunction>;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 컨텐츠 관련 컴포넌트들, 게임 컨텐츠에 필요한 ui도 포함
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PlayerAnimation {
	XMFLOAT3 m_Velocity;
	XMFLOAT3 m_Heading;
};

namespace component {
	/////////////////////////////////////////////////////////
	// Player Animation Controll component
	// Player를 쓰는 애를 위한 애니메이션 컨트롤 컴포넌트
	//
	class PlayerAnimControll : public ComponentBase<PlayerAnimControll> {
		PlayerAnimation m_AnimData;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		float m_BefSpeed = 0;

		bool m_BefKeyDown = false;
	};

	/////////////////////////////////////////////////////////
	// Dia Animation Controll component
	// Dia를 쓰는 애를 위한 애니메이션 컨트롤 컴포넌트
	//
	class DiaAnimationControl : public ComponentBase<DiaAnimationControl> {
	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		float m_BefSpeed = 0;

		bool m_BefKeyDown = false;
	};

	/////////////////////////////////////////////////////////
	// Door Component
	// 문짝 컴포넌트, 잠금해제상태라면 플레이어가 직접 열 수 있다.
	//
	class DoorControl : public ComponentBase<DoorControl> {
		float m_MaxAngle = 120.0f;
		float m_CurrentAngle = 0.0f;

		bool m_Locked = true;
		int m_Answer = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		bool IsLocked() const { return m_Locked; }
		float GetMaxAngle() const { return m_MaxAngle; }
		float GetCurAngle() const { return m_CurrentAngle; }
		int GetAnswer() const { return m_Answer; }

		void SetLock(bool lock) { m_Locked = lock; }
		void SetMaxAngle(float angle) { m_MaxAngle = angle; }
		void SetCurAngle(float angle) { m_CurrentAngle = angle; }

	};

	/////////////////////////////////////////////////////////
	// Holdable Component
	// 손에 들 수 있는 오브젝트(손에 들 수 있다)
	//
	class Holdable : public ComponentBase<Holdable> {
		ActionFunctionMap m_ActionMap;

		// entity를 들고 있는 주체
		Entity* m_HoldingMaster = nullptr;

		// 원래 있던 곳, (사용하지 않는 Holdable들은 안보이는곳에 보관
		Entity* m_OriginalParent = nullptr;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		void SetAction(const Input_State_In_LongLong& keyCond, const ActionFunction& act) { m_ActionMap[keyCond] = act; }
		const ActionFunctionMap& GetActionMap() const { return m_ActionMap; }

		void SetMaster(Entity* master) { m_HoldingMaster = master; }
		Entity* GetMaster() const { return m_HoldingMaster; }
		Entity* GetOriginParent() const { return m_OriginalParent; }

		virtual void ShowYourself() const {};
	};

	/////////////////////////////////////////////////////////
	// Attackable Component
	// 공격 아이템 
	//
	class Attackable : public ComponentBase<Attackable> {

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};

	/////////////////////////////////////////////////////////
	// Throwable Component
	// CCTV, drinks
	//
	class Throwable : public ComponentBase<Throwable> {
		XMFLOAT3 m_DirectionOrigin = { 0.0f, 0.0f, 1.0f };
		XMFLOAT3 m_DirectionResult = { 0.0f, 0.0f, 1.0f };
		float m_ThrowBakeTime = 0.0f;
		float m_ThrowBakeTimeMax = 3.0f;
		float m_ThrowSpeed = 2000.0f;			// abuot 70km/h, our unit ->cm/s

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const {}
	};

#define MAX_INVENTORY 4
	/////////////////////////////////////////////////////////
	// Inventory Component
	// Holdable 들을 보관한다
	//
	class Inventory : public ComponentBase<Inventory> {
		Entity* m_Items[MAX_INVENTORY] = {nullptr, };
		std::string m_TargetEntityNames[MAX_INVENTORY] = { "", };
		//const char* m_TargetEntityNames[MAX_INVENTORY] = { nullptr, };
		int m_CurrentHolding = 0;

		// anim socket
		Entity* m_HoldingSocket = nullptr;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		bool ChangeHoldingItem(int idx, ECSManager* manager);

		Entity* GetCurrentHoldingItem() const { return m_Items[m_CurrentHolding]; }
		const Entity* GetHoldingSocket() const { return m_HoldingSocket; }

		void EraseCurrentHolding() { m_Items[m_CurrentHolding] = nullptr; }
		
	};

#define MAX_CCTV 4
	/////////////////////////////////////////////////////////
	// Screen Component
	// CCTV와 연결되는 스크린
	//
	class Screen : public ComponentBase<Screen> {
		int m_CameraRenderTargets[MAX_CCTV] = { 0, };

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// UI 관련 컴포넌트들
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////
	// UIKeypad component
	// 문열기 미니게임의 결과
	//
	class UIKeypad : public ComponentBase<UIKeypad> {
		Entity* m_DoorEntity = nullptr;
		int m_Answer = 0;
		int m_Current = 0;

	public:
		virtual void Create(Json::Value& v, ResourceManager* rm = nullptr);
		virtual void OnStart(Entity* selfEntity, ECSManager* manager = nullptr, ResourceManager* rm = nullptr);

		virtual void ShowYourself() const;

		Entity* GetDoor() { return m_DoorEntity; }

		void SetDoor(Entity* door) { m_DoorEntity = door; }
		void SetAnswer(int ans) { m_Answer = ans; }
		int GetAnswer() const { return m_Answer; }

		void SetCurrent(int Current) { m_Current = Current; }
		int GetCurrent() const { return m_Current; }
	};
}
